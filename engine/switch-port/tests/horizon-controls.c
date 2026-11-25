/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../horizon-panel.h"

static uint64_t keys[4];
static bool combat;
static bool held(uint8_t vk) { return keys[vk / 64] & (UINT64_C(1) << (vk % 64)); }
static void poll(NxMapper *mapper, const NxProfile *profile, uint32_t pad, bool minus) {
    memset(keys,0,sizeof(keys));
    nx_mapper_poll(mapper,profile,&combat,pad,minus,true,keys);
}

static void test_pad(void) {
    NxMapper mapper = {0};
    NxProfile profile = nx_safe_profile;
    profile.manual_battle = 1;
    profile.bindings[0][1][0] = 121; /* Minus+A -> F10. */
    poll(&mapper,&profile,NX_A,false);
    assert(!held(13));
    poll(&mapper,&profile,0,false);
    poll(&mapper,&profile,NX_B | NX_R,false);
    assert(held('X') && !held(27));
    poll(&mapper,&profile,NX_R,false);
    assert(held('X')); /* B releases, R still owns X. */
    poll(&mapper,&profile,0,false);
    assert(!held('X'));
    poll(&mapper,&profile,0,true);
    poll(&mapper,&profile,NX_A,true);
    assert(held(121) && !held(13) && !combat);
    poll(&mapper,&profile,NX_A,false);
    assert(!held(121) && !held(13) && !combat);
    poll(&mapper,&profile,NX_A,false);
    assert(!held(13));
    poll(&mapper,&profile,0,false);
    poll(&mapper,&profile,NX_A,false);
    assert(held(13));
    poll(&mapper,&profile,NX_A,true);
    assert(!held(121)); /* Button first cannot create a new chord edge. */
    poll(&mapper,&profile,0,false);
    assert(!combat);
    poll(&mapper,&profile,0,true);
    poll(&mapper,&profile,0,false);
    assert(combat);
    poll(&mapper,&profile,NX_ZL | NX_ZR | NX_A | NX_B | NX_X | NX_Y | NX_UP,false);
    assert(held('A') && held('B') && held('V') && held('X') && held('C') && held('Z') && held(38));
    mapper = (NxMapper){0}; /* Same barrier as focus or modal transition. */
    poll(&mapper,&profile,NX_ZL,false);
    assert(!held('A'));
    poll(&mapper,&profile,0,false);
    poll(&mapper,&profile,NX_ZL,false);
    assert(held('A'));

    profile.bindings[1][0][0] = '1';
    poll(&mapper,&profile,NX_A,false);
    nx_key_add(keys,'1'); /* Physical key and touchscreen share one output bit. */
    assert(held('1'));
    poll(&mapper,&profile,0,false);
    nx_key_add(keys,'1');
    assert(held('1'));
    poll(&mapper,&profile,0,false);
    assert(!held('1'));
}

