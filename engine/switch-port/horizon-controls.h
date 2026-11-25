/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Shared geometry for the handheld HID coordinates and the fullscreen shader.
 * Logical GM window sizes and the game's letterboxing are independent. */
#define NX_TOUCH_WIDTH 1280
#define NX_TOUCH_HEIGHT 720
#define NX_PARTY_LEFT 1180
#define NX_PARTY_TOP 80
#define NX_PARTY_SIZE 80
#define NX_PARTY_PITCH 96
#define NX_PARTY_COUNT 6
#define NX_SAFARI_COUNT 5
#define NX_FLEE_SLOT 7
#define NX_FLEE_LEFT 1000
#define NX_FLEE_TOP 596
#define NX_FLEE_WIDTH 158
#define NX_FLEE_HEIGHT 60
#define NX_MEGA_SLOT 8
#define NX_MEGA_TOP 524
#define NX_MEGA_KEY 'M'

enum {
    NX_A = 1u << 0, NX_B = 1u << 1, NX_X = 1u << 2, NX_Y = 1u << 3,
    NX_LEFT = 1u << 4, NX_UP = 1u << 5, NX_RIGHT = 1u << 6, NX_DOWN = 1u << 7,
    NX_L = 1u << 8, NX_R = 1u << 9, NX_ZL = 1u << 10, NX_ZR = 1u << 11,
    NX_L3 = 1u << 12, NX_R3 = 1u << 13,
    NX_RIGHT_SL = 1u << 14, NX_RIGHT_SR = 1u << 15,
    NX_RS_UP = 1u << 16, NX_RS_DOWN = 1u << 17,
    NX_RS_LEFT = 1u << 18, NX_RS_RIGHT = 1u << 19,
    NX_RS_DIRECTIONS = NX_RS_UP | NX_RS_DOWN | NX_RS_LEFT | NX_RS_RIGHT,
};

typedef struct {
    uint32_t directions;
    bool armed;
} NxRightStick;

typedef struct {
    bool down, pending;
} NxMegaInput;

/* Render-only polls can see a complete tap before the next GML Step. */
static inline void nx_mega_keep_until_step(NxMegaInput *input, bool battle, uint64_t keys[4]) {
    if (!battle) {
        *input = (NxMegaInput){0};
        return;
    }
    const uint64_t mask = UINT64_C(1) << (NX_MEGA_KEY % 64);
    uint64_t *word = &keys[NX_MEGA_KEY / 64];
    bool down = (*word & mask) != 0;
    input->pending |= down && !input->down;
    input->down = down;
    if (input->pending) *word |= mask;
}

static inline uint32_t nx_stick_axis(int value, uint32_t previous, uint32_t negative, uint32_t positive) {
    const int press = 16000;   /* About 49% of libnx's 32767 full scale. */
    const int release = 11000;
    if (value >= (previous & positive ? release : press)) return positive;
    if (value <= -(previous & negative ? release : press)) return negative;
    return 0;
}

static inline uint32_t nx_right_stick_keys(NxRightStick *stick, bool enabled, int x, int y) {
    if (!enabled) {
        *stick = (NxRightStick){0};
        return 0;
    }
    if (!stick->armed) {
        const int neutral = 8000;
        stick->armed = x > -neutral && x < neutral && y > -neutral && y < neutral;
        return 0;
    }
    stick->directions = nx_stick_axis(x, stick->directions, NX_RS_LEFT, NX_RS_RIGHT)
                      | nx_stick_axis(y, stick->directions, NX_RS_DOWN, NX_RS_UP);
    return stick->directions;
}

typedef struct {
    bool touch_armed;
    unsigned slot; /* 0 = no key, 1–6 = party, 7 = flee, 8 = Mega. */
    uint32_t finger;
} NxControls;

static inline bool nx_touch_begin(bool *armed, bool available, unsigned count) {
    bool began = available && count == 1 && *armed;
    *armed = available && count == 0;
    return began;
}

static inline unsigned nx_party_hit(unsigned x, unsigned y) {
    if (x >= NX_FLEE_LEFT && x < NX_FLEE_LEFT + NX_FLEE_WIDTH &&
        y >= NX_FLEE_TOP && y < NX_FLEE_TOP + NX_FLEE_HEIGHT)
        return NX_FLEE_SLOT;
    if (x < NX_PARTY_LEFT || x >= NX_PARTY_LEFT + NX_PARTY_SIZE || y < NX_PARTY_TOP)
        return 0;
    y -= NX_PARTY_TOP;
    unsigned slot = y / NX_PARTY_PITCH;
    return slot < NX_PARTY_COUNT && y % NX_PARTY_PITCH < NX_PARTY_SIZE ? slot + 1 : 0;
}

static inline unsigned nx_controls_touch_layout(NxControls *controls, bool active, unsigned count,
                                         uint32_t finger, unsigned x, unsigned y, bool safari, bool mega) {
    if (!active || count != 1) {
        controls->slot = 0;
        controls->touch_armed = active && count == 0;
    } else if (controls->touch_armed) {
        controls->touch_armed = false;
        controls->finger = finger;
        controls->slot = mega && !safari &&
            x >= NX_FLEE_LEFT && x < NX_FLEE_LEFT + NX_FLEE_WIDTH &&
            y >= NX_MEGA_TOP && y < NX_MEGA_TOP + NX_FLEE_HEIGHT
            ? NX_MEGA_SLOT : nx_party_hit(x, y);
        if (safari && controls->slot > NX_SAFARI_COUNT && controls->slot != NX_FLEE_SLOT)
            controls->slot = 0;
    } else if (controls->finger != finger || (!mega && controls->slot == NX_MEGA_SLOT)) {
        controls->slot = 0;
    }
    /* The initial hit owns the gesture until release. Sliding, starting
     * outside the column and overlapping fingers cannot create another key. */
    return controls->slot;
}

static inline unsigned nx_controls_touch(NxControls *controls, bool active, unsigned count,
                                         uint32_t finger, unsigned x, unsigned y) {
    return nx_controls_touch_layout(controls, active, count, finger, x, y, false, false);
}

static inline uint8_t nx_safari_touch_vk(unsigned slot) {
    static const uint8_t keys[NX_SAFARI_COUNT] = {'B', 'V', 'P', 'Z', 'X'};
    if (slot == NX_FLEE_SLOT) return 'E';
    return slot && slot <= NX_SAFARI_COUNT ? keys[slot - 1] : 0;
}

static inline uint8_t nx_battle_touch_vk(unsigned slot) {
    if (slot == NX_MEGA_SLOT) return NX_MEGA_KEY;
    if (slot == NX_FLEE_SLOT) return 'E';
    return slot && slot <= NX_PARTY_COUNT ? (uint8_t)('0'+slot) : 0;
}
