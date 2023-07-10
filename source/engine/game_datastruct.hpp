// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef ME_GAMEDATASTRUCT_HPP
#define ME_GAMEDATASTRUCT_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "engine/core/core.hpp"
#include "engine/core/mathlib.hpp"
#include "engine/core/sdl_wrapper.h"
#include "engine/ecs/ecs.hpp"
#include "engine/meta/static_relfection.hpp"
#include "engine/physics/box2d.h"
#include "engine/renderer/renderer_gpu.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/utils/type.hpp"
#include "game_basic.hpp"

struct Chunk;
struct Populator;
struct World;
struct RigidBody;
struct b2Body;
struct R_Target;
struct CellData;
struct Biome;
struct Material;
struct Player;
struct Game;
struct ImGuiContext;

#define RegisterFunctions(name, func)    \
    Meta::AnyFunction any_##func{&func}; \
    GAME()->HostData.Functions.insert(std::make_pair(#name, any_##func))

#define GetFunctions(name) GAME()->HostData.Functions[name]

class WorldEntity {
public:
    std::string name;

    f32 x = 0;
    f32 y = 0;
    f32 vx = 0;
    f32 vy = 0;
    int hw = 14;
    int hh = 26;
    bool ground = false;
    RigidBody *rb = nullptr;
    bool is_player = false;

    WorldEntity(const WorldEntity &) = default;

    WorldEntity(bool isplayer, f32 x, f32 y, f32 vx, f32 vy, int hw, int hh, RigidBody *rb, std::string n = "unknown")
        : is_player(isplayer), x(x), y(y), vx(vx), vy(vy), hw(hw), hh(hh), rb(rb), name(n) {}
    ~WorldEntity() {
        // if (static_cast<bool>(rb)) delete rb;
    }
};

template <>
struct ME::meta::static_refl::TypeInfo<WorldEntity> : TypeInfoBase<WorldEntity> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            Field{TSTR("x"), &Type::x},
            Field{TSTR("y"), &Type::y},
            Field{TSTR("vx"), &Type::vx},
            Field{TSTR("vy"), &Type::vy},
            Field{TSTR("hw"), &Type::hw},
            Field{TSTR("hh"), &Type::hh},
            Field{TSTR("ground"), &Type::ground},

            // Field{TSTR("body"), &Type::body},
            Field{TSTR("rb"), &Type::rb},
            Field{TSTR("is_player"), &Type::is_player},
    };
};

ME_GUI_DEFINE_BEGIN(template <>, WorldEntity)
ME::meta::static_refl::TypeInfo<WorldEntity>::ForEachVarOf(var, [&](const auto &field, auto &&value) { ImGui::Auto(value, std::string(field.name)); });
ME_GUI_DEFINE_END

void ReleaseGameData();

#pragma region Material

enum PhysicsType {
    AIR = 0,
    SOLID = 1,
    SAND = 2,
    SOUP = 3,
    GAS = 4,
    PASSABLE = 5,
    OBJECT = 5,
};

#define INTERACT_NONE 0
#define INTERACT_TRANSFORM_MATERIAL 1  // id, radius
#define INTERACT_SPAWN_MATERIAL 2      // id, radius
#define EXPLODE 3                      // radius

#define REACT_TEMPERATURE_BELOW 4  // temperature, id
#define REACT_TEMPERATURE_ABOVE 5  // temperature, id

struct MaterialInteraction {
    int type = INTERACT_NONE;
    int data1 = 0;
    int data2 = 0;
    int ofsX = 0;
    int ofsY = 0;
};

template <>
struct ME::meta::static_refl::TypeInfo<MaterialInteraction> : TypeInfoBase<MaterialInteraction> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            Field{TSTR("type"), &MaterialInteraction::type}, Field{TSTR("data1"), &MaterialInteraction::data1}, Field{TSTR("data2"), &MaterialInteraction::data2},
            Field{TSTR("ofsX"), &MaterialInteraction::ofsX}, Field{TSTR("ofsY"), &MaterialInteraction::ofsY},
    };
};

