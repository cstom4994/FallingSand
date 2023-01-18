// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef _METADOT_GLOBAL_HPP_
#define _METADOT_GLOBAL_HPP_

#include <map>
#include <unordered_map>

#include "core/core.hpp"
#include "engine/audio.hpp"
#include "engine/code_reflection.hpp"
#include "engine/engine.h"
#include "engine/engine_platform.h"
#include "engine/filesystem.h"
#include "engine/imgui_impl.hpp"
#include "game/game_shaders.hpp"
#include "game/game_ui.hpp"

class Game;
class Scripts;
class UIData;

struct Global {
    Game *game = nullptr;
    Scripts *scripts = nullptr;
    UIData *uidata = nullptr;
    GameData GameData_;
    Audio audioEngine;
    I18N I18N;
};

extern Global global;

#endif