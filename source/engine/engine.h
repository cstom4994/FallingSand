// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef ME_ENGINE_H
#define ME_ENGINE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <string>

#include "engine/core/core.hpp"
#include "engine/core/platform.h"
#include "engine/core/sdl_wrapper.h"
#include "engine/renderer/gpu.hpp"
#include "engine/renderer/renderer_gpu.h"
#include "engine/utils/utility.hpp"
#include "engine/utils/utils.hpp"

// Engine functions called from main
int InitEngine(void (*InitCppReflection)());
void EngineUpdate();
void EngineUpdateEnd();
void EndEngine(int errorOcurred);
void DrawSplash();

typedef struct EngineData {
    C_Window *window;
    C_GLContext *glContext;

    std::string gamepath;

    // Maximum memory that can be used
    u64 max_mem = 4294967296;  // 4096mb

    int windowWidth;
    int windowHeight;

    i32 render_scale = 4;

    unsigned maxFPS;

    R_Target *realTarget;
    R_Target *target;

    struct {
        i32 feelsLikeFps;
        i64 lastTime;
        i64 lastCheckTime;
        i64 lastTickTime;
        i64 lastLoadingTick;
        i64 now;
        i64 startTime;
        i64 deltaTime;

        i32 tickCount;

        f32 mspt;
        i32 tpsTrace[TraceTimeNum];
        f32 tps;
        u32 maxTps;

        u16 frameTimesTrace[TraceTimeNum];
        u32 frameCount;
        f32 framesPerSecond;
    } time;

} EngineData;

extern EngineData g_engine_data;

ME_INLINE EngineData *ENGINE() { return &g_engine_data; }

#define ENGINE() ENGINE()

void ExitGame();
void GameExited();

int InitCore();
bool InitTime();
bool InitScreen(int windowWidth, int windowHeight, int scale, int maxFPS);

void UpdateTime();
void WaitUntilNextFrame();

void InitFPS();
void ProcessTickTime();
f32 GetFPS();

#endif