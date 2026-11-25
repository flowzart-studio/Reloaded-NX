/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "horizon-panel.h"

enum { MENU, SOURCES, CATALOG, ASSIGN, SEARCH, EXIT, SHORTCUTS };
enum { EDIT_NORMAL, EDIT_COMBAT, SPECIAL, SAVE, APPLY, RESTORE, FIELD, BATTLE_MODE, BACK, CLOSE };
enum { FIRST_ROW = 3, VISIBLE = 8 };
static const char *const buttons[NX_SOURCES] = {
    "A","B","X","Y","Cruceta izq.","Cruceta arriba","Cruceta der.","Cruceta abajo",
    "L","R","ZL","ZR","L3","R3 (clic del stick)","SL derecho","SR derecho",
    "Stick derecho arriba","Stick derecho abajo","Stick derecho izquierda","Stick derecho derecha"
};
static const char *const layer_names[NX_LAYERS] = {"Botón simple","Acorde con Minus","Acorde con ZL","Acorde con ZR"};
static const char *const menu[] = {
    [EDIT_NORMAL] = "Editar modo normal",
    [EDIT_COMBAT] = "Editar modo combate",
    [SPECIAL] = "Funciones F1-F12",
    [SAVE] = "Guardar y aplicar",
    [APPLY] = "Aplicar solo en esta sesión",
    [RESTORE] = "Restaurar perfil seguro",
    [FIELD] = "Habilidades de campo",
    [BATTLE_MODE] = "Activación de combate",
    [BACK] = "Volver sin aplicar",
    [CLOSE] = "Cerrar Reloaded NX",
};
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))
_Static_assert(FIRST_ROW + COUNT(menu) < NX_PANEL_ROWS, "panel needs room for status");
#define CELL(r,c,...) snprintf(view->rows[r][c], NX_PANEL_TEXT_BYTES, __VA_ARGS__)

void nx_panel_begin(NxControlPanel *panel, const NxProfile *profile, const char *status, bool combat) {
    *panel = (NxControlPanel){.draft = *profile, .status = status, .battle_mode = combat};
}

static const char *mode_name(const NxControlPanel *p) { return p->combat ? "Combate" : "Normal"; }
static unsigned source_count(const NxControlPanel *p) { return p->layer < NX_WITH_ZL ? NX_SOURCES : NX_SOURCES - 2; }
static unsigned source_index(const NxControlPanel *p, unsigned position) {
    return position + (p->layer >= NX_WITH_ZL && position >= 10 ? 2u : 0u);
}
static const NxAction *current(const NxControlPanel *p) {
    return nx_action_for_vk(nx_bindings(&p->draft,p->combat,p->layer)[source_index(p,p->source)]);
}

static void navigate(NxControlPanel *p, unsigned limit, uint32_t key) {
    if (!limit) { p->selection = 0; return; }
    if (key == NX_UP || key == NX_DOWN) p->selection = nx_panel_list_step(p->selection,limit,key==NX_DOWN);
    if (key == NX_L) p->selection = p->selection >= VISIBLE ? p->selection - VISIBLE : 0;
    if (key == NX_R) {
        p->selection += VISIBLE;
        if (p->selection >= limit) p->selection = limit - 1;
    }
}

static unsigned grid_step(unsigned index, unsigned count, uint32_t key) {
    unsigned column = index % 2;
    if (key == NX_LEFT || key == NX_RIGHT) return index ^ 1u;
    if (key == NX_UP || key == NX_DOWN)
        return 2 * nx_panel_list_step(index / 2, count / 2, key == NX_DOWN) + column;
    return index;
}

static const char *menu_disabled(const NxControlPanel *p, const NxProfile *active, unsigned index) {
    if (index != SPECIAL && index != FIELD) return NULL;
    if (memcmp(&p->draft,active,sizeof(*active)) != 0)
        return "Aplica o descarta el borrador antes de enviar un atajo.";
    if (index == FIELD && p->battle_mode)
        return "Habilidades de campo disponibles fuera del modo combate.";
    return NULL;
}