static void test_touch(void) {
    NxControls controls = {0};
    const unsigned x = NX_PARTY_LEFT, y = NX_PARTY_TOP;
    assert(!nx_party_hit(x - 1, y));
    assert(!nx_party_hit(x + NX_PARTY_SIZE, y));
    assert(!nx_party_hit(x, y - 1));
    assert(!nx_party_hit(x, y + 5 * NX_PARTY_PITCH + NX_PARTY_SIZE));
    assert(!nx_controls_touch(&controls, true, 1, 7, x, y)); /* Already held on enable. */
    for (unsigned i = 0; i < NX_PARTY_COUNT; ++i) {
        const unsigned top = y + i * NX_PARTY_PITCH;
        const unsigned slot = i + 1;
        assert(nx_party_hit(x + NX_PARTY_SIZE - 1, top + NX_PARTY_SIZE - 1) == i + 1);
        assert(!nx_party_hit(x, top + NX_PARTY_SIZE));
        assert(!nx_controls_touch(&controls, true, 0, 0, 0, 0));
        assert(nx_controls_touch(&controls, true, 1, 7, x, top) == slot);
        assert(nx_controls_touch(&controls, true, 1, 7, x, top) == slot);
        assert(nx_controls_touch(&controls, true, 1, 7, x + NX_PARTY_SIZE, y) == slot);
        assert(!nx_controls_touch(&controls, true, 0, 0, 0, 0));
        assert(nx_controls_touch(&controls, true, 1, 7, x, top) == slot);
        assert(!nx_controls_touch(&controls, true, 1, 8, x, top));
    }
    assert(!nx_controls_touch(&controls, true, 0, 0, 0, 0));
    assert(!nx_controls_touch(&controls, true, 1, 7, x - 1, y));
    assert(!nx_controls_touch(&controls, true, 1, 7, x, y));
    assert(!nx_controls_touch(&controls, true, 0, 0, 0, 0));
    assert(nx_controls_touch(&controls, true, 1, 7, x, y));
    assert(!nx_controls_touch(&controls, true, 2, 7, x, y));
    assert(!nx_controls_touch(&controls, true, 1, 7, x, y));
    assert(!nx_controls_touch(&controls, true, 0, 0, 0, 0));
    assert(nx_controls_touch(&controls, true, 1, 7, x, y) == 1);
    assert(!nx_controls_touch(&controls, false, 1, 7, x, y)); /* Focus/mode loss. */
    assert(!nx_controls_touch(&controls, true, 1, 7, x, y));
    const unsigned fx=NX_FLEE_LEFT+5, fy=NX_FLEE_TOP+5;
    assert(!nx_controls_touch(&controls,true,0,0,0,0));
    assert(nx_controls_touch(&controls,true,1,9,fx,fy)==NX_FLEE_SLOT);
    assert(nx_battle_touch_vk(NX_FLEE_SLOT)=='E' && nx_battle_touch_vk(6)=='6');
    assert(nx_controls_touch(&controls,true,1,9,x,y)==NX_FLEE_SLOT);
    assert(!nx_controls_touch(&controls,true,2,9,fx,fy));
    assert(!nx_controls_touch(&controls,true,1,9,fx,fy));
    assert(!nx_controls_touch(&controls,true,0,0,0,0));
    assert(nx_controls_touch(&controls,true,1,9,fx,fy)==NX_FLEE_SLOT);
    assert(!nx_controls_touch(&controls,false,1,9,fx,fy));
}

static bool save_fails;
int nx_control_save(const NxProfile *profile) { (void)profile; return save_fails ? -1 : 0; }

static int tap(NxControlPanel *panel, NxProfile *profile, unsigned column, unsigned row) {
    return nx_panel_touch(panel,NX_PANEL_LEFT + column * (NX_PANEL_WIDTH/64),
                          NX_PANEL_TOP + row * NX_PANEL_ROW_HEIGHT,profile);
}

static void choose_id(NxControlPanel *panel, NxProfile *profile, unsigned id) {
    char digits[4];
    assert(id<1000);
    snprintf(digits,sizeof(digits),"%u",id);
    nx_panel_input(panel,NX_X,profile);
    for (const char *p=digits;*p;++p) {
        unsigned digit=(unsigned)(*p-'0');
        tap(panel,profile,(digit%2)*32,4+digit/2);
    }
    tap(panel,profile,32,11);
    nx_panel_input(panel,NX_A,profile);
    nx_panel_input(panel,NX_A,profile);
}

