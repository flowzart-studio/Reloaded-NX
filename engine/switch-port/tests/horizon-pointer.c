/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "../horizon-pointer.h"

int main(void) {
    const NxPointerRect bounds = {160, 0, 1120, 720};
    NxPointer p = {.x = 640, .y = 360};
    NxPointerFinger contacts[] = {{42, 700, 300}, {44, 1190, 80}};
    assert(nx_pointer_touch(&p, bounds, contacts, 1).flags == NX_POINTER_VALID);
    nx_pointer_touch(&p, bounds, contacts, 0);
    assert(nx_pointer_touch(&p, bounds, contacts, 1).flags & NX_POINTER_DOWN);
    contacts[0].x = 1250;
    NxPointerSample dragged = nx_pointer_touch(&p, bounds, contacts, 2);
    assert((dragged.flags & NX_POINTER_DOWN) && dragged.x == 1119);
    assert(!(nx_pointer_touch(&p, bounds, contacts + 1, 1).flags & NX_POINTER_DOWN));
    assert(!(nx_pointer_touch(&p, bounds, contacts, 1).flags & NX_POINTER_DOWN));
    nx_pointer_touch(&p, bounds, contacts, 0);
    assert(!(nx_pointer_touch(&p, bounds, contacts, 1).flags & NX_POINTER_DOWN));
    contacts[0].x = 500;
    assert(!(nx_pointer_touch(&p, bounds, contacts, 1).flags & NX_POINTER_DOWN));
    nx_pointer_touch(&p, bounds, contacts, 0);
    assert(!(nx_pointer_touch(&p, bounds, contacts, 2).flags & NX_POINTER_DOWN));

    p = (NxPointer){.x = 640, .y = 360};
    assert(!(nx_pointer_stick(&p, bounds, 0, 0, 0.016f, true).flags & NX_POINTER_DOWN));
    NxPointerSample idle = nx_pointer_stick(&p, bounds, 6499, -6499, 0.016f, false);
    assert(idle.x == 640 && idle.y == 360);
    NxPointerSample press = nx_pointer_stick(&p, bounds, 32767, 32767, 0.016f, true);
    assert((press.flags & NX_POINTER_DOWN) && press.x > idle.x && press.y < idle.y);
    NxPointerSample held = nx_pointer_stick(&p, bounds, 32767, 0, 2.0f, true);
    assert((held.flags & NX_POINTER_DOWN) && held.x > press.x && held.x - press.x < 42);
    assert(!(nx_pointer_stick(&p, bounds, 0, 0, 0.016f, false).flags & NX_POINTER_DOWN));
    /* Enter the map with its opening R3 already held, then leave on a held click. */
    const NxPointerRect map = {0, 0, 1280, 720};
    p = (NxPointer){.x=640,.y=360};
    assert(!(nx_pointer_stick(&p,map,0,0,0.0f,true).flags & NX_POINTER_DOWN));
    nx_pointer_stick(&p,map,0,0,0.0f,false);
    assert(nx_pointer_stick(&p,map,32767,0,0.016f,true).flags & NX_POINTER_DOWN);
    p = (NxPointer){.x=p.x,.y=p.y};
    assert(!(nx_pointer_stick(&p,map,0,0,0.0f,true).flags & NX_POINTER_DOWN));
    contacts[0] = (NxPointerFinger){42,1250,360};
    assert(!(nx_pointer_touch(&p,map,contacts,1).flags & NX_POINTER_DOWN));
    nx_pointer_touch(&p,map,contacts,0);
    NxPointerSample arrow = nx_pointer_touch(&p,map,contacts,1);
    assert((arrow.flags & NX_POINTER_DOWN) && arrow.x==1250 && arrow.y==360);
    NxPointerSample hover = nx_pointer_touch(&p,map,contacts,0);
    assert(hover.flags==NX_POINTER_VALID && hover.x==arrow.x && hover.y==arrow.y);

    /* Handheld polls empty touch before stick. Neither a hover nor a held click
       carried across the end of a touch gesture may create a new press. */
    p = (NxPointer){.x=640,.y=360};
    nx_pointer_touch(&p,bounds,contacts,0);
    assert(!p.stick_visible);
    assert(nx_pointer_stick(&p,bounds,8000,0,0.05f,false).flags==NX_POINTER_VALID);
    assert(p.stick_visible && p.x>640);
    assert(nx_pointer_velocity(6499)==0 && nx_pointer_velocity(16000)>nx_pointer_velocity(8000));
    assert(nx_pointer_velocity(32767)>nx_pointer_velocity(16000));
    for (unsigned i=0;i<50;i++) {
        nx_pointer_touch(&p,bounds,contacts,0);
        assert(nx_pointer_stick(&p,bounds,32767,32767,0.05f,false).flags==NX_POINTER_VALID);
    }
    assert(p.x==1119 && p.y==0);
    contacts[0]=(NxPointerFinger){42,700,300};
    assert(nx_pointer_touch(&p,bounds,contacts,1).flags & NX_POINTER_DOWN);
    assert(!p.stick_visible);
    assert(nx_pointer_touch(&p,bounds,contacts,0).flags==NX_POINTER_VALID);
    assert(nx_pointer_stick(&p,bounds,0,0,0.016f,true).flags==NX_POINTER_VALID);
    assert(p.x==700 && p.y==300);
    nx_pointer_stick(&p,bounds,0,0,0.016f,false);
    assert(nx_pointer_stick(&p,bounds,0,0,0.016f,true).flags & NX_POINTER_DOWN);
    assert(nx_pointer_touch(&p,bounds,contacts,1).flags==NX_POINTER_VALID);
    assert(nx_pointer_touch(&p,bounds,contacts,1).flags==NX_POINTER_VALID);
    nx_pointer_touch(&p,bounds,contacts,0);
    assert(nx_pointer_touch(&p,bounds,contacts,1).flags & NX_POINTER_DOWN);
    puts("Horizon map/PC pointer: touch, stick, hover, click, handoff and bounds PASS");
}