struct Material {
    std::string name;
    std::string index_name;

    int id = 0;
    int physicsType = 0;
    u8 alpha = 0;
    f32 density = 0;
    int iterations = 0;
    int emit = 0;
    u32 emitColor = 0;
    u32 color = 0;
    u32 addTemp = 0;
    f32 conductionSelf = 1.0;
    f32 conductionOther = 1.0;

    bool is_scriptable = false;

    bool interact = false;
    int *nInteractions = nullptr;

    std::vector<MaterialInteraction> *interactions = nullptr;

    bool react = false;
    int nReactions = 0;

    std::vector<MaterialInteraction> reactions;

    int slipperyness = 1;

    Material(int id, std::string name, std::string index_name, PhysicsType physicsType, int slipperyness, u8 alpha, f32 density, int iterations, int emit, u32 emitColor, u32 color);
    Material(int id, std::string name, std::string index_name, PhysicsType physicsType, int slipperyness, u8 alpha, f32 density, int iterations, int emit, u32 emitColor)
        : Material(id, name, index_name, physicsType, slipperyness, alpha, density, iterations, emit, emitColor, 0xffffffff) {}
    Material(int id, std::string name, std::string index_name, PhysicsType physicsType, int slipperyness, u8 alpha, f32 density, int iterations)
        : Material(id, name, index_name, physicsType, slipperyness, alpha, density, iterations, 0, 0) {}
    Material(int id, std::string name, std::string index_name, PhysicsType physicsType, int slipperyness, f32 density, int iterations)
        : Material(id, name, index_name, physicsType, slipperyness, 0xff, density, iterations) {}
    Material() : Material(0, "Air", "", PhysicsType::AIR, 4, 0, 0) {}
};

template <>
struct ME::meta::static_refl::TypeInfo<Material> : TypeInfoBase<Material> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            Field{TSTR("name"), &Material::name},
            Field{TSTR("index_name"), &Material::index_name},
            Field{TSTR("id"), &Material::id},
            Field{TSTR("is_scriptable"), &Material::is_scriptable},
            Field{TSTR("physicsType"), &Material::physicsType},
            Field{TSTR("alpha"), &Material::alpha},
            Field{TSTR("density"), &Material::density},
            Field{TSTR("iterations"), &Material::iterations},
            Field{TSTR("emit"), &Material::emit},
            Field{TSTR("emitColor"), &Material::emitColor},
            Field{TSTR("color"), &Material::color},
            Field{TSTR("addTemp"), &Material::addTemp},
            Field{TSTR("conductionSelf"), &Material::conductionSelf},
            Field{TSTR("conductionOther"), &Material::conductionOther},
            Field{TSTR("interact"), &Material::interact},
            Field{TSTR("nInteractions"), &Material::nInteractions},
            // Field{TSTR("interactions"), &Material::interactions},
            Field{TSTR("react"), &Material::react},
            Field{TSTR("nReactions"), &Material::nReactions},
            // Field{TSTR("reactions"), &Material::reactions},
            Field{TSTR("slipperyness"), &Material::slipperyness},
    };
};

ME_GUI_DEFINE_BEGIN(template <>, Material)
ME::meta::static_refl::TypeInfo<Material>::ForEachVarOf(var, [](auto field, auto &&value) {
    static_assert(std::is_lvalue_reference_v<decltype(value)>);
    ImGui::Auto(value, std::string(field.name));
});
ME_GUI_DEFINE_END