static void test_right_stick(void) {
    NxProfile active = nx_safe_profile;
    NxControlPanel panel;
    NxMapper mapper = {0};
    NxRightStick stick = {0};
    const uint8_t actions[] = {NX_MEGA_KEY, 'P', 'E', 'S'};
    nx_panel_begin(&panel,&active,NULL,false);
    nx_panel_input(&panel,NX_DOWN,&active);
    nx_panel_input(&panel,NX_A,&active); /* Combate */
    nx_panel_input(&panel,NX_R,&active);
    nx_panel_input(&panel,NX_R,&active);
    for (unsigned i=0; i<sizeof(actions); ++i) {
        tap(&panel,&active,1,3+i);
        choose_id(&panel,&active,nx_action_for_vk(actions[i])->id);
        assert(panel.draft.bindings[1][NX_SIMPLE][16+i]==actions[i]);
    }
    assert(nx_action(23)->vk==NX_MEGA_KEY && nx_action(84)->vk==1);
    assert(panel.draft.bindings[1][NX_SIMPLE][13]=='D');
    /* Assign the same Mega action to a physical face button through the editor. */
    panel.selection=0;
    tap(&panel,&active,1,3);
    choose_id(&panel,&active,23);
    assert(panel.draft.bindings[1][NX_SIMPLE][0]==NX_MEGA_KEY);
    active=panel.draft;
    combat=true;
    poll(&mapper,&active,0,false);
    poll(&mapper,&active,NX_A|NX_R3,false);
    assert(held(NX_MEGA_KEY) && held('D'));
    poll(&mapper,&active,0,false);
    assert(!held(NX_MEGA_KEY));

    assert(!nx_right_stick_keys(&stick,true,0,22000)); /* Held upon entering. */
    assert(!nx_right_stick_keys(&stick,true,0,8000));
    assert(!nx_right_stick_keys(&stick,true,0,7999));
    assert(!nx_right_stick_keys(&stick,true,15999,15999));
    uint32_t direction=nx_right_stick_keys(&stick,true,16000,16000);
    assert(direction==(NX_RS_RIGHT|NX_RS_UP));
    poll(&mapper,&active,direction|NX_R3|NX_Y,false);
    assert(held('S') && held(NX_MEGA_KEY) && held('D') && held('Z'));
    direction=nx_right_stick_keys(&stick,true,15000,11000);
    assert(direction==(NX_RS_RIGHT|NX_RS_UP)); /* Hysteresis on both axes. */
    direction=nx_right_stick_keys(&stick,true,-22000,10999);
    assert(direction==NX_RS_LEFT);
    poll(&mapper,&active,direction,false);
    assert(held('E') && !held(NX_MEGA_KEY) && !held('S') && !held('D'));
    direction=nx_right_stick_keys(&stick,true,0,-26000);
    assert(direction==NX_RS_DOWN);
    poll(&mapper,&active,direction,false);
    assert(held('P') && !held('E'));

    assert(!nx_right_stick_keys(&stick,false,0,-26000)); /* PC, map or Vuelo. */
    assert(!nx_right_stick_keys(&stick,true,0,-26000)); /* Exit with stick held. */
    assert(!nx_right_stick_keys(&stick,true,0,0));
    assert(nx_right_stick_keys(&stick,true,0,-26000)==NX_RS_DOWN);

    active.chords[1][0][16]='F';
    memset(keys,0,sizeof(keys));
    nx_mapper_poll(&mapper,&active,&combat,NX_ZL|NX_RS_UP|NX_R3,false,false,keys);
    assert(held('A') && held('D') && !held('F') && !held(NX_MEGA_KEY));
    poll(&mapper,&active,0,false);
    poll(&mapper,&active,NX_ZL|NX_RS_UP,false);
    assert(held('F') && !held('A') && !held(NX_MEGA_KEY));
    poll(&mapper,&active,NX_RS_UP,false);
    assert(!held('F') && !held(NX_MEGA_KEY));
    combat=false;
}

static void test_mega_touch(void) {
    NxControls touch = {0};
    NxMegaInput input = {0};
    const unsigned x=NX_FLEE_LEFT, y=NX_MEGA_TOP;
    assert(!nx_controls_touch_layout(&touch,true,0,0,0,0,true,false));
    assert(!nx_controls_touch_layout(&touch,true,1,1,x,y,true,false));
    assert(!nx_controls_touch_layout(&touch,true,1,1,x,y,false,true));
    assert(!nx_controls_touch_layout(&touch,true,0,0,0,0,false,true));
    assert(nx_controls_touch_layout(&touch,true,1,1,x,y,false,true)==NX_MEGA_SLOT);
    assert(nx_controls_touch_layout(&touch,true,1,1,x,NX_FLEE_TOP,false,true)==NX_MEGA_SLOT);
    memset(keys,0,sizeof(keys));
    nx_key_add(keys,nx_battle_touch_vk(touch.slot));
    nx_mega_keep_until_step(&input,true,keys);
    assert(held(NX_MEGA_KEY) && input.pending);
    assert(!nx_controls_touch_layout(&touch,true,0,0,0,0,false,true));
    memset(keys,0,sizeof(keys));
    nx_mega_keep_until_step(&input,true,keys);
    assert(held(NX_MEGA_KEY)); /* Tap released during interpolation. */
    input.pending=false;
    memset(keys,0,sizeof(keys));
    nx_mega_keep_until_step(&input,true,keys);
    assert(!held(NX_MEGA_KEY));
    assert(nx_controls_touch_layout(&touch,true,1,2,x,NX_FLEE_TOP,false,true)==NX_FLEE_SLOT);
    assert(nx_controls_touch_layout(&touch,true,1,2,x,y,false,true)==NX_FLEE_SLOT);
    assert(!nx_controls_touch_layout(&touch,true,2,2,x,y,false,true));
    assert(!nx_controls_touch_layout(&touch,true,1,2,x,y,false,true));
}