static void open_catalog(NxControlPanel *p) {
    p->source = p->selection;
    p->selection = current(p)->id;
    p->page = CATALOG;
}

static void open_search(NxControlPanel *p) {
    p->catalog_return = p->selection;
    p->search_id = p->search_digits = p->search_cell = 0;
    p->page = SEARCH;
    p->status = NULL;
}

static void search_submit(NxControlPanel *p) {
    if (!p->search_digits || p->search_id >= nx_action_count()) {
        p->status = "Ese ID no está en el catálogo.";
        return;
    }
    p->selection = p->search_id;
    p->page = CATALOG;
    p->status = NULL;
}

static void search_digit(NxControlPanel *p, unsigned digit) {
    if (p->search_digits < 3) {
        p->search_id = p->search_id * 10 + digit;
        ++p->search_digits;
        p->status = NULL;
    }
}

static void apply_binding(NxControlPanel *p) {
    unsigned source = source_index(p,p->source);
    uint8_t *map = p->layer < NX_WITH_ZL ? p->draft.bindings[p->combat][p->layer]
                                       : p->draft.chords[p->combat][p->layer-NX_WITH_ZL];
    map[source] = nx_action(p->selection)->vk;
    p->status = "Asignación lista. Vuelve al menú para guardar o aplicar.";
    p->page = SOURCES;
    p->selection = p->source;
}

static unsigned duplicates(const NxControlPanel *p) {
    const uint8_t *map = nx_bindings(&p->draft,p->combat,p->layer);
    const uint8_t vk = nx_action(p->selection)->vk;
    unsigned count = 0;
    if (vk) for (unsigned i=0;i<NX_SOURCES;++i)
        if (i!=source_index(p,p->source) && map[i]==vk) ++count;
    return count;
}

static unsigned shortcut_count(const NxControlPanel *p) { return p->field_shortcuts ? 8 : 12; }
static uint8_t shortcut_key(const NxControlPanel *p, unsigned index) {
    return (uint8_t)((p->field_shortcuts ? '1' : 112) + index);
}
static void open_shortcuts(NxControlPanel *p, bool field) {
    p->page = SHORTCUTS;
    p->selection = 0;
    p->field_shortcuts = field;
    p->confirm_shortcut = false;
    p->status = NULL;
}

static int shortcut_input(NxControlPanel *p, uint32_t key) {
    if (key == NX_B) {
        if (p->confirm_shortcut) p->confirm_shortcut = false;
        else { p->page = MENU; p->selection = p->field_shortcuts ? FIELD : SPECIAL; }
    } else if (key == NX_A) {
        if (p->confirm_shortcut) {
            p->shortcut_vk = shortcut_key(p,p->selection);
            return NX_PANEL_SEND;
        }
        p->confirm_shortcut = true;
    } else if (!p->confirm_shortcut) {
        p->selection = grid_step(p->selection,shortcut_count(p),key);
    }
    return NX_PANEL_STAY;
}

