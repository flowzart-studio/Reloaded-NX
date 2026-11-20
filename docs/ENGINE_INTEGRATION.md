# Later engine integration checklist

This is a path to completing the same Reloaded-NX repository, not a new engine project or authorization to change stable 1.6.

1. Resolve L1 and L2 with verifiable evidence applicable to the exact existing payload; record required notices and compatibility analysis. Resolve the four L4 notices or document a viable corresponding-source package that excludes them, including verified build requirements.
2. Preserve the stable NRO and source provenance. Do not replace functional components, rebuild or change a version merely to clear this documentation decision.
3. Delimit every included runtime asset and preserve its original notices/preferred source where required. Keep game EXE/data, saves, keys, proprietary artwork, comparative imagery, private history and diagnostics excluded.
4. Extend this public tree only with cleared engine sources and corresponding build dependencies. Restore complete, accurate build/install guides and update LICENSE_SCOPE.md, THIRD_PARTY_NOTICES.md, inventories and both READMEs. Do not apply this edition's MIT scope to upstream engine code.
5. Reuse existing build evidence accurately. Verify correspondence and integrity of the actual release packages; do not claim bit-identical reproducibility or hardware equivalence of the prior rebuilt binary. No additional experimental build is authorized by this checklist.
6. Recheck the specific final payload for credentials/private content, required notices, link correctness and package hashes. Review any image individually.
7. Present a concrete complete-release candidate to Pedro for final upload approval. Only then create/tag/publish GitHub Release v1.6 with the actual approved assets and their hashes. A documentation commit must not be labeled as complete source distribution.
