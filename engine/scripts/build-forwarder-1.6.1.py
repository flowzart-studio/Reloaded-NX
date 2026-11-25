"""Build the optional Reloaded-NX 1.6.1 HOME shortcut with local NTON tools."""

# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
import re
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path


TITLE_ID = "01834ad7b4ee8000"
SD_PATH = b"sdmc:/switch/reloaded-nx/reloaded-nx.nro"
NRO_SHA = "32b52aeb13fcb873ffed0288162ff831ef7f4b79f6735f050af015187fbd8d8c"
ICON_SHA = "2547b95a86026bdd031413efcad711fb9d193431a63ea49085c346080668dcc9"
LOADER_REVISION = "c5d543a91b7cd1df880f98c9ab831be9a40d024f"
EXEFS = {
    "main": "b8089e39f63af1827c9a41898c7a70be18cab447e6f41af3e1aa8fe1c4afafe8",
    "main.npdm": "cf3236ed4eaa743fea90678f5dc73c608221ee52429ebe0e93c74aa596d6cc8f",
}
TOOLS = {
    "hacbrewpack.exe": "a4a78e20efeb00dea69a4b02f5d16f7c6161b7aa9af802517756ba019cb7f8c4",
    "nstool.exe": "70cd77d353190e07039a361f0912a27784475dbc1fb7a13a660e43e18412da95",
}
SAVE_SIZES = (0x3080, 0x3088, 0x3090, 0x3098, 0x30A0, 0x3148, 0x3150,
              0x3158, 0x3160, 0x3168, 0x3170, 0x3178, 0x3180)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def checked(path, expected):
    data = path.read_bytes()
    if digest(data) != expected:
        raise ValueError(f"Unexpected SHA-256 for {path.name}")
    return data


def run(tool, args, stage):
    # Keep tool diagnostics and local key paths out of reports and manifests.
    result = subprocess.run([str(tool), *map(str, args)], cwd=stage, capture_output=True)
    if result.returncode:
        raise ValueError(f"{tool.name} failed with exit status {result.returncode}")
    return result.stdout


def identity(nacp):
    return (
        nacp[:0x200].split(b"\0", 1)[0],
        nacp[0x200:0x300].split(b"\0", 1)[0],
        nacp[0x3060:0x3070].split(b"\0", 1)[0],
    )


def verify(nsp, nstool, keys, stage, template, title_id):
    pfs = stage / "verify"
    run(nstool, ["-t", "pfs", "-x", pfs, nsp], stage)
    contents = list(pfs.iterdir())
    if len(contents) != 3 or any(p.suffix != ".nca" for p in contents):
        raise ValueError("Unexpected NSP contents")
    kinds = {}
    for index, nca in enumerate(sorted(contents)):
        part = stage / f"part-{index}"
        run(nstool, ["-t", "nca", "-k", keys, "--part0", part, nca], stage)
        if (part / "main").is_file():
            if {p.name for p in part.iterdir()} != {"main", "main.npdm"}:
                raise ValueError("Unexpected ExeFS contents")
            checked(part / "main", EXEFS["main"])
            npdm = (part / "main.npdm").read_bytes()
            aci = struct.unpack_from("<I", npdm, 0x70)[0]
            acid = struct.unpack_from("<I", npdm, 0x78)[0]
            if struct.unpack_from("<Q", npdm, aci + 0x10)[0] != title_id:
                raise ValueError("NPDM Title ID mismatch")
            for section, field in ((aci, 0x30), (acid, 0x230)):
                offset, size = struct.unpack_from("<II", npdm, section + field)
                start = section + offset
                if not size or npdm[start:start + size] != template[start:start + size]:
                    raise ValueError("NPDM capabilities mismatch")
            program = stage / "verify-program"
            run(nstool, ["-t", "nca", "-k", keys, "-x", program, nca], stage)
            if {p.name for p in program.iterdir()} != {"0", "1"}:
                raise ValueError("Unexpected program partition")
            romfs = program / "1"
            if {p.name for p in romfs.iterdir()} != {"nextNroPath", "nextArgv"}:
                raise ValueError("Unexpected RomFS contents")
            for name in ("nextNroPath", "nextArgv"):
                if (romfs / name).read_bytes() != SD_PATH:
                    raise ValueError(f"Incorrect path in {name}")
            kinds["program"] = nca.name
        elif (part / "control.nacp").is_file():
            if {p.name for p in part.iterdir()} != {"control.nacp", "icon_AmericanEnglish.dat"}:
                raise ValueError("Unexpected Control contents")
            nacp = (part / "control.nacp").read_bytes()
            if (len(nacp) != 0x4000 or identity(nacp) != (b"Reloaded-NX", b"Flowzart Studio", b"1.6.1")
                    or nacp[0x3035] or any(struct.unpack_from("<Q", nacp, offset)[0] for offset in SAVE_SIZES)):
                raise ValueError("Unexpected NACP identity, video capture or save allocation")
            checked(part / "icon_AmericanEnglish.dat", ICON_SHA)
            kinds["control"] = nca.name
        else:
            cnmt, = part.glob("*.cnmt")
            if len(list(part.iterdir())) != 1 or struct.unpack_from("<Q", cnmt.read_bytes())[0] != title_id:
                raise ValueError("Unexpected CNMT")
            kinds["meta"] = nca.name
    if set(kinds) != {"program", "control", "meta"}:
        raise ValueError("Missing NSP content")
    return kinds


