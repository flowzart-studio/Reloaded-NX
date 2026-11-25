/* SPDX-License-Identifier: MIT */
#pragma once
#include "horizon-profile.h"

/* Add missing game-configured actions only to free Minus chords. The source
 * profile and every explicit binding remain intact. Ignore right-stick slots:
 * they may be disabled by a pointer context. Caller always uses Normal here. */
static inline NxProfile nx_casino_profile(const NxProfile *saved, const uint8_t required[5]) {
    NxProfile result = *saved;
    result.manual_battle = 0;
    for (unsigned action = 0; action < 5; ++action) {
        uint8_t key = required[action];
        if (!key) continue;
        bool found = false;
        for (unsigned layer = 0; layer < NX_LAYERS; ++layer)
            for (unsigned source = 0; source < 16; ++source)
                if (nx_bindings(&result, false, layer)[source] == key) found = true;
        if (found) continue;
        for (unsigned source = 0; source < 16; ++source) {
            if (!result.bindings[0][NX_MINUS][source]) {
                result.bindings[0][NX_MINUS][source] = key;
                break;
            }
        }
    }
    return result;
}
