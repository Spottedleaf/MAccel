#ifndef RL_DEFS_H
#define RL_DEFS_H RL_DEFS_H

#define RL_TICKRATE 128
#define RL_X86_CACHE_LINE_SIZE 64

#include <immintrin.h>

#ifdef __SSE2__

/* rl_pause_intrin maps to _mm_pause if available */
#define rl_pause_intrin() _mm_pause()

#else

#define rl_pause_intrin()

#endif /* __SSE2__ */


/* errno defs */

#define RL_EOVERF 1
#define RL_ENOMEM 2
#define RL_ECORUP 3
#define RL_EINVAL 4
#define RL_ERANGE 5
#define RL_EOTHER 6 /* Signals that errno is set */

/* end errno defs */

#endif /* RL_DEFS_H */