def build(args):
    output = args.output.resolve()
    if output.exists():
        raise FileExistsError(output)
    keys = args.keys.resolve()
    if not keys.is_file() or keys.is_relative_to(output):
        raise ValueError("Supply a local NTON key file outside the output directory")
    assets = args.nton.resolve()
    if not (assets / "bin").is_dir():
        assets = assets / "_internal/nton"
    catalog = json.loads((assets / "assets/game_title_ids.json").read_text(encoding="utf-8"))
    title_id = args.title_id.lower()
    if (not re.fullmatch(r"01[0-9a-f]{11}000", title_id)
            or int(title_id, 16) < 0x0100000000010000
            or title_id in {key.lower() for key in catalog}):
        raise ValueError("Invalid Title ID or collision in the NTON catalog")
    nro = checked(args.nro, NRO_SHA)
    asset = struct.unpack_from("<I", nro, 0x18)[0]
    icon_offset, icon_size, nacp_offset, nacp_size = struct.unpack_from("<QQQQ", nro, asset + 8)
    icon = nro[asset + icon_offset:asset + icon_offset + icon_size]
    nacp = bytearray(nro[asset + nacp_offset:asset + nacp_offset + nacp_size])
    if len(nacp) != 0x4000 or digest(icon) != ICON_SHA or identity(nacp) != (b"Reloaded-NX", b"Flowzart Studio", b"1.6.1"):
        raise ValueError("Unexpected NRO identity or icon")
    nacp[0x3025] = nacp[0x3035] = 0
    for offset in SAVE_SIZES:
        struct.pack_into("<Q", nacp, offset, 0)
    struct.pack_into("<H", nacp, 0x3188, 0)
    for name, expected in TOOLS.items():
        checked(assets / "bin" / name, expected)
    with tempfile.TemporaryDirectory(prefix="rnx-home-", dir=output.parent) as temporary:
        stage = Path(temporary)
        for directory in ("control", "exefs", "romfs"):
            (stage / directory).mkdir()
        for name, expected in EXEFS.items():
            (stage / "exefs" / name).write_bytes(checked(assets / "assets/exefs" / name, expected))
        template = (stage / "exefs/main.npdm").read_bytes()
        for name in ("nextNroPath", "nextArgv"):
            (stage / "romfs" / name).write_bytes(SD_PATH)
        (stage / "control/control.nacp").write_bytes(nacp)
        (stage / "control/icon_AmericanEnglish.dat").write_bytes(icon)
        run(assets / "bin/hacbrewpack.exe", ["--titleid", title_id, "--nologo", "-k", keys], stage)
        nsp = stage / "hacbrewpack_nsp" / f"{title_id}.nsp"
        kinds = verify(nsp, assets / "bin/nstool.exe", keys, stage, template, int(title_id, 16))
        output.mkdir()
        filename = f"Reloaded-NX-1.6.1-rc.2-HOME-{title_id}.nsp"
        shutil.copyfile(nsp, output / filename)
    manifest = dict(title_id=title_id.upper(), title="Reloaded-NX", author="Flowzart Studio",
                    version="1.6.1", package_version="1.6.1-rc.2",
                    canonical_nro=SD_PATH.decode(), tested_nro_sha256=NRO_SHA, icon_sha256=ICON_SHA,
                    filename=filename, sha256=digest((output / filename).read_bytes()),
                    loader_revision=LOADER_REVISION, exefs=EXEFS, verified_nca=kinds,
                    physical_acceptance=False, installed_title_inventory="check locally before DBI")
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"{filename} {manifest['sha256']}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for item in ("nro", "nton", "keys", "output"):
        parser.add_argument(f"--{item}", required=True, type=Path)
    parser.add_argument("--title-id", default=TITLE_ID)
    build(parser.parse_args())