struct MaterialsList {
    std::unordered_map<int, Material> ScriptableMaterials;
    Material GENERIC_AIR;
    Material GENERIC_SOLID;
    Material GENERIC_SAND;
    Material GENERIC_LIQUID;
    Material GENERIC_GAS;
    Material GENERIC_PASSABLE;
    Material GENERIC_OBJECT;
    Material STONE;
    Material GRASS;
    Material DIRT;
    Material SMOOTH_STONE;
    Material COBBLE_STONE;
    Material SMOOTH_DIRT;
    Material COBBLE_DIRT;
    Material SOFT_DIRT;
    Material WATER;
    Material LAVA;
    Material CLOUD;
    Material GOLD_ORE;
    Material GOLD_MOLTEN;
    Material GOLD_SOLID;
    Material IRON_ORE;
    Material OBSIDIAN;
    Material STEAM;
    Material SOFT_DIRT_SAND;
    Material FIRE;
    Material FLAT_COBBLE_STONE;
    Material FLAT_COBBLE_DIRT;
};

void InitMaterials();
void RegisterMaterial(int s_id, std::string name, std::string index_name, int physicsType, int slipperyness, u8 alpha, f32 density, int iterations, int emit, u32 emitColor, u32 color);
void PushMaterials();

struct GameData {
    i32 ofsX = 0;
    i32 ofsY = 0;

    f32 plPosX = 0;
    f32 plPosY = 0;

    f32 camX = 0;
    f32 camY = 0;

    f32 desCamX = 0;
    f32 desCamY = 0;

    f32 freeCamX = 0;
    f32 freeCamY = 0;

    std::vector<Biome *> biome_container;
    std::vector<Material *> materials_container;
    i32 materials_count;
    Material **materials_array;

    MaterialsList materials_list;

    struct {
        std::unordered_map<std::string, ME::meta::any_function> Functions;
    } HostData;
};

extern GameData g_game_data;

ME_INLINE GameData *GAME() { return &g_game_data; }

class MaterialInstance {
public:
    Material *mat;
    u32 color;
    i32 temperature;
    u32 id = 0;
    bool moved = false;
    f32 fluidAmount = 2.0f;
    f32 fluidAmountDiff = 0.0f;
    u8 settleCount = 0;

    MaterialInstance(Material *mat, u32 color, i32 temperature);
    MaterialInstance(Material *mat, u32 color) : MaterialInstance(mat, color, 0){};
    MaterialInstance() : MaterialInstance(&GAME()->materials_list.GENERIC_AIR, 0x000000, 0){};
    inline bool operator==(const MaterialInstance &other) { return this->mat->id == other.mat->id; }
};

template <>
struct ME::meta::static_refl::TypeInfo<MaterialInstance> : TypeInfoBase<MaterialInstance> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {};
};

ME_GUI_DEFINE_BEGIN(template <>, MaterialInstance)
ImGui::Text("MaterialInstance:\n%d", var.id);
ImGui::Auto(var.mat, "Material");
ME_GUI_DEFINE_END

extern MaterialInstance Tiles_NOTHING;
extern MaterialInstance Tiles_TEST_SOLID;
extern MaterialInstance Tiles_TEST_SAND;
extern MaterialInstance Tiles_TEST_LIQUID;
extern MaterialInstance Tiles_TEST_GAS;
extern MaterialInstance Tiles_OBJECT;

MaterialInstance TilesCreateTestSand();
MaterialInstance TilesCreateTestTexturedSand(int x, int y);
MaterialInstance TilesCreateTestLiquid();
MaterialInstance TilesCreateStone(int x, int y);
MaterialInstance TilesCreateGrass();
MaterialInstance TilesCreateDirt();
MaterialInstance TilesCreateSmoothStone(int x, int y);
MaterialInstance TilesCreateCobbleStone(int x, int y);
MaterialInstance TilesCreateSmoothDirt(int x, int y);
MaterialInstance TilesCreateCobbleDirt(int x, int y);
MaterialInstance TilesCreateSoftDirt(int x, int y);
MaterialInstance TilesCreateWater();
MaterialInstance TilesCreateLava();
MaterialInstance TilesCreateCloud(int x, int y);
MaterialInstance TilesCreateGold(int x, int y);
MaterialInstance TilesCreateIron(int x, int y);
MaterialInstance TilesCreateObsidian(int x, int y);
MaterialInstance TilesCreateSteam();
MaterialInstance TilesCreateFire();
MaterialInstance TilesCreate(Material *mat, int x, int y);
MaterialInstance TilesCreate(int id, int x, int y);