static void test_panel(void) {
    NxProfile active = nx_safe_profile;
    NxControlPanel panel;
    bool vks[256]={0};
    for (unsigned i=0;i<nx_action_count();++i) {
        const NxAction *action=nx_action(i);
        assert(action->id==i && !vks[action->vk]);
        vks[action->vk]=true;
        assert(nx_action_for_vk(action->vk)==action);
        assert(action->title[0] && action->description[0] && action->context[0] && action->status[0]);
    }
    assert(nx_action(0)->vk==0);
    nx_panel_begin(&panel,&active,NULL,false);
    nx_panel_input(&panel,NX_A,&active); /* Normal. */
    nx_panel_input(&panel,NX_A,&active); /* Switch A. */
    choose_id(&panel,&active,nx_action_for_vk(32)->id);
    tap(&panel,&active,1,5); /* Switch X. */
    choose_id(&panel,&active,nx_action_for_vk(13)->id);
    tap(&panel,&active,1,4); /* Switch B. */
    choose_id(&panel,&active,0);
    assert(panel.draft.bindings[0][0][0]==32 && panel.draft.bindings[0][0][2]==13);
    assert(panel.draft.bindings[0][0][1]==0 && active.bindings[0][0][1]=='X');
    nx_panel_input(&panel,NX_X,&active); /* Minus. */
    nx_panel_input(&panel,NX_X,&active); /* ZL. */
    tap(&panel,&active,1,5); /* ZL + X. */
    for(unsigned key=112;key<=123;++key) {
        if(key!=112) tap(&panel,&active,1,5);
        choose_id(&panel,&active,nx_action_for_vk((uint8_t)key)->id);
        assert(panel.draft.chords[0][0][2]==key);
    }
    tap(&panel,&active,1,3);
    for(unsigned i=0;i<nx_action_count();i+=8) nx_panel_input(&panel,NX_R,&active);
    assert(panel.selection==nx_action_count()-1);
    nx_panel_input(&panel,NX_B,&active); /* Back to sources. */
    nx_panel_input(&panel,NX_B,&active);
    nx_panel_input(&panel,NX_DOWN,&active);
    nx_panel_input(&panel,NX_DOWN,&active);
    save_fails=true;
    assert(nx_panel_input(&panel,NX_A,&active)==NX_PANEL_STAY && active.bindings[0][0][1]=='X');
    save_fails=false;
    assert(nx_panel_input(&panel,NX_A,&active)==NX_PANEL_BACK && active.bindings[0][0][1]==0);
    assert(active.chords[0][0][2]==123);
    nx_panel_begin(&panel,&active,NULL,false);
    tap(&panel,&active,1,8); /* Restore. */
    assert(memcmp(&panel.draft,&nx_safe_profile,sizeof(NxProfile))==0);
    assert(tap(&panel,&active,1,7)==NX_PANEL_BACK); /* Apply in memory. */
    assert(memcmp(&active,&nx_safe_profile,sizeof(NxProfile))==0);
    nx_panel_begin(&panel,&active,NULL,false);
    nx_panel_input(&panel,NX_UP,&active); /* Last option wraps. */
    assert(nx_panel_input(&panel,NX_A,&active)==NX_PANEL_STAY);
    assert(nx_panel_input(&panel,NX_A | NX_B,&active)==NX_PANEL_STAY);
    assert(nx_panel_input(&panel,NX_B,&active)==NX_PANEL_STAY);
    assert(nx_panel_input(&panel,NX_A,&active)==NX_PANEL_STAY);
    assert(nx_panel_input(&panel,NX_A,&active)==NX_PANEL_CLOSE);
}

static void test_right_joycon_rails(void) {
    NxProfile active = nx_safe_profile;
    NxControlPanel panel;
    NxMapper mapper = {0};
    combat=false;
    poll(&mapper,&active,0,false);
    poll(&mapper,&active,NX_RIGHT_SL|NX_RIGHT_SR|NX_A|NX_R3,false);
    assert(held(13) && held('M') && !held(1));

    nx_panel_begin(&panel,&active,NULL,false);
    nx_panel_input(&panel,NX_A,&active);
    nx_panel_input(&panel,NX_R,&active);
    tap(&panel,&active,1,10); /* SR derecho. */
    choose_id(&panel,&active,nx_action_for_vk(1)->id);
    assert(panel.draft.bindings[0][NX_SIMPLE][15]==1);
    nx_panel_input(&panel,NX_X,&active);
    nx_panel_input(&panel,NX_X,&active);
    nx_panel_input(&panel,NX_R,&active);
    tap(&panel,&active,1,7); /* ZL + SL derecho. */
    choose_id(&panel,&active,nx_action_for_vk('P')->id);
    assert(panel.draft.chords[0][0][14]=='P');
    active=panel.draft;

    poll(&mapper,&active,0,false);
    poll(&mapper,&active,NX_RIGHT_SR|NX_B,false);
    assert(held(1) && held('X'));
    poll(&mapper,&active,0,false);
    poll(&mapper,&active,NX_ZL|NX_RIGHT_SL,false);
    assert(held('P') && !held('A'));
    poll(&mapper,&active,NX_RIGHT_SL,false);
    assert(!held('P'));

    combat=true;
    active.bindings[1][NX_SIMPLE][15]='P';
    active.bindings[1][NX_MINUS][14]='E';
    poll(&mapper,&active,0,false);
    poll(&mapper,&active,NX_RIGHT_SR|NX_R,false);
    assert(held('P') && held(13) && !held(1));
    poll(&mapper,&active,0,true);
    poll(&mapper,&active,NX_RIGHT_SL,true);
    assert(held('E'));
    poll(&mapper,&active,NX_RIGHT_SL,false);
    assert(!held('E'));
}