int nx_panel_input(NxControlPanel *p, uint32_t key, NxProfile *active) {
    if (!key || (key & (key - 1))) return NX_PANEL_STAY;
    if (p->page == SHORTCUTS) return shortcut_input(p,key);
    if (key == NX_B) {
        switch (p->page) {
        case MENU: return NX_PANEL_BACK;
        case SOURCES: p->selection = p->combat ? EDIT_COMBAT : EDIT_NORMAL; break;
        case CATALOG: p->page = SOURCES; p->selection = p->source; return NX_PANEL_STAY;
        case ASSIGN: p->page = CATALOG; return NX_PANEL_STAY;
        case SEARCH: p->page = CATALOG; p->selection = p->catalog_return; p->status = NULL; return NX_PANEL_STAY;
        case EXIT: p->selection = CLOSE; break;
        }
        p->page = MENU;
        return NX_PANEL_STAY;
    }
    if (p->page == EXIT) return key == NX_A ? NX_PANEL_CLOSE : NX_PANEL_STAY;
    switch (p->page) {
    case MENU:
        if (key == NX_UP || key == NX_DOWN) {
            do { navigate(p,COUNT(menu),key); }
            while (menu_disabled(p,active,p->selection));
        }
        if (key != NX_A) break;
        if ((p->selection == FIELD || p->selection == SPECIAL) && menu_disabled(p,active,p->selection)) {
            p->status = menu_disabled(p,active,p->selection);
            break;
        }
        switch (p->selection) {
        case EDIT_NORMAL: case EDIT_COMBAT:
            p->combat = p->selection == EDIT_COMBAT;
            p->layer = NX_SIMPLE;
            p->selection = 0;
            p->page = SOURCES;
            break;
        case SAVE:
            if (nx_control_save(&p->draft) != 0) {
                p->status = "No se pudo guardar en la SD. El perfil activo se conserva.";
                break;
            }
            *active = p->draft;
            return NX_PANEL_BACK;
        case APPLY: *active = p->draft; return NX_PANEL_BACK;
        case RESTORE:
            p->draft = nx_safe_profile;
            p->status = "Perfil seguro preparado. Guarda o aplica para activarlo.";
            break;
        case SPECIAL: case FIELD: open_shortcuts(p,p->selection == FIELD); break;
        case BATTLE_MODE:
            p->draft.manual_battle = !p->draft.manual_battle;
            p->status = "Guarda o aplica para cambiar la activación de combate.";
            break;
        case BACK: return NX_PANEL_BACK;
        case CLOSE: p->page = EXIT; break;
        }
        break;
    case SOURCES:
        navigate(p,source_count(p),key);
        if (key == NX_X) {
            p->layer = (p->layer + 1) % NX_LAYERS;
            p->selection = 0;
        }
        if (key == NX_A) open_catalog(p);
        break;
    case CATALOG:
        navigate(p,nx_action_count(),key);
        if (key == NX_X) open_search(p);
        if (key == NX_A) p->page = ASSIGN;
        break;
    case ASSIGN:
        if (key == NX_A) apply_binding(p);
        break;
    case SEARCH:
        p->search_cell = grid_step(p->search_cell,10,key);
        if (key == NX_A) search_digit(p,p->search_cell);
        if (key == NX_X) p->search_id = p->search_digits = 0;
        if (key == NX_Y) search_submit(p);
        break;
    }
    return NX_PANEL_STAY;
}

