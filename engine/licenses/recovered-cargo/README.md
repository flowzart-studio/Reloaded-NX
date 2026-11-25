# Recovered original package notices

These files are copied byte for byte from the upstream revisions identified in SOURCES.json. They supplement the exact published Cargo packages; they do not alter package content or `.cargo-checksum.json`. The .crate archives were independently downloaded from static.crates.io and matched against their frozen checksums.

Nineteen of the twenty-three L4 cases have original notices recovered. r-efi and r-efi-alloc place the complete grants and copyright in AUTHORS. RustySynth's original notice covers both MeltySynth (2021) and RustySynth (2022), Nobuaki Tanaka; its source archive differs from the VCS revision only in line endings. The two winapi target packages match the original i686/x86_64 subtrees in all 1,389/1,418 compared payload files; their repository manifest version is 0.3.2 while the published package is 0.4.0, so the release-time commit is corroboration of unchanged payload and original notices, not a claim of matching repository version numbers.

vex-sdk records a dirty publication tree in its own VCS metadata. All thirty comparable payload files match that revision; that observation does not prove unrecorded packaging operations never occurred. khronos_api includes Git submodule payloads not represented as ordinary blobs in the root tree; their original per-file notices remain in the checksum-verified registry archive. Keep those notices as well as the repository-level Apache license.

block, malloc_buf, objc-foundation and objc_id remain pending. Their MIT declarations are retained, but no complete historical notice was recovered. A LICENSE added to malloc_buf in 2020 by another contributor is retained only as private research; it is not automatically assigned to the older 0.0.6 release. No generic license text or invented copyright replaces these gaps.

Choosing MIT where offered does not remove additional per-file notices or relicense the GPL runner. gl_generator/khronos_api notices resolve their missing standalone notice issue only; the generated-output question remains a separate L2 issue.
