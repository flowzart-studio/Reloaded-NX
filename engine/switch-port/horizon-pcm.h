/* SPDX-License-Identifier: MIT. Format conversion only, shared with host regression. */
#pragma once
#include <stdint.h>
#include <math.h>
static inline int16_t nx_pcm_i16(float x){if(!isfinite(x))return 0;if(x>1)x=1;if(x< -1)x=-1;return (int16_t)(x*32767.0f);}