static void test_trigger_chords(void) {
    NxProfile profile=nx_safe_profile;
    NxMapper mapper={0};
    combat=false;
    profile.chords[0][0][2]=123;
    profile.chords[0][1][2]=121;
    profile.bindings[0][1][2]=112;
    poll(&mapper,&profile,0,false);
    poll(&mapper,&profile,NX_ZL,false);
    assert(!held('A'));
    poll(&mapper,&profile,NX_ZL|NX_X,false);
    assert(held(123) && !held(32) && !held('A'));
    poll(&mapper,&profile,NX_X,false);
    assert(!held(123) && !held(32) && !held('A'));
    poll(&mapper,&profile,0,false);
    poll(&mapper,&profile,NX_ZL,false);
    poll(&mapper,&profile,0,false);
    assert(held('A')); /* Standalone tap. */
    poll(&mapper,&profile,NX_X,false);
    assert(held(32) && !held('A'));
    poll(&mapper,&profile,NX_X|NX_ZL,false);
    assert(held(32) && !held(123));
    poll(&mapper,&profile,0,false);
    assert(!held('A')); /* Button-first chord attempt cannot synthesize input. */
    poll(&mapper,&profile,NX_X|NX_ZL|NX_ZR,false);
    assert(held(123) && !held(121));
    poll(&mapper,&profile,0,false);
    assert(!held('A') && !held('B'));
    poll(&mapper,&profile,NX_X|NX_ZL|NX_ZR,true);
    assert(held(112) && !held(123) && !held(121));
}

static void test_shortcuts(void) {
    NxProfile active = nx_safe_profile;
    NxControlPanel panel;
    for (unsigned field=0;field<2;++field) {
        unsigned count=field?8:12;
        for (unsigned i=0;i<count;++i) {
            nx_panel_begin(&panel,&active,NULL,false);
            assert(tap(&panel,&active,1,field?9:5)==NX_PANEL_STAY);
            assert(tap(&panel,&active,(i%2)*32,3+i/2)==NX_PANEL_STAY);
            assert(panel.shortcut_vk==0);
            assert(tap(&panel,&active,(i%2)*32,3+i/2)==NX_PANEL_STAY);
            assert(tap(&panel,&active,1,11)==NX_PANEL_SEND);
            assert(panel.shortcut_vk==(field?'1':112)+i);
        }
    }
    nx_panel_begin(&panel,&active,NULL,true);
    assert(tap(&panel,&active,1,9)==NX_PANEL_STAY); /* Field is blocked in battle mode. */
    assert(!panel.confirm_shortcut && panel.status);
    nx_panel_begin(&panel,&active,NULL,false);
    panel.draft.bindings[0][0][0]='A';
    assert(tap(&panel,&active,1,5)==NX_PANEL_STAY); /* Draft blocks sending. */
    assert(!panel.confirm_shortcut && panel.status);
    nx_panel_begin(&panel,&active,NULL,false);
    assert(tap(&panel,&active,1,5)==NX_PANEL_STAY);
    nx_panel_input(&panel,NX_DOWN,&active);
    nx_panel_input(&panel,NX_RIGHT,&active);
    nx_panel_input(&panel,NX_A,&active);
    assert(tap(&panel,&active,32,11)==NX_PANEL_STAY); /* Cancel confirmation. */
    assert(!panel.confirm_shortcut && !panel.shortcut_vk);
    nx_panel_input(&panel,NX_A,&active);
    assert(nx_panel_input(&panel,NX_A,&active)==NX_PANEL_SEND);
    assert(panel.shortcut_vk==115);
    NxShortcutPulse pulse={.queued=panel.shortcut_vk};
    assert(nx_shortcut_poll(&pulse,false)==0);
    assert(nx_shortcut_poll(&pulse,true)==115);
    assert(nx_shortcut_poll(&pulse,false)==115);
    assert(nx_shortcut_poll(&pulse,false)==115);
    assert(nx_shortcut_poll(&pulse,true)==0);

    bool armed=false;
    assert(!nx_touch_begin(&armed,true,1));
    assert(!nx_touch_begin(&armed,true,0));
    assert(nx_touch_begin(&armed,true,1));
    assert(!nx_touch_begin(&armed,true,1));
    assert(!nx_touch_begin(&armed,true,0));
    assert(!nx_touch_begin(&armed,true,2));
    assert(!nx_touch_begin(&armed,true,1));
    assert(!nx_touch_begin(&armed,false,0));
    assert(!nx_touch_begin(&armed,true,1));
}

