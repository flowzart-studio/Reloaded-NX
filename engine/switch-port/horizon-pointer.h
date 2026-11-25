/* SPDX-License-Identifier: MIT */
#pragma once
#include "horizon-controls.h"

enum { NX_POINTER_NONE, NX_POINTER_BOX, NX_POINTER_MAP, NX_POINTER_FLY };

enum { NX_POINTER_VALID = 1, NX_POINTER_DOWN = 2, NX_PC_CLICK = 1 };

/* All coordinates use the HID reference grid, including in docked mode. */
typedef struct { uint16_t x, y; uint8_t flags, reserved; } NxPointerSample;
_Static_assert(sizeof(NxPointerSample) == 6, "C/Rust pointer layout");

typedef struct { uint16_t left, top, right, bottom; } NxPointerRect;
typedef struct { uint32_t id; unsigned x, y; } NxPointerFinger;
typedef struct {
    float x, y;
    uint32_t owner;
    bool touch_armed, owned;
    bool stick_armed, stick_down, stick_visible;
} NxPointer;

static inline float nx_pointer_clamp(float value, unsigned min, unsigned end) {
    if (value < min) return (float)min;
    if (value >= end) return (float)(end - 1);
    return value;
}

static inline void nx_pointer_position(NxPointer *pointer, NxPointerRect rect, float x, float y) {
    pointer->x = nx_pointer_clamp(x, rect.left, rect.right);
    pointer->y = nx_pointer_clamp(y, rect.top, rect.bottom);
}

static inline NxPointerSample nx_pointer_sample(const NxPointer *pointer, bool down) {
    return (NxPointerSample){(uint16_t)pointer->x, (uint16_t)pointer->y,
                            NX_POINTER_VALID | (down ? NX_POINTER_DOWN : 0), 0};
}

static inline NxPointerSample nx_pointer_touch(NxPointer *pointer, NxPointerRect rect,
                                              const NxPointerFinger *fingers, unsigned count) {
    if (count || pointer->owned) {
        // A touch can take over a hover, but must start a new gesture after a stick drag.
        if (pointer->stick_down) pointer->touch_armed = false;
        pointer->stick_armed = pointer->stick_down = pointer->stick_visible = false;
    }
    if (!count) {
        pointer->touch_armed = true;
        pointer->owned = false;
        return nx_pointer_sample(pointer, false);
    }
    if (pointer->touch_armed) {
        pointer->touch_armed = false;
        if (count == 1 && fingers[0].x >= rect.left && fingers[0].x < rect.right &&
            fingers[0].y >= rect.top && fingers[0].y < rect.bottom) {
            pointer->owner = fingers[0].id;
            pointer->owned = true;
        }
    }
    if (pointer->owned) {
        for (unsigned i = 0; i < count; ++i) {
            if (fingers[i].id != pointer->owner) continue;
            nx_pointer_position(pointer, rect, fingers[i].x, fingers[i].y);
            return nx_pointer_sample(pointer, true);
        }
        pointer->owned = false;
    }
    return nx_pointer_sample(pointer, false);
}

static inline float nx_pointer_velocity(int axis) {
    const int deadzone = 6500;
    float magnitude = (axis < 0 ? -axis : axis);
    if (magnitude <= deadzone) return 0;
    float normalized = (magnitude - deadzone) / (32768 - deadzone);
    return (axis < 0 ? -1 : 1) * normalized * normalized * 820;
}

static inline NxPointerSample nx_pointer_stick(NxPointer *pointer, NxPointerRect rect,
                                             int x, int y, float seconds, bool click) {
    if (!click) pointer->stick_armed = true;
    pointer->stick_down = click && pointer->stick_armed;
    if (seconds > 0.05f) seconds = 0.05f;
    const float vx = nx_pointer_velocity(x), vy = nx_pointer_velocity(y);
    if (vx || vy || pointer->stick_down) pointer->stick_visible = true;
    nx_pointer_position(pointer, rect, pointer->x + vx * seconds, pointer->y - vy * seconds);
    return nx_pointer_sample(pointer, pointer->stick_down);
}
