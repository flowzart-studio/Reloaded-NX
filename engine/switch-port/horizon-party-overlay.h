/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "horizon-panel.h"

bool nx_party_overlay_open(void);
bool nx_party_overlay_draw(uint32_t width, uint32_t height, unsigned slot, bool mega);
bool nx_safari_overlay_draw(uint32_t width, uint32_t height, unsigned slot);
void nx_pointer_overlay_draw(uint32_t width, uint32_t height, unsigned x, unsigned y, bool down);
void nx_party_overlay_close(void);
bool nx_control_overlay_draw(uint32_t width, uint32_t height, const NxPanelView *view);