static void test_battle_context(void) {
    NxBattleContext state = {0};
    nx_battle_context(&state,191,true,false);
    assert(!state.active); /* Scene arrived, original controller is initializing. */
    nx_battle_context(&state,191,true,true);
    assert(state.active); /* Wild or trainer with the same original controller. */
    nx_battle_context(&state,191,true,false);
    assert(state.active); /* Switching and outcome dialogue stay in the battle room. */
    nx_battle_context(&state,51,false,false);
    assert(!state.active); /* Win, loss and flee all commit a different room. */
    nx_battle_context(&state,191,true,false);
    assert(!state.active); /* A new battle in the same room has to initialize again. */
    nx_battle_context(&state,47,true,true);
    assert(state.active);
    nx_battle_context(&state,715,false,true);
    assert(!state.active); /* Safari never activates the conventional controller. */
    nx_battle_context(&state,542,true,true);
    assert(state.active);
    nx_battle_context(&state,540,false,false);
    assert(!state.active);

    NxProfile profile = nx_safe_profile;
    NxMapper mapper = {0};
    combat = false;
    profile.bindings[0][1][2] = 112;
    poll(&mapper,&profile,0,false);
    poll(&mapper,&profile,0,true);
    poll(&mapper,&profile,0,false);
    assert(!combat); /* Bare Minus is ignored with the default automatic policy. */
    poll(&mapper,&profile,0,true);
    poll(&mapper,&profile,NX_X,true);
    assert(held(112)); /* Minus chords keep their profile semantics. */
    combat = true;
    mapper = (NxMapper){0}; /* Context barrier used before producing new keys. */
    poll(&mapper,&profile,NX_X,true);
    assert(!held(112) && !held('C'));
    poll(&mapper,&profile,NX_X,false);
    assert(!held('C'));
    poll(&mapper,&profile,0,false);
    poll(&mapper,&profile,NX_X,false);
    assert(held('C'));
}

static void safari_poll(NxMapper *mapper, const NxProfile *profile, uint32_t chord_buttons,
                        uint32_t pad, bool minus) {
    memset(keys,0,sizeof(keys));
    nx_safari_mapper_poll(mapper,profile,chord_buttons,pad,minus,true,keys);
}

