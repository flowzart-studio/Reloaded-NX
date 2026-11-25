/* SPDX-License-Identifier: MIT */
#pragma once
#include "horizon-controls.h"

enum { NX_SOURCES = 20 };
enum { NX_SIMPLE, NX_MINUS, NX_WITH_ZL, NX_WITH_ZR, NX_LAYERS };
typedef struct {
    uint8_t bindings[2][2][NX_SOURCES]; /* Normal/combat, base/Minus chord. */
    uint8_t chords[2][2][NX_SOURCES];   /* Normal/combat, ZL/ZR chord. */
    uint8_t manual_battle;
} NxProfile;
_Static_assert(sizeof(NxProfile) == 161, "C/Rust profile layout");

static const NxProfile nx_safe_profile = {.bindings = {
    {{13,'X',32,'Z',37,38,39,40,16,'X','A','B','O','M'},
     {0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{'V','X','C','Z',37,38,39,40,16,13,'A','B','S','D'},
     {0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
}};

typedef struct {
    int32_t room;
    bool active;
} NxBattleContext;

static inline void nx_battle_context(NxBattleContext *state, int32_t room,
                                     bool eligible, bool ready) {
    if (!eligible || state->room != room) state->active = false;
    if (eligible && ready) state->active = true;
    state->room = room;
}

typedef struct {
    uint32_t previous_pad, blocked, trigger_taps;
    uint32_t safari_pending, safari_latched;
    uint64_t safari_pulses[4];
    uint8_t active[NX_SOURCES], layer[NX_SOURCES];
    bool minus_down, used_chord, armed;
} NxMapper;

static inline const uint8_t *nx_bindings(const NxProfile *profile, bool combat, unsigned layer) {
    return layer < NX_WITH_ZL ? profile->bindings[combat][layer] : profile->chords[combat][layer - NX_WITH_ZL];
}

static inline bool nx_combat_has_key(const NxProfile *profile, uint8_t key) {
    for (unsigned layer = 0; layer < NX_LAYERS; ++layer) {
        const uint8_t *bindings = nx_bindings(profile, true, layer);
        for (unsigned button = 0; button < NX_SOURCES; ++button)
            if (bindings[button] == key) return true;
    }
    return false;
}

/* Safari uses the saved combat bindings. Supply missing actions on empty
 * Minus chords for this encounter only, leaving the stored profile intact. */
static inline NxProfile nx_safari_profile(const NxProfile *profile) {
    NxProfile result = *profile;
    result.manual_battle = 0;
    const uint8_t keys[] = {'B', 'V'};
    const unsigned buttons[] = {11, 0}; /* ZR, A */
    for (unsigned action = 0; action < sizeof(keys); ++action) {
        uint8_t *fallback = &result.bindings[1][NX_MINUS][buttons[action]];
        if (!*fallback && !nx_combat_has_key(profile, keys[action])) *fallback = keys[action];
    }
    return result;
}

static const struct {
    uint32_t buttons;
    uint8_t key;
    bool hold;
} nx_safari_chords[] = {
    {NX_L | NX_R, 'E', false},
    {NX_L3 | NX_R3, 'P', true},
};

static inline uint32_t nx_safari_chord_buttons(const NxProfile *profile) {
    uint32_t enabled = 0;
    for (unsigned i = 0; i < sizeof(nx_safari_chords)/sizeof(nx_safari_chords[0]); ++i) {
        uint32_t buttons = nx_safari_chords[i].buttons;
        if (nx_combat_has_key(profile, nx_safari_chords[i].key)) continue;
        /* Reserve only stock buttons. Explicit edits to either member in
         * any layer keep their original timing and take precedence. */
        for (unsigned layer = 0; layer < NX_LAYERS; ++layer) {
            const uint8_t *bindings = nx_bindings(profile, true, layer);
            const uint8_t *defaults = nx_bindings(&nx_safe_profile, true, layer);
            for (unsigned button = 0; button < NX_SOURCES; ++button)
                if ((buttons & (1u << button)) && bindings[button] != defaults[button])
                    buttons = 0;
        }
        enabled |= buttons;
    }
    return enabled;
}

static inline void nx_key_add(uint64_t keys[4], uint8_t vk) {
    if (vk) keys[vk / 64] |= UINT64_C(1) << (vk % 64);
}

static inline void nx_mapper_poll(NxMapper *mapper, const NxProfile *profile, bool *combat,
                                   uint32_t pad, bool minus, bool stick_keys, uint64_t keys[4]) {
    if (!stick_keys) pad &= ~NX_RS_DIRECTIONS;
    if (!mapper->armed) {
        mapper->armed = !pad && !minus;
        return;
    }
    if (minus && !mapper->minus_down) mapper->used_chord = pad != 0;
    if (minus && pad) mapper->used_chord = true;
    bool next_combat = *combat;
    if (profile->manual_battle && !minus && mapper->minus_down && !mapper->used_chord) next_combat = !*combat;
    const uint32_t triggers[] = {NX_ZL,NX_ZR};
    uint32_t targets[2] = {0};
    uint32_t changed = pad ^ mapper->previous_pad;
    if (minus || mapper->minus_down || next_combat != *combat) mapper->trigger_taps = 0;
    for (unsigned t = 0; t < 2; ++t) {
        for (unsigned i = 0; i < NX_SOURCES; ++i) {
            uint32_t bit = 1u << i;
            if (profile->chords[next_combat][t][i] && (stick_keys || !(bit & NX_RS_DIRECTIONS)))
                targets[t] |= bit;
        }
        if (!targets[t] || minus || !(changed & triggers[t])) continue;
        if (pad & triggers[t]) {
            if (!(mapper->previous_pad & targets[t])) mapper->trigger_taps |= triggers[t];
        } else if (mapper->trigger_taps & triggers[t]) {
            nx_key_add(keys,profile->bindings[next_combat][NX_SIMPLE][10+t]);
            mapper->trigger_taps &= ~triggers[t];
        }
    }
    mapper->blocked &= pad;
    for (unsigned i = 0; i < NX_SOURCES; ++i) {
        uint32_t bit = 1u << i;
        if (!(pad & bit)) {
            mapper->active[i] = 0;
            continue;
        }
        if (mapper->previous_pad & bit) {
            unsigned layer = mapper->layer[i];
            if (layer >= NX_WITH_ZL) {
                if (minus || !(pad & triggers[layer-NX_WITH_ZL])) mapper->blocked |= bit;
            } else if (mapper->active[i] != profile->bindings[next_combat][minus][i]) {
                mapper->blocked |= bit;
            }
        } else {
            unsigned layer = minus ? NX_MINUS : NX_SIMPLE;
            if (!minus) {
                for (unsigned t = 0; t < 2; ++t) {
                    if (!(pad & triggers[t]) || !(targets[t] & bit)) continue;
                    if (layer == NX_SIMPLE) layer = NX_WITH_ZL + t;
                    mapper->trigger_taps &= ~triggers[t];
                }
            }
            mapper->layer[i] = (uint8_t)layer;
            mapper->active[i] = nx_bindings(profile,next_combat,layer)[i];
            if (!minus && (bit == NX_ZL || bit == NX_ZR) && targets[i-10]) {
                mapper->active[i] = 0;
                mapper->blocked |= bit;
            }
        }
        if (!(mapper->blocked & bit)) nx_key_add(keys, mapper->active[i]);
    }
    mapper->previous_pad = pad;
    mapper->minus_down = minus;
    *combat = next_combat;
}

static inline void nx_safari_mapper_poll(NxMapper *mapper, const NxProfile *profile,
                                          uint32_t chord_buttons, uint32_t pad, bool minus, bool stick_keys,
                                          uint64_t keys[4]) {
    bool combat = true;
    if (!mapper->armed) {
        nx_mapper_poll(mapper, profile, &combat, pad, minus, stick_keys, keys);
        return;
    }
    for (unsigned i = 0; i < sizeof(nx_safari_chords)/sizeof(nx_safari_chords[0]); ++i) {
        uint32_t buttons = nx_safari_chords[i].buttons;
        if (!(chord_buttons & buttons)) continue;
        uint32_t down = pad & buttons;
        if (down == buttons) {
            /* Drop any solo taps still queued for this pair before publishing
             * the chord. A queued Enter could otherwise accept Huir. */
            for (unsigned button = 0; button < NX_SOURCES; ++button) {
                if (!(buttons & (1u << button))) continue;
                uint8_t vk = profile->bindings[1][NX_SIMPLE][button];
                mapper->safari_pulses[vk / 64] &= ~(UINT64_C(1) << (vk % 64));
            }
            if (nx_safari_chords[i].hold)
                nx_key_add(keys, nx_safari_chords[i].key);
            else if (!(mapper->safari_latched & buttons))
                nx_key_add(mapper->safari_pulses, nx_safari_chords[i].key);
            mapper->safari_latched |= buttons;
        }
        /* Delay standalone taps until release so the first member of an
         * eventual chord cannot leak an action (notably R sends Enter). */
        if (!(mapper->safari_latched & buttons) && !minus) {
            uint32_t released = mapper->safari_pending & buttons & ~down;
            for (unsigned button = 0; button < NX_SOURCES; ++button)
                if (released & (1u << button))
                    nx_key_add(mapper->safari_pulses, profile->bindings[1][NX_SIMPLE][button]);
        }
        mapper->safari_pending &= ~buttons;
        if (!minus && !(mapper->safari_latched & buttons))
            mapper->safari_pending |= down;
        if (!down) mapper->safari_latched &= ~buttons;
    }
    nx_mapper_poll(mapper, profile, &combat, pad & ~chord_buttons, minus, stick_keys, keys);
    /* Render-only iterations can poll more than once before GML Step. */
    for (unsigned word = 0; word < 4; ++word)
        keys[word] |= mapper->safari_pulses[word];
}

static inline void nx_safari_step_finished(NxMapper *mapper) {
    for (unsigned word = 0; word < 4; ++word)
        mapper->safari_pulses[word] = 0;
}
