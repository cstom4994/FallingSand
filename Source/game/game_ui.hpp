// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef _METADOT_GAMEUI_HPP_
#define _METADOT_GAMEUI_HPP_

#include <vector>

#include "engine/imgui_impl.hpp"
#include "engine/renderer/renderer_gpu.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "game/World.hpp"
#include "game/game_datastruct.hpp"

class Game;

struct I18N {
    void Init();
    void Load(std::string lang);
    std::string Get(std::string text);
};

namespace GameUI {

void GameUI_Draw(Game *game);

void DrawDebugUI(Game *game);

class DebugDrawUI {
public:
    static bool visible;
    static int selIndex;
    static std::vector<R_Image *> images;
    static std::vector<R_Image *> tools_images;

    static Material *selectedMaterial;
    static U8 brushSize;

    static void Setup();

    static void Draw(Game *game);
};

class MainMenuUI {
public:
    static bool visible;
    static int state;
    static bool setup;
    static R_Image *title;
    static bool connectButtonEnabled;
    static ImVec2 pos;
    static std::vector<std::tuple<std::string, WorldMeta>> worlds;
    static long long lastRefresh;

    static void RefreshWorlds(Game *game);
    static void Setup();
    static void Draw(Game *game);
    static void DrawMainMenu(Game *game);
    static void DrawWorldLists(Game *game);
};

class InGameUI {
public:
    static bool visible;
    static int state;
    static bool setup;

    static void Setup();
    static void Draw(Game *game);
    static void DrawInGame(Game *game);
};

class CreateWorldUI {
public:
    static bool setup;
    static char worldNameBuf[32];

    static bool createWorldButtonEnabled;
    static std::string worldFolderLabel;
    static int selIndex;

    static void Setup();
    static void Reset(Game *game);
    static void Draw(Game *game);
    static void inputChanged(std::string text, Game *game);
};

class OptionsUI {
#if defined(METADOT_BUILD_AUDIO)
    static std::map<std::string, FMOD::Studio::Bus *> busMap;
#endif

public:
    static int item_current_idx;
    static bool vsync;
    static bool minimizeOnFocus;

    static void Draw(Game *game);
    static void DrawGeneral(Game *game);
    static void DrawVideo(Game *game);
    static void DrawAudio(Game *game);
    static void DrawInput(Game *game);
};

}  // namespace GameUI

#endif