static void test_safari(void) {
    NxProfile saved = nx_safe_profile;
    saved.manual_battle = 1;
    NxProfile safari = nx_safari_profile(&saved);
    uint32_t chords = nx_safari_chord_buttons(&saved);
    assert(chords == (NX_L|NX_R|NX_L3|NX_R3));
    NxMapper mapper = {0};
    safari_poll(&mapper,&safari,chords,NX_L|NX_R,false);
    assert(!held('E')); /* Held on room entry. */
    safari_poll(&mapper,&safari,chords,0,false);
    safari_poll(&mapper,&safari,chords,NX_ZR,false);
    assert(held('B'));
    safari_poll(&mapper,&safari,chords,NX_A,false);
    assert(held('V') && !held('B'));
    safari_poll(&mapper,&safari,chords,NX_L3|NX_X,true);
    assert(!held('E') && !held('P')); /* RC1 fallbacks are gone. */
    safari_poll(&mapper,&safari,chords,0,false);
    nx_safari_step_finished(&mapper);
    assert(saved.manual_battle && !safari.manual_battle);

    const uint32_t shoulders[] = {NX_L, NX_R};
    const uint32_t sticks[] = {NX_L3, NX_R3};
    for (unsigned first = 0; first < 2; ++first) {
        for (unsigned poll_index = 0; poll_index < 8; ++poll_index) {
            safari_poll(&mapper,&safari,chords,shoulders[first],false);
            assert(!held(13) && !held(16) && !held('E'));
        }
        safari_poll(&mapper,&safari,chords,NX_L|NX_R,false);
        assert(held('E') && !held(13) && !held(16));
        safari_poll(&mapper,&safari,chords,NX_L|NX_R,false);
        assert(held('E')); /* Same down edge survives a render-only poll. */
        nx_safari_step_finished(&mapper);
        safari_poll(&mapper,&safari,chords,NX_L|NX_R,false);
        assert(!held('E')); /* Publish the up edge after one logic step. */
        safari_poll(&mapper,&safari,chords,shoulders[first],false);
        safari_poll(&mapper,&safari,chords,NX_L|NX_R,false);
        assert(!held('E') && !held(13) && !held(16));
        safari_poll(&mapper,&safari,chords,0,false);
        assert(!held(13) && !held(16));

        safari_poll(&mapper,&safari,chords,sticks[first],false);
        assert(!held('S') && !held('D') && !held('P'));
        safari_poll(&mapper,&safari,chords,NX_L3|NX_R3,false);
        assert(held('P') && !held('S') && !held('D'));
        nx_safari_step_finished(&mapper);
        safari_poll(&mapper,&safari,chords,NX_L3|NX_R3,false);
        assert(held('P'));
        safari_poll(&mapper,&safari,chords,sticks[first],false);
        assert(!held('P') && !held('S') && !held('D'));
        safari_poll(&mapper,&safari,chords,0,false);
        assert(!held('S') && !held('D'));
    }

    safari_poll(&mapper,&safari,chords,NX_L|NX_R|NX_L3|NX_R3,false);
    assert(held('E') && held('P')); /* Same HID sample and both pairs. */
    nx_safari_step_finished(&mapper);
    safari_poll(&mapper,&safari,chords,0,false);
    assert(!held('E') && !held('P'));
    safari_poll(&mapper,&safari,chords,NX_R,false);
    assert(!held(13));
    safari_poll(&mapper,&safari,chords,0,false);
    assert(held(13)); /* Solo R still confirms a dialogue, on release. */
    nx_safari_step_finished(&mapper);
    safari_poll(&mapper,&safari,chords,0,false);
    assert(!held(13));

    safari_poll(&mapper,&safari,chords,NX_R,false);
    safari_poll(&mapper,&safari,chords,0,false);
    assert(held(13));
    safari_poll(&mapper,&safari,chords,NX_L|NX_R,false);
    assert(held('E') && !held(13)); /* Queued R cannot carry into Huir. */
    nx_safari_step_finished(&mapper);
    safari_poll(&mapper,&safari,chords,0,false);

    saved.bindings[1][NX_SIMPLE][12] = 'E';
    saved.chords[1][1][2] = 'P';
    safari = nx_safari_profile(&saved);
    chords = nx_safari_chord_buttons(&saved);
    assert(!chords);
    safari_poll(&mapper,&safari,chords,NX_L3,false);
    assert(held('E'));
    safari_poll(&mapper,&safari,chords,0,false);
    safari_poll(&mapper,&safari,chords,NX_ZR|NX_X,false);
    assert(held('P') && !held('B'));

    saved = nx_safe_profile;
    saved.bindings[1][NX_MINUS][8] = 'M';
    saved.chords[1][0][13] = 'Z';
    safari = nx_safari_profile(&saved);
    chords = nx_safari_chord_buttons(&saved);
    assert(!chords); /* Custom assignments on constituent buttons win. */
    safari_poll(&mapper,&safari,chords,NX_L,true);
    assert(held('M'));
    safari_poll(&mapper,&safari,chords,0,false);
    safari_poll(&mapper,&safari,chords,NX_ZL|NX_R3,false);
    assert(held('Z') && !held('P'));

    saved.bindings[1][NX_SIMPLE][0] = 'C';
    saved.bindings[1][NX_MINUS][0] = 'M';
    safari = nx_safari_profile(&saved);
    assert(safari.bindings[1][NX_SIMPLE][0]=='C' && safari.bindings[1][NX_MINUS][0]=='M');

    NxControls touch = {0};
    const uint8_t expected[] = {'B','V','P','Z','X',0,'E'};
    for (unsigned i = 0; i < sizeof(expected); ++i) {
        unsigned x=i==NX_FLEE_SLOT-1?NX_FLEE_LEFT:NX_PARTY_LEFT;
        unsigned y=i==NX_FLEE_SLOT-1?NX_FLEE_TOP:NX_PARTY_TOP+i*NX_PARTY_PITCH;
        nx_controls_touch_layout(&touch,true,0,0,0,0,true,false);
        unsigned slot=nx_controls_touch_layout(&touch,true,1,1,x,y,true,false);
        assert(nx_safari_touch_vk(slot)==expected[i]);
        assert(nx_controls_touch_layout(&touch,true,1,1,NX_PARTY_LEFT,NX_PARTY_TOP,true,false)==slot);
    }
    nx_controls_touch(&touch,false,0,0,0,0);
    assert(!nx_controls_touch(&touch,true,1,1,NX_PARTY_LEFT,NX_PARTY_TOP));
    combat=false;
    mapper=(NxMapper){0};
    poll(&mapper,&saved,NX_L3,false);
    assert(!held('E') && !held('O'));
    poll(&mapper,&saved,0,false);
    poll(&mapper,&saved,NX_L3,false);
    assert(held('O') && !held('E')); /* Fresh press after return to Normal. */
}

