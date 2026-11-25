/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <string.h>
#include "../horizon-casino.h"
int main(void) {
    const uint8_t slots[5] = {32,'Z','X','C','X'};
    NxProfile original = nx_safe_profile;
    NxProfile effective = nx_casino_profile(&original, slots);
    assert(memcmp(&original,&nx_safe_profile,sizeof(original))==0);
    assert(effective.bindings[0][NX_MINUS][0]=='C');
    assert(memcmp(effective.bindings[0][0],original.bindings[0][0],NX_SOURCES)==0);
    assert(memcmp(effective.bindings[1],original.bindings[1],sizeof original.bindings[1])==0);
    const uint8_t poker[5]={'Z','X',32,0,0};
    effective=nx_casino_profile(&original,poker);
    assert(memcmp(&effective,&original,sizeof original)==0);
    original.bindings[0][NX_MINUS][0]='Q';
    effective=nx_casino_profile(&original,slots);
    assert(effective.bindings[0][NX_MINUS][0]=='Q');
    assert(effective.bindings[0][NX_MINUS][1]=='C');
    const uint8_t remapped[5]={'J','K','L','N','H'};
    effective=nx_casino_profile(&original,remapped);
    for(unsigned i=0;i<5;i++)assert(effective.bindings[0][NX_MINUS][i+1]==remapped[i]);
    for(unsigned i=0;i<16;i++)original.bindings[0][NX_MINUS][i]='Q';
    effective=nx_casino_profile(&original,slots);
    assert(memcmp(&effective,&original,sizeof original)==0); /* no destructive fallback */
    original=nx_safe_profile;
    effective=nx_casino_profile(&original,slots);
    NxMapper mapper={0}; bool combat=false; uint64_t keys[4]={0};
    nx_mapper_poll(&mapper,&effective,&combat,0,false,true,keys);
    nx_mapper_poll(&mapper,&effective,&combat,NX_A,true,true,keys);
    assert(keys['C'/64] & (UINT64_C(1)<<('C'%64)));
    memset(keys,0,sizeof keys);
    nx_mapper_poll(&mapper,&effective,&combat,0,false,true,keys);
    assert(!(keys['C'/64] & (UINT64_C(1)<<('C'%64))));
    mapper=(NxMapper){0};
    nx_mapper_poll(&mapper,&original,&combat,0,false,true,keys);
    nx_mapper_poll(&mapper,&original,&combat,NX_A,false,true,keys);
    assert(keys[0] & (UINT64_C(1)<<13)); /* normal A restored */
    return 0;
}
