/* SPDX-License-Identifier: MIT */
#pragma once
#include "horizon-profile.h"

#define NX_PANEL_ROWS 14
#define NX_PANEL_LEFT 64
#define NX_PANEL_TOP 58
#define NX_PANEL_WIDTH 1152
#define NX_PANEL_ROW_HEIGHT 43
#define NX_PANEL_TEXT_BYTES 128

typedef struct {
    unsigned id;
    uint8_t vk;
    const char *title, *description, *key, *context, *status;
} NxAction;

typedef struct {
    char rows[NX_PANEL_ROWS][2][NX_PANEL_TEXT_BYTES];
    int row, column; /* column -1 highlights both halves. */
} NxPanelView;

typedef struct {
    NxProfile draft;
    unsigned page, selection, source, layer;
    unsigned search_cell, search_id, search_digits, catalog_return;
    bool combat, battle_mode, field_shortcuts, confirm_shortcut;
    uint8_t shortcut_vk;
    const char *status;
} NxControlPanel;

enum { NX_PANEL_STAY, NX_PANEL_BACK, NX_PANEL_CLOSE, NX_PANEL_SEND };
typedef struct { uint8_t queued, active; } NxShortcutPulse;

/* One outer event interval. Nested io_handle polls keep the same held bit. */
static inline uint8_t nx_shortcut_poll(NxShortcutPulse *pulse, bool outer) {
    if (outer) {
        pulse->active = pulse->queued;
        pulse->queued = 0;
    }
    return pulse->active;
}

int nx_control_load(NxProfile *out);
int nx_control_save(const NxProfile *profile);
unsigned nx_action_count(void);
const NxAction *nx_action(unsigned id);
const NxAction *nx_action_for_vk(uint8_t vk);
void nx_panel_begin(NxControlPanel *panel, const NxProfile *profile, const char *status, bool combat);
int nx_panel_input(NxControlPanel *panel, uint32_t pressed, NxProfile *active);
int nx_panel_touch(NxControlPanel *panel, unsigned x, unsigned y, NxProfile *active);
void nx_panel_text(const NxControlPanel *panel, const NxProfile *active, NxPanelView *view);

static inline unsigned nx_panel_list_step(unsigned index, unsigned count, bool forward) {
    if (!count || index >= count) return 0;
    if (forward) return index + 1 == count ? 0 : index + 1;
    return index ? index - 1 : count - 1;
}
