// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#include "rng.h"

#include "core/cpp/utils.hpp"
#include "core/mathlib.hpp"
#include "core/utils/utility.hpp"

RNG* RNG_Create() {
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, ME_gettime(), 1);
    unsigned int seed = pcg32_random_r(&rng);

    RNG* sRNG = new RNG;
    sRNG->rng = rng;
    sRNG->root_seed = seed;

    return sRNG;
}

void RNG_Delete(RNG* rng) {
    if (NULL != rng) delete rng;
}

u32 RNG_Next(RNG* rng) { return pcg32_random_r(&rng->rng); }