int nx_panel_touch(NxControlPanel *p, unsigned x, unsigned y, NxProfile *active) {
    if (x < NX_PANEL_LEFT || x >= NX_PANEL_LEFT + NX_PANEL_WIDTH || y < NX_PANEL_TOP) return NX_PANEL_STAY;
    unsigned row = (y - NX_PANEL_TOP) / NX_PANEL_ROW_HEIGHT;
    bool right = x >= NX_PANEL_LEFT + NX_PANEL_WIDTH / 2;
    if (p->page == MENU && row >= FIRST_ROW && row < FIRST_ROW + COUNT(menu)) {
        unsigned index = row - FIRST_ROW;
        const char *disabled = menu_disabled(p,active,index);
        if (disabled) {
            p->status = disabled;
            return NX_PANEL_STAY;
        }
        p->selection = index;
        return nx_panel_input(p,NX_A,active);
    }
    if (p->page == SHORTCUTS) {
        if (row == 11 && (right || p->confirm_shortcut))
            return shortcut_input(p,right?NX_B:NX_A);
        if (!p->confirm_shortcut && row >= FIRST_ROW && row < FIRST_ROW + shortcut_count(p) / 2) {
            p->selection = (row - FIRST_ROW) * 2 + right;
            p->confirm_shortcut = true;
        }
        return NX_PANEL_STAY;
    }
    if (row == 12 && right) return nx_panel_input(p,NX_B,active);
    if (p->page == EXIT && row == 11) return nx_panel_input(p,right?NX_B:NX_A,active);
    if (p->page == ASSIGN && row == 11) return nx_panel_input(p,right?NX_B:NX_A,active);
    if (p->page == SEARCH) {
        if (row >= 4 && row <= 8) {
            p->search_cell = (row - 4) * 2 + right;
            search_digit(p,p->search_cell);
        }
        if (row == 11) return nx_panel_input(p,right?NX_Y:NX_X,active);
    }
    if (p->page == SOURCES || p->page == CATALOG) {
        if (row == 2 && right) return nx_panel_input(p,NX_X,active);
        if (row == 11) return nx_panel_input(p,right?NX_R:NX_L,active);
        if (row >= FIRST_ROW && row < FIRST_ROW + VISIBLE) {
            unsigned selected = (p->selection / VISIBLE) * VISIBLE + row - FIRST_ROW;
            if (selected < (p->page == SOURCES ? source_count(p) : nx_action_count())) {
                p->selection = selected;
                return nx_panel_input(p,NX_A,active);
            }
        }
    }
    return NX_PANEL_STAY;
}