#pragma endregion Material

class Structure {
public:
    MaterialInstance *tiles;
    int w;
    int h;

    Structure(int w, int h, MaterialInstance *tiles);
    Structure(C_Surface *texture, Material templ);
    Structure() = default;
};

class World;

class Structures {
public:
    static Structure makeTree(World world, int x, int y);
    static Structure makeTree1(World world, int x, int y);
};

class PlacedStructure {
public:
    Structure base;
    int x;
    int y;

    PlacedStructure(Structure base, int x, int y);
    PlacedStructure(const PlacedStructure &p2) {
        this->base = Structure(base);
        this->x = x;
        this->y = y;
    }
};

class Biome {
public:
    int id = -1;
    std::string name;
    explicit Biome(std::string name, int id) : name(std::move(name)), id(std::move(id)){};
};

template <>
struct ME::meta::static_refl::TypeInfo<Biome> : TypeInfoBase<Biome> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            Field{TSTR("id"), &Type::id},
            Field{TSTR("name"), &Type::name},
    };
};

ME_GUI_DEFINE_BEGIN(template <>, Biome)
ImGui::Text("Name: %s", var.name.c_str());
ImGui::Text("ID: %d", var.id);
ME_GUI_DEFINE_END

Biome *BiomeGet(std::string name);
ME_INLINE int BiomeGetID(std::string name) { return BiomeGet(name)->id; }

struct WorldGenerator {
    virtual void generateChunk(World *world, Chunk *ch) = 0;
    virtual std::vector<Populator *> getPopulators() = 0;
};

struct Populator {
    virtual int getPhase() = 0;
    virtual std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk **area, bool *dirty, int tx, int ty, int tw, int th, Chunk *ch, World *world) = 0;
};

#pragma region Populators

struct TestPhase1Populator : public Populator {
    int getPhase() { return 1; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk *area, bool *dirty, int tx, int ty, int tw, int th, Chunk ch, World *world);
};

struct TestPhase2Populator : public Populator {
    int getPhase() { return 2; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk *area, bool *dirty, int tx, int ty, int tw, int th, Chunk ch, World *world);
};

struct TestPhase3Populator : public Populator {
    int getPhase() { return 3; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk *area, bool *dirty, int tx, int ty, int tw, int th, Chunk ch, World *world);
};

struct TestPhase4Populator : public Populator {
    int getPhase() { return 4; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk *area, bool *dirty, int tx, int ty, int tw, int th, Chunk ch, World *world);
};

struct TestPhase5Populator : public Populator {
    int getPhase() { return 5; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk *area, bool *dirty, int tx, int ty, int tw, int th, Chunk ch, World *world);
};

struct TestPhase6Populator : public Populator {
    int getPhase() { return 6; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk *area, bool *dirty, int tx, int ty, int tw, int th, Chunk ch, World *world);
};

struct TestPhase0Populator : public Populator {
    int getPhase() { return 0; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk *area, bool *dirty, int tx, int ty, int tw, int th, Chunk ch, World *world);
};

struct CavePopulator : public Populator {
    int getPhase() { return 0; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk **area, bool *dirty, int tx, int ty, int tw, int th, Chunk *ch, World *world);
};

struct CobblePopulator : public Populator {
    int getPhase() { return 1; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk **area, bool *dirty, int tx, int ty, int tw, int th, Chunk *ch, World *world);
};

struct OrePopulator : public Populator {
    int getPhase() { return 0; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk **area, bool *dirty, int tx, int ty, int tw, int th, Chunk *ch, World *world);
};

struct TreePopulator : public Populator {
    int getPhase() { return 1; }
    std::vector<PlacedStructure> apply(MaterialInstance *chunk, MaterialInstance *layer2, Chunk **area, bool *dirty, int tx, int ty, int tw, int th, Chunk *ch, World *world);
};

#pragma endregion Populators

#endif
