//! Filesystem side of the native controls panel. Only the runner's config directory is used.
use serde::{Deserialize, Serialize};
use std::{
    fs::{self, File, OpenOptions},
    io::{self, Read, Write},
    path::Path,
};

type Layers = [[[u8; 20]; 2]; 2];
type RailLayers = [[[u8; 16]; 2]; 2];
type LegacyLayers = [[[u8; 14]; 2]; 2];

#[derive(Debug, Deserialize, Serialize)]
#[serde(untagged)]
enum ProfileLayers {
    Current(Layers),
    Rails(RailLayers),
    Legacy(LegacyLayers),
}

impl ProfileLayers {
    fn decode(self, version: u8) -> io::Result<Layers> {
        match (self, version) {
            (Self::Current(layers), 6) => Ok(layers),
            (Self::Rails(layers), 5) => Ok(expand_layers(layers)),
            (Self::Legacy(layers), 1..=4) => Ok(expand_layers(layers)),
            _ => Err(invalid()),
        }
    }
}

fn expand_layers<const N: usize>(layers: [[[u8; N]; 2]; 2]) -> Layers {
    let mut expanded = Layers::default();
    for (new, old) in expanded.iter_mut().flatten().zip(layers.iter().flatten()) {
        new[..N].copy_from_slice(old);
    }
    expanded
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Bindings {
    bindings: Layers,
    chords: Layers,
    manual_battle: u8,
}
const LIMIT: u64 = 4096;
const PRIMARY: &str = "controls-1.4.json";
const TEMP: &str = "controls-1.4.json.tmp";
const BACKUP: &str = "controls-1.4.json.bak";
const LEGACY: [&str;15] = ["controls-1.3.json","controls-1.3.json.tmp","controls-1.3.json.bak",
                         "controls-1.1.json","controls-1.1.json.tmp","controls-1.1.json.bak",
                         "controls-1.0.1.json","controls-1.0.1.json.tmp","controls-1.0.1.json.bak",
                         "controls-h06d.json","controls-h06d.json.tmp","controls-h06d.json.bak",
                         "controls.json","controls.json.tmp","controls.json.bak"];

#[derive(Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
struct Profile {
    version: u8,
    bindings: ProfileLayers,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    chords: Option<ProfileLayers>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    manual_battle: Option<bool>,
}

fn valid_key(key: u8) -> bool {
    matches!(key, 0 | 1 | 8 | 13 | 16..=19 | 27 | 32..=40 | 44..=46 |
        48..=57 | 65..=90 | 96..=107 | 109..=123 | 161)
}

fn invalid() -> io::Error {
    io::Error::new(io::ErrorKind::InvalidData, "invalid controls profile")
}

fn valid(config: &Bindings) -> bool {
    config.manual_battle <= 1 && [&config.bindings,&config.chords].into_iter().flatten().flatten().flatten().copied().all(valid_key)
        && config.chords.iter().flatten().all(|layer| layer[10] == 0 && layer[11] == 0)
}

fn read(path: &Path) -> io::Result<Bindings> {
    let mut bytes = Vec::new();
    File::open(path)?.take(LIMIT + 1).read_to_end(&mut bytes)?;
    if bytes.len() as u64 > LIMIT { return Err(invalid()); }
    let profile: Profile = serde_json::from_slice(&bytes)?;
    let (chords, manual_battle) = match (profile.version, profile.chords, profile.manual_battle) {
        (1, None, None) => (Layers::default(), 0),
        (2 | 3, Some(chords), None) => (chords.decode(profile.version)?, 0),
        (4..=6, Some(chords), Some(manual)) => (chords.decode(profile.version)?, u8::from(manual)),
        _ => return Err(invalid()),
    };
    let config = Bindings { bindings: profile.bindings.decode(profile.version)?, chords, manual_battle };
    if !valid(&config) { return Err(invalid()); }
    Ok(config)
}

fn load(dir: &Path) -> Option<(Bindings, bool)> {
    [PRIMARY, TEMP, BACKUP].into_iter().chain(LEGACY).find_map(|name| {
        read(&dir.join(name)).ok().map(|bindings| (bindings, name != PRIMARY))
    })
}

fn remove_if_present(path: &Path) -> io::Result<()> {
    match fs::remove_file(path) {
        Err(e) if e.kind() == io::ErrorKind::NotFound => Ok(()),
        result => result,
    }
}

fn save(dir: &Path, bindings: Bindings) -> io::Result<()> {
    if !valid(&bindings) { return Err(invalid()); }
    fs::create_dir_all(dir)?;
    let primary = dir.join(PRIMARY);
    let temp = dir.join(TEMP);
    let backup = dir.join(BACKUP);
    if read(&primary).is_err() && read(&temp).is_ok() {
        // Preserve a recoverable generation before reusing the staging filename.
        remove_if_present(&primary)?;
        fs::rename(&temp, &primary)?;
    }
    remove_if_present(&temp)?;
    let mut file = OpenOptions::new().create_new(true).write(true).open(&temp)?;
    serde_json::to_writer_pretty(&mut file, &Profile { version: 6, bindings: ProfileLayers::Current(bindings.bindings),
        chords: Some(ProfileLayers::Current(bindings.chords)),
        manual_battle: Some(bindings.manual_battle != 0) })?;
    file.write_all(b"\n")?;
    file.sync_all()?;
    drop(file);
    if read(&temp)? != bindings { return Err(invalid()); }

    if read(&primary).is_ok() {
        remove_if_present(&backup)?;
        fs::rename(&primary, &backup)?;
    } else {
        remove_if_present(&primary)?;
    }
    fs::rename(&temp, &primary)
}

#[cfg(target_os = "horizon")]
const DIR: &str = "sdmc:/switch/reloaded-nx/config";

const _: [(); 161] = [(); std::mem::size_of::<Bindings>()];

// C owns the buffer and calls these on the runner thread only.
#[cfg(target_os = "horizon")]
#[no_mangle]
pub unsafe extern "C" fn nx_control_load(out: *mut Bindings) -> i32 {
    let dir = Path::new(DIR);
    match load(dir) {
        Some((bindings, recovered)) => {
            unsafe { out.write(bindings) };
            if recovered { eprintln!("CONTROLS06B recovered profile"); }
            if recovered { 2 } else { 1 }
        }
        None if [PRIMARY, TEMP, BACKUP].into_iter().chain(LEGACY).any(|name| {
            !matches!(fs::metadata(dir.join(name)), Err(e) if e.kind() == io::ErrorKind::NotFound)
        }) => {
            eprintln!("CONTROLS06B invalid or unreadable profile; using safe defaults");
            -1
        }
        None => 0,
    }
}

#[cfg(target_os = "horizon")]
#[no_mangle]
pub unsafe extern "C" fn nx_control_save(bindings: *const Bindings) -> i32 {
    match save(Path::new(DIR), unsafe { bindings.read() }) {
        Ok(()) => 0,
        Err(e) => { eprintln!("CONTROLS06B save failed: {e}"); -1 }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn versioned_roundtrip_and_recovery() {
        let dir = std::env::temp_dir().join(format!("reloaded-controls-{}", std::process::id()));
        fs::create_dir(&dir).unwrap();
        let mut first = Bindings::default();
        first.bindings[0][0][1] = b'X';
        let mut second = first;
        second.bindings[1][1][2] = 1; // Docked PC click.
        second.chords[0][0][2] = 123;
        second.manual_battle = 1;
        second.bindings[1][0][14] = b'Z';
        second.chords[0][1][15] = 1;
        second.bindings[1][0][16..].copy_from_slice(b"MPES");
        second.chords[0][1][19] = b'M';
        assert!(load(&dir).is_none());
        let mut old = LegacyLayers::default();
        old[0][0][1] = b'X';
        let legacy = serde_json::to_vec(&Profile {
            version: 4, bindings: ProfileLayers::Legacy(old),
            chords: Some(ProfileLayers::Legacy(LegacyLayers::default())), manual_battle: Some(false),
        }).unwrap();
        let old_file = dir.join("controls.json");
        fs::write(&old_file, &legacy).unwrap();
        assert_eq!(load(&dir), Some((first, true)));
        let mut rail_bindings = RailLayers::default();
        rail_bindings[0][0][1] = b'X';
        rail_bindings[1][0][14] = b'Z';
        let mut rail_chords = RailLayers::default();
        rail_chords[0][1][15] = 1;
        let rails = serde_json::to_vec(&Profile {
            version: 5, bindings: ProfileLayers::Rails(rail_bindings),
            chords: Some(ProfileLayers::Rails(rail_chords)), manual_battle: Some(true),
        }).unwrap();
        let rails_file = dir.join(LEGACY[0]);
        fs::write(&rails_file, &rails).unwrap();
        let migrated = Bindings { bindings: expand_layers(rail_bindings), chords: expand_layers(rail_chords),
            manual_battle: 1 };
        assert_eq!(load(&dir), Some((migrated, true)));
        assert_eq!(migrated.bindings[1][0][16..], [0; 4]);
        assert_eq!(migrated.chords[0][1][16..], [0; 4]);
        assert!(!dir.join(PRIMARY).exists());
        save(&dir, first).unwrap();
        save(&dir, second).unwrap();
        assert_eq!(load(&dir), Some((second, false)));
        assert_eq!(read(&dir.join(BACKUP)).unwrap(), first);
        fs::write(dir.join(PRIMARY), b"{broken").unwrap();
        assert_eq!(load(&dir), Some((first, true)));
        save(&dir, second).unwrap();
        fs::rename(dir.join(PRIMARY), dir.join(TEMP)).unwrap();
        assert_eq!(load(&dir), Some((second, true)));
        save(&dir, first).unwrap();
        assert_eq!(load(&dir), Some((first, false)));
        assert_eq!(read(&dir.join(BACKUP)).unwrap(), second);
        let mut invalid_keys = first;
        invalid_keys.bindings[0][1][2] = 255;
        assert!(save(&dir, invalid_keys).is_err());
        assert_eq!(load(&dir), Some((first, false)));
        fs::write(dir.join(PRIMARY), vec![b' '; LIMIT as usize + 1]).unwrap();
        fs::write(dir.join(BACKUP), serde_json::to_vec(&Profile {version: 99, bindings: ProfileLayers::Current(second.bindings), chords: Some(ProfileLayers::Current(second.chords)), manual_battle: None}).unwrap()).unwrap();
        assert_eq!(fs::read(&rails_file).unwrap(), rails);
        assert_eq!(fs::read(&old_file).unwrap(), legacy);
        assert_eq!(load(&dir), Some((migrated, true)));
        fs::remove_file(rails_file).unwrap();
        assert_eq!(load(&dir), Some((first, true)));
        fs::remove_file(old_file).unwrap();
        assert!(load(&dir).is_none());
        let blocked = dir.join("not-a-directory");
        fs::write(&blocked,b"file").unwrap();
        assert!(save(&blocked, first).is_err());
        fs::remove_file(blocked).unwrap();
        fs::remove_file(dir.join(PRIMARY)).unwrap();
        fs::remove_file(dir.join(BACKUP)).unwrap();
        fs::remove_dir(dir).unwrap();
    }
}