static void test_circular_navigation(void) {
    assert(nx_panel_list_step(0,0,false)==0 && nx_panel_list_step(0,0,true)==0);
    assert(nx_panel_list_step(0,1,false)==0 && nx_panel_list_step(0,1,true)==0);
    NxProfile active = nx_safe_profile;
    NxControlPanel panel;
    NxPanelView view;
    nx_panel_begin(&panel,&active,NULL,true);
    nx_panel_input(&panel,NX_UP,&active);
    nx_panel_text(&panel,&active,&view);
    assert(view.row==12); /* Close needs confirmation, even from the wrapped row. */
    nx_panel_input(&panel,NX_DOWN,&active);
    assert(panel.selection==0);
    tap(&panel,&active,1,8); /* Restore. */
    nx_panel_input(&panel,NX_DOWN,&active);
    assert(panel.selection==7); /* Skip disabled field shortcuts in combat. */
    assert(tap(&panel,&active,1,9)==NX_PANEL_STAY && panel.selection==7);
    nx_panel_input(&panel,NX_A,&active);
    assert(panel.draft.manual_battle && !active.manual_battle);
    assert(tap(&panel,&active,1,5)==NX_PANEL_STAY && panel.selection==7);
    assert(tap(&panel,&active,1,7)==NX_PANEL_BACK && active.manual_battle);

    nx_panel_begin(&panel,&active,NULL,false);
    nx_panel_input(&panel,NX_A,&active);
    nx_panel_input(&panel,NX_UP,&active);
    assert(panel.selection==NX_SOURCES-1);
    nx_panel_input(&panel,NX_DOWN,&active);
    assert(panel.selection==0);
    nx_panel_input(&panel,NX_R,&active);
    assert(panel.selection==8);
    nx_panel_input(&panel,NX_L,&active);
    assert(panel.selection==0);
    nx_panel_input(&panel,NX_A,&active);
    panel.selection=0;
    nx_panel_input(&panel,NX_UP,&active);
    assert(panel.selection==nx_action_count()-1);
    nx_panel_input(&panel,NX_DOWN,&active);
    assert(panel.selection==0);
    nx_panel_input(&panel,NX_X,&active);
    nx_panel_input(&panel,NX_UP,&active);
    assert(panel.search_cell==8);
    nx_panel_input(&panel,NX_LEFT,&active);
    assert(panel.search_cell==9);

    nx_panel_begin(&panel,&active,NULL,false);
    tap(&panel,&active,1,5);
    nx_panel_input(&panel,NX_LEFT,&active);
    nx_panel_input(&panel,NX_UP,&active);
    assert(panel.selection==11); /* F1 wraps to F12 using the two axes. */
    nx_panel_input(&panel,NX_A,&active);
    assert(nx_panel_input(&panel,NX_A,&active)==NX_PANEL_SEND && panel.shortcut_vk==123);
}

int main(void) {
    test_pad();
    test_touch();
    test_trigger_chords();
    test_right_joycon_rails();
    test_right_stick();
    test_mega_touch();
    test_panel();
    test_shortcuts();
    test_battle_context();
    test_circular_navigation();
    test_safari();
    puts("Horizon controls: Mega, right stick, auto/manual context, circular panels, profiles, Huir, Safari and shortcuts PASS");
    return 0;
}