void nx_panel_text(const NxControlPanel *p, const NxProfile *active, NxPanelView *view) {
    *view = (NxPanelView){.row = -1, .column = -1};
    CELL(0,0,"Reloaded NX");
    CELL(0,1,"Controles");
    if (p->status) CELL(13,0,"%s",p->status);
    switch (p->page) {
    case MENU:
        CELL(1,0,"Configuración y accesos rápidos");
        CELL(1,1,"Modo activo: %s",p->battle_mode?"Combate":"Normal");
        for (unsigned i=0;i<COUNT(menu);++i) {
            CELL(i+FIRST_ROW,0,"%s",menu[i]);
            if (menu_disabled(p,active,i)) CELL(i+FIRST_ROW,1,"No disponible");
        }
        CELL(BATTLE_MODE+FIRST_ROW,1,"%s%s",p->draft.manual_battle?"Manual":"Automática",
             p->draft.manual_battle!=active->manual_battle?" (borrador)":"");
        if (!menu_disabled(p,active,p->selection)) view->row = (int)p->selection + FIRST_ROW;
        CELL(2,0,"Cruceta para elegir  ·  A para abrir");
        CELL(2,1,"B  Volver al juego");
        break;
    case SOURCES: case CATALOG: {
        bool sources = p->page == SOURCES;
        unsigned limit = sources ? source_count(p) : nx_action_count();
        unsigned start = (p->selection / VISIBLE) * VISIBLE;
        CELL(1,0,"%s  /  %s",mode_name(p),layer_names[p->layer]);
        if (sources) {
            CELL(2,0,"Botón  /  Acción asignada");
            CELL(2,1,"X  Cambiar tipo de pulsación");
            if (!p->status)
                CELL(13,0,"En PC, mapa y Vuelo, las direcciones del stick derecho mueven el puntero.");
        } else {
            const NxAction *old = current(p);
            CELL(1,1,"%s  ·  Actual #%u",buttons[source_index(p,p->source)],old->id);
            CELL(2,0,"Catálogo de acciones");
            CELL(2,1,"X  Buscar por ID");
        }
        for (unsigned i=start;i<limit && i<start+VISIBLE;++i) {
            unsigned row = i-start+FIRST_ROW;
            if (sources) {
                unsigned source = source_index(p,i);
                const NxAction *action = nx_action_for_vk(nx_bindings(&p->draft,p->combat,p->layer)[source]);
                CELL(row,0,"%s",buttons[source]);
                CELL(row,1,"#%u  %s",action->id,action->title);
            } else {
                const NxAction *action = nx_action(i);
                CELL(row,0,"#%u  %s",action->id,action->title);
                CELL(row,1,"%s",action->context);
            }
        }
        view->row = (int)(p->selection-start)+FIRST_ROW;
        CELL(11,0,"L  Página anterior  ·  %u de %u",start/VISIBLE+1,(limit+VISIBLE-1)/VISIBLE);
        CELL(11,1,"R  Página siguiente");
        CELL(12,0,"%s",sources?"A  Ver catálogo":"A  Revisar asignación");
        CELL(12,1,"B  Atrás");
        if (!sources) CELL(13,0,"%s",nx_action(p->selection)->description);
        break;
    }
    case ASSIGN: {
        const NxAction *old = current(p);
        const NxAction *next = nx_action(p->selection);
        unsigned shared = duplicates(p);
        CELL(1,0,"Revisar asignación");
        CELL(2,0,"%s  /  %s  /  %s",mode_name(p),layer_names[p->layer],buttons[source_index(p,p->source)]);
        CELL(4,0,"Antes:  #%u  %s",old->id,old->title);
        CELL(6,0,"Después:  #%u  %s",next->id,next->title);
        CELL(7,0,"%s",next->description);
        CELL(8,0,"%s  ·  %s",next->context,next->status);
        if (shared) CELL(9,0,"Esta acción ya está en %u control(es) de esta capa.",shared);
        if (p->layer >= NX_WITH_ZL && next->vk) {
            unsigned source = source_index(p,p->source);
            if (p->draft.chords[p->combat][p->layer == NX_WITH_ZL][source])
                CELL(10,0,"Ambos gatillos tienen acorde con este botón. ZL tiene prioridad.");
            else
                CELL(10,0,"%s solo se activa al soltarlo si no usas el acorde.",p->layer==NX_WITH_ZL?"ZL":"ZR");
        }
        CELL(11,0,"A  Asignar al borrador");
        CELL(11,1,"B  Cancelar");
        view->row = 11;
        view->column = 0;
        break;
    }
    case SEARCH:
        CELL(1,0,"Buscar una acción por número");
        if (p->search_digits) CELL(2,0,"ID: %0*u",(int)(p->search_digits%4),p->search_id);
        else CELL(2,0,"ID: -");
        for (unsigned i=0;i<10;++i) CELL(4+i/2,i%2,"%u",i);
        view->row = (int)(4+p->search_cell/2);
        view->column = (int)(p->search_cell%2);
        CELL(11,0,"X  Limpiar");
        CELL(11,1,"Y  Ir al ID");
        CELL(12,0,"A  Añadir dígito");
        CELL(12,1,"B  Volver al catálogo");
        break;
    case EXIT:
        CELL(4,0,"¿Cerrar Reloaded NX?");
        CELL(6,0,"Confirma con otra pulsación de A.");
        CELL(11,0,"A  Cerrar");
        CELL(11,1,"B  Volver");
        break;
    case SHORTCUTS:
        CELL(1,0,"%s",p->field_shortcuts?"Habilidades de campo":"Funciones F1-F12");
        for (unsigned i=0;i<shortcut_count(p);++i) {
            const NxAction *action = nx_action_for_vk(shortcut_key(p,i));
            CELL(FIRST_ROW+i/2,i%2,"%s   %s",action->key,action->title);
        }
        view->row = (int)(FIRST_ROW+p->selection/2);
        view->column = (int)(p->selection%2);
        if (p->confirm_shortcut) {
            const NxAction *action = nx_action_for_vk(shortcut_key(p,p->selection));
            CELL(10,0,"¿Enviar %s  ·  %s?",action->key,action->title);
            CELL(11,0,"A  Enviar y volver");
            CELL(11,1,"B  Cancelar");
        } else {
            CELL(11,1,"B  Volver sin enviar");
            CELL(12,0,"Toca una acción o usa la cruceta y A.");
        }
        break;
    }
}
