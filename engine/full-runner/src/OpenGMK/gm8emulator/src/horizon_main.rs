//! Native C entry point. Uses original assets from an isolated execution copy.
#[path = "horizon_control_store.rs"]
mod control_store;
#[path = "sprite_open_probe.rs"]
mod sprite_open_probe;

pub(crate) const SOUNDFONT_PATH: &str = "sdmc:/switch/reloaded-nx/engine/TimGM6mb.sf2";

#[no_mangle]
pub extern "C" fn nx_full_run()->i32 {
    std::panic::set_hook(Box::new(|info|eprintln!("RUST_PANIC {info}")));
    let root="sdmc:/switch/reloaded-nx/session/game";
    if let Err(e)=std::fs::create_dir_all("sdmc:/switch/reloaded-nx/session/temp") {eprintln!("TEMP_DIR {e}");}
    std::env::set_var("TMPDIR","sdmc:/switch/reloaded-nx/session/temp");
    std::env::set_var("OPENGMK_SOUNDFONT",SOUNDFONT_PATH);
    // Read-only diagnostic: use the same numeric reader as GML. Never select,
    // create or overwrite the game's intro configuration here.
    let config=format!("{root}/data/audio/config.ini");
    match crate::gml::file::TextHandle::open(&config,crate::gml::file::AccessMode::Read) {
        Ok(mut file)=>eprintln!("INTRO04 config={config:?} value={:?} original_selection_preserved=true",file.read_real()),
        Err(error)=>eprintln!("INTRO04 config={config:?} read_error={error}; original GML handles creation/selection"),
    }
    sprite_open_probe::run(root);
    crate::battle_trace::init();
    let cache_worker=crate::resource_cache::start(root);
    let path=format!("{root}/Proyecto Reloaded The Last Beta 1.9.2.2 Full.exe");
    eprintln!("FULL_RUNNER_START original EXE path={path}; mods/overrides disabled");
    let code=crate::run_game(vec!["reloaded-nx".into(),path,"-t".into()]);
    drop(cache_worker);
    eprintln!("FULL_RUNNER_END code={code}");code
}
