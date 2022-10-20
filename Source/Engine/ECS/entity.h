// Copyright(c) 2022, KaoruXun All rights reserved.

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>

#include "container.h"
#include "exception.h"
#include "metafunctions.h"
#include "typelist.h"

namespace MetaEngine::ECS {
    enum class entity_status {
        UNINITIALIZED,
        DELETED,
        STALE,
        OK
    };

    class entity_grouping;

    template<typename...>
    class event_manager;

    struct control_block_t
    {
        bool breakout = false;
    };

    // Safety classes so that you can only create using the proper list types
    template<typename Components, typename Tags>
    class entity_manager {
        static_assert(meta::delay<Components, Tags>,
                      "The template parameters must be of type component_list and tag_list");
    };

    namespace detail {
        using entity_grouping_id_t = std::uintmax_t;

        template<typename, typename>
        struct entity_event_manager;

        template<typename Components, typename Tags>
        class entity {
            static_assert(meta::delay<Components, Tags>, "Don't create entities manually, use "
                                                         "entity_manager::entity_t or create_entity() "
                                                         "instead");
        };

        template<typename... Components, typename... Tags>
        class entity<component_list<Components...>, tag_list<Tags...>> {
        public:
            using component_list_t = component_list<Components...>;
            using tag_list_t = tag_list<Tags...>;
            using entity_manager_t = entity_manager<component_list_t, tag_list_t>;

        private:
            using component_t = meta::typelist<Components...>;
            using tag_t = meta::typelist<Tags...>;
            using comp_tag_t = meta::typelist<Components..., Tags...>;

            friend entity_manager_t;
            struct private_access
            {
                explicit private_access() = default;
            };

            detail::entity_id_t id;
            entity_manager_t *entityManager = nullptr;
            mutable meta::type_bitset<comp_tag_t> compTags;

        public:
            entity() = default;

            entity(private_access, detail::entity_id_t id, entity_manager_t *entityManager) noexcept;

            entity_status get_status() const;

            template<typename Component>
            bool has_component() const;

            // Adds the component if it doesn't exist, otherwise returns the existing component
            template<typename Component, typename... Args>
            std::pair<Component &, bool> add_component(Args &&...args);
            template<typename Component>
            auto add_component(Component &&comp);

            // Returns if the component was removed or not (in the case that it didn't exist)
            template<typename Component>
            bool remove_component();

            // Must have component in order to get it, otherwise you have an invalid_component exception
            template<typename Component>
            Component &get_component();
            template<typename Component>
            const Component &get_component() const;

            template<typename Tag>
            bool has_tag() const;

            // Returns the previous tag value
            template<typename Tag>
            bool set_tag(bool set);

            // Updates entity to the stored one, returns false if deleted
            bool sync() const;

            void destroy();

            bool operator<(const entity &other) const noexcept;
            bool operator==(const entity &other) const noexcept;
        };
    }// namespace detail

    template<typename... Components, typename... Tags>
    class entity_manager<component_list<Components...>, tag_list<Tags...>> {
    public:
        using component_list_t = component_list<Components...>;
        using tag_list_t = tag_list<Tags...>;
        using entity_t = detail::entity<component_list_t, tag_list_t>;

    private:
        using component_t = meta::typelist<Components...>;
        using tag_t = meta::typelist<Tags...>;
        using comp_tag_t = meta::typelist<Components..., Tags...>;
        using entity_container = flat_set<entity_t>;
        using entity_event_manager_t = detail::entity_event_manager<component_list_t, tag_list_t>;

        friend entity_t;
        friend entity_grouping;

        static_assert(meta::is_typelist_unique_v<comp_tag_t>,
                      "component_list and tag_list must not intersect");

        constexpr static auto ComponentCount = sizeof...(Components);
        constexpr static auto TagCount = sizeof...(Tags);
        constexpr static auto CompTagCount = ComponentCount + TagCount;

        detail::entity_id_t currentEntityId = 0;
        typename component_list_t::type components;
        entity_container entities;
        std::size_t maxLinearSearchDistance = 64;
        const entity_event_manager_t *eventManager = nullptr;
        detail::entity_grouping_id_t currentGroupingId = CompTagCount;
        flat_map<detail::entity_grouping_id_t,
                 std::pair<meta::type_bitset<comp_tag_t>, entity_container>>
                groupings;

        [[noreturn]] void report_error(error_code errCode, const char *msg) const;

        std::pair<const entity_t *, entity_status> get_entity_and_status(const entity_t &entity) const;

        entity_t &assert_entity(const entity_t &entity);
        const entity_t &assert_entity(const entity_t &entity) const;

        template<typename T>
        void add_bit(entity_t &local, entity_t &foreign);

        template<typename T>
        void remove_bit(entity_t &local, entity_t &foreign);

        template<typename... Ts>
        std::pair<entity_container &, bool> get_smallest_container();

        template<typename Component, typename... Args>
        std::pair<Component &, bool> add_component(entity_t &entity, Args &&...args);

        template<typename... Ts>
        void add_components(entity_t &entity, Ts &&...ts);

        template<typename Component>
        bool remove_component(entity_t &entity);

        template<typename Component>
        const Component &get_component(const entity_t &entity) const;

        template<typename Tag>
        bool set_tag(entity_t &entity, bool set);

        // Expose this?
        template<typename... Ts>
        void set_tags(entity_t &entity, bool set);

        bool sync(const entity_t &entity) const;

        void destroy_entity(entity_t &entity);

        void destroy_grouping(detail::entity_grouping_id_t id);

    public:
        using return_container = std::vector<entity_t>;

        entity_manager();
        entity_manager(const entity_manager &) = delete;
        entity_manager &operator=(const entity_manager &) = delete;

        template<typename... Ts, typename... Us>
        entity_t create_entity(Us &&...us);

        // Gets all entities that have the components and tags provided
        template<typename... Ts>
        return_container get_entities();

        template<typename... Ts, typename Func>
        void for_each(Func &&func);

        // No error handling (what if the grouping already exists?)
        template<typename... Ts>
        entity_grouping create_grouping();

        template<typename... Events>
        void set_event_manager(const event_manager<component_list_t, tag_list_t, Events...> &em);

        void clear_event_manager();

#ifdef METADOT_ECS_NO_EXCEPTIONS
        using error_callback_t = void(error_code, const char *);

    private:
        std::function<error_callback_t> errorCallback;

        [[noreturn]] void handle_error(error_code err, const char *msg) const;

    public:
        void set_error_callback(std::function<error_callback_t> cb);
#endif
    };

    class entity_grouping {
        detail::entity_grouping_id_t id;
        void *manager = nullptr;
        void (*destroy_ptr)(entity_grouping *);

        template<typename CTs, typename TTs>
        static void destroy_impl(entity_grouping *self) {
            auto em = static_cast<entity_manager<CTs, TTs> *>(self->manager);
            em->destroy_grouping(self->id);
        }

        template<typename, typename>
        friend class entity_manager;
        struct private_access
        {
            explicit private_access() = default;
        };

    public:
        entity_grouping() = default;

        template<typename CTs, typename TTs>
        entity_grouping(private_access, entity_manager<CTs, TTs> &em,
                        detail::entity_grouping_id_t id) noexcept;

        entity_grouping(entity_grouping &&other) noexcept;
        entity_grouping &operator=(entity_grouping &&other) noexcept;

        bool is_valid() const;

        bool destroy();
    };
}// namespace MetaEngine::ECS

// INC

// Copyright(c) 2022, KaoruXun All rights reserved.

#include <algorithm>
#include <type_traits>

#include "event.h"

namespace MetaEngine::ECS {
    namespace detail {
#define ENTITY_TEMPS template<typename... CTs, typename... TTs>
#define ENTITY_SPEC entity<component_list<CTs...>, tag_list<TTs...>>

        ENTITY_TEMPS
        ENTITY_SPEC::entity(private_access, detail::entity_id_t id,
                            entity_manager_t *entityManager) noexcept
            : id(id), entityManager(entityManager) {}

        ENTITY_TEMPS
        entity_status ENTITY_SPEC::get_status() const {
            if (!entityManager) {
                return entity_status::UNINITIALIZED;
            }
            return entityManager->get_entity_and_status(*this).second;
        }

        ENTITY_TEMPS
        template<typename Component>
        bool ENTITY_SPEC::has_component() const {
            assert(entityManager);
            constexpr bool isCompValid = meta::typelist_has_type_v<Component, component_t>;

            METADOT_ECS_CHECK(isCompValid, "has_component called with invalid component")
            else {
                return meta::get<Component>(compTags);
            }
        }

        ENTITY_TEMPS
        template<typename Component, typename... Args>
        std::pair<Component &, bool> ENTITY_SPEC::add_component(Args &&...args) {
            assert(entityManager);
            constexpr bool isCompValid = meta::typelist_has_type_v<Component, component_t>;
            constexpr bool isConstructible = std::is_constructible_v<Component, Args &&...>;

            METADOT_ECS_CHECK(isCompValid, "add_component called with invalid component")
            METADOT_ECS_CHECK_ALSO(isConstructible,
                                   "add_component cannot construct component with given args")
            else {
                return entityManager->template add_component<Component>(*this,
                                                                        std::forward<Args>(args)...);
            }
        }

        ENTITY_TEMPS
        template<typename Component>
        auto ENTITY_SPEC::add_component(Component &&comp) {
            return add_component<std::decay_t<Component>, Component>(std::forward<Component>(comp));
        }

        ENTITY_TEMPS
        template<typename Component>
        bool ENTITY_SPEC::remove_component() {
            assert(entityManager);
            constexpr bool isCompValid = meta::typelist_has_type_v<Component, component_t>;

            METADOT_ECS_CHECK(isCompValid, "remove_component called with invalid component")
            else {
                return entityManager->template remove_component<Component>(*this);
            }
        }

        ENTITY_TEMPS
        template<typename Component>
        Component &ENTITY_SPEC::get_component() {
            return const_cast<Component &>(std::as_const(*this).template get_component<Component>());
        }

        ENTITY_TEMPS
        template<typename Component>
        const Component &ENTITY_SPEC::get_component() const {
            assert(entityManager);
            constexpr bool isCompValid = meta::typelist_has_type_v<Component, component_t>;

            METADOT_ECS_CHECK(isCompValid, "get_component called with invalid component")
            else {
                return entityManager->template get_component<Component>(*this);
            }
        }

        ENTITY_TEMPS
        template<typename Tag>
        bool ENTITY_SPEC::has_tag() const {
            assert(entityManager);
            constexpr bool isTagValid = meta::typelist_has_type_v<Tag, tag_t>;

            METADOT_ECS_CHECK(isTagValid, "has_tag called with invalid tag")
            else {
                return meta::get<Tag>(compTags);
            }
        }

        ENTITY_TEMPS
        template<typename Tag>
        bool ENTITY_SPEC::set_tag(bool set) {
            assert(entityManager);
            constexpr bool isTagValid = meta::typelist_has_type_v<Tag, tag_t>;

            METADOT_ECS_CHECK(isTagValid, "set_tag called with invalid tag")
            else {
                return entityManager->template set_tag<Tag>(*this, set);
            }
        }

        ENTITY_TEMPS
        bool ENTITY_SPEC::sync() const {
            assert(entityManager);
            return entityManager->sync(*this);
        }

        ENTITY_TEMPS
        void ENTITY_SPEC::destroy() {
            assert(entityManager);
            entityManager->destroy_entity(*this);
        }

        ENTITY_TEMPS
        bool ENTITY_SPEC::operator<(const entity &other) const noexcept {
            assert(entityManager && entityManager == other.entityManager);
            return id < other.id;
        }

        ENTITY_TEMPS
        bool ENTITY_SPEC::operator==(const entity &other) const noexcept {
            assert(entityManager && entityManager == other.entityManager);
            return id == other.id;
        }

#undef ENTITY_TEMPS
#undef ENTITY_SPEC
    }// namespace detail

#define ENTITY_MANAGER_TEMPS template<typename... CTs, typename... TTs>
#define ENTITY_MANAGER_SPEC entity_manager<component_list<CTs...>, tag_list<TTs...>>

    ENTITY_MANAGER_TEMPS void ENTITY_MANAGER_SPEC::report_error(error_code errCode,
                                                                const char *msg) const {
#ifdef METADOT_ECS_NO_EXCEPTIONS
        handle_error(errCode, msg);
#else
        switch (errCode) {
            case error_code::BAD_ENTITY:
                throw bad_entity(msg);
            case error_code::INVALID_COMPONENT:
                throw invalid_component(msg);
        }
        // unreachable
        assert(0);
        std::terminate();
#endif
    }

    ENTITY_MANAGER_TEMPS
    auto ENTITY_MANAGER_SPEC::get_entity_and_status(const entity_t &entity) const
            -> std::pair<const entity_t *, entity_status> {
        auto local = entities.find(entity);
        if (local == entities.end()) {
            return {nullptr, entity_status::DELETED};
        }
        if (entity.compTags != local->compTags) {
            return {&*local, entity_status::STALE};
        }
        return {&*local, entity_status::OK};
    }

    ENTITY_MANAGER_TEMPS
    auto ENTITY_MANAGER_SPEC::assert_entity(const entity_t &entity) -> entity_t & {
        return const_cast<entity_t &>(std::as_const(*this).assert_entity(entity));
    }

    ENTITY_MANAGER_TEMPS
    auto ENTITY_MANAGER_SPEC::assert_entity(const entity_t &entity) const -> const entity_t & {
#if NDEBUG
        return *entities.find(entity);
#else
        auto [localEnt, status] = get_entity_and_status(entity);
        switch (status) {
            case entity_status::DELETED:
                report_error(error_code::BAD_ENTITY, "Entity has been deleted.");
            case entity_status::STALE:
                report_error(error_code::BAD_ENTITY,
                             "Entity's components/tags are stale. Update entities with sync().");
            case entity_status::OK:
                return *localEnt;
            case entity_status::UNINITIALIZED:
                // impossible
                break;
        }
        // unreachable
        assert(0);
        std::terminate();
#endif
    }

    ENTITY_MANAGER_TEMPS
    template<typename T>
    void ENTITY_MANAGER_SPEC::add_bit(entity_t &local, entity_t &foreign) {
        auto prevBits = local.compTags;

        meta::get<T>(local.compTags) = meta::get<T>(foreign.compTags) = true;

        meta::type_bitset<comp_tag_t> singleBit;
        meta::get<T>(singleBit) = true;
        for (auto &[id, grouping]: groupings) {
            auto &[groupingBitset, groupingContainer] = grouping;
            bool wasInGrouping = (groupingBitset & prevBits) == groupingBitset;
            bool enteredGrouping = (groupingBitset & (prevBits | singleBit)) == groupingBitset;

            if (!wasInGrouping && enteredGrouping) {
                auto [itr, emplaced] = groupingContainer.emplace(local);
                assert(emplaced);
            } else if (wasInGrouping) {
                auto ent = groupingContainer.find(local);
                assert(ent != groupingContainer.end());
                meta::get<T>(ent->compTags) = true;
            }
        }
    }

    ENTITY_MANAGER_TEMPS
    template<typename T>
    void ENTITY_MANAGER_SPEC::remove_bit(entity_t &local, entity_t &foreign) {
        meta::get<T>(local.compTags) = meta::get<T>(foreign.compTags) = false;

        meta::type_bitset<comp_tag_t> singleBit;
        meta::get<T>(singleBit) = true;
        for (auto &[id, grouping]: groupings) {
            auto &[groupingBitset, groupingContainer] = grouping;
            bool inGrouping = (groupingBitset & local.compTags) == groupingBitset;
            bool wasInGrouping = (groupingBitset & (local.compTags | singleBit)) == groupingBitset;
            if (!inGrouping && wasInGrouping) {
                auto er = groupingContainer.erase(local);
                assert(er == 1);
            } else if (inGrouping) {
                auto ent = groupingContainer.find(local);
                assert(ent != groupingContainer.end());
                meta::get<T>(ent->compTags) = false;
            }
        }
    }

    ENTITY_MANAGER_TEMPS
    template<typename... Ts>
    auto ENTITY_MANAGER_SPEC::get_smallest_container() -> std::pair<entity_container &, bool> {
        if constexpr (sizeof...(Ts) == 0) {
            return {entities, true};
        } else if constexpr (sizeof...(Ts) == 1) {
            return {groupings.find(meta::typelist_index_v<Ts..., comp_tag_t>)->second.second, true};
        } else {
            auto key = meta::make_key<meta::typelist<Ts...>, comp_tag_t>();
            auto &[bitset, container] =
                    std::min_element(groupings.begin(), groupings.end(),
                                     [&key](const auto &lhs, const auto &rhs) {
                                         if ((lhs.second.first & key) == lhs.second.first &&
                                             (rhs.second.first & key) == rhs.second.first) {
                                             return lhs.second.second.size() < rhs.second.second.size();
                                         }
                                         return (lhs.second.first & key) == lhs.second.first &&
                                                (rhs.second.first & key) != rhs.second.first;
                                     })
                            ->second;
            return {container, bitset == key};
        }
    }

    ENTITY_MANAGER_TEMPS
    template<typename Component, typename... Args>
    std::pair<Component &, bool> ENTITY_MANAGER_SPEC::add_component(entity_t &entity, Args &&...args) {
        auto &myEnt = assert_entity(entity);
        assert(meta::get<Component>(entity.compTags) == meta::get<Component>(myEnt.compTags));

        auto &container = meta::get<Component, component_list_t>(components);
        if (meta::get<Component>(entity.compTags)) {
            auto comp = container.find(entity.id);
            assert(comp != container.end());
            return {comp->second, false};
        }

        auto [itr, emplaced] = container.try_emplace(entity.id, std::forward<Args>(args)...);
        assert(emplaced);

        add_bit<Component>(myEnt, entity);

        if (eventManager) {
            eventManager->broadcast(component_added<entity_t, Component>{myEnt, itr->second});
        }

        return {itr->second, true};
    }

    ENTITY_MANAGER_TEMPS
    template<typename... Ts>
    void ENTITY_MANAGER_SPEC::add_components(entity_t &entity, Ts &&...ts) {
        ((void) add_component<std::decay_t<Ts>>(entity, std::forward<Ts>(ts)), ...);
    }

    ENTITY_MANAGER_TEMPS
    template<typename Component>
    bool ENTITY_MANAGER_SPEC::remove_component(entity_t &entity) {
        auto &myEnt = assert_entity(entity);
        assert(meta::get<Component>(entity.compTags) == meta::get<Component>(myEnt.compTags));
        if (!meta::get<Component>(entity.compTags)) {
            return false;
        }

        auto &container = meta::get<Component, component_list_t>(components);
        auto comp = container.find(entity.id);
        assert(comp != container.end());

        if (eventManager) {
            eventManager->broadcast(component_removed<entity_t, Component>{myEnt, comp->second});
        }

        container.erase(comp);

        remove_bit<Component>(myEnt, entity);

        return true;
    }

    ENTITY_MANAGER_TEMPS
    template<typename Component>
    const Component &ENTITY_MANAGER_SPEC::get_component(const entity_t &entity) const {
        auto &myEnt = assert_entity(entity);
        assert(meta::get<Component>(entity.compTags) == meta::get<Component>(myEnt.compTags));
        // TODO: Possibly remove check on non debug build?
        if (!meta::get<Component>(entity.compTags)) {
            report_error(error_code::INVALID_COMPONENT,
                         "Tried to get a component the entity does not have");
        }

        const auto &container = meta::get<Component, component_list_t>(components);
        auto comp = container.find(entity.id);
        assert(comp != container.end());
        return comp->second;
    }

    ENTITY_MANAGER_TEMPS
    template<typename Tag>
    bool ENTITY_MANAGER_SPEC::set_tag(entity_t &entity, bool set) {
        auto &myEnt = assert_entity(entity);
        assert(meta::get<Tag>(entity.compTags) == meta::get<Tag>(myEnt.compTags));

        bool old = meta::get<Tag>(entity.compTags);
        if (old != set) {
            if (set) {
                add_bit<Tag>(myEnt, entity);
                if (eventManager) {
                    eventManager->broadcast(tag_added<entity_t, Tag>{myEnt});
                }
            } else {
                if (eventManager) {
                    eventManager->broadcast(tag_removed<entity_t, Tag>{myEnt});
                }
                remove_bit<Tag>(myEnt, entity);
            }
        }
        return old;
    }

    ENTITY_MANAGER_TEMPS
    template<typename... Ts>
    void ENTITY_MANAGER_SPEC::set_tags(entity_t &entity, bool set) {
        ((void) set_tag<Ts>(entity, set), ...);
    }

    ENTITY_MANAGER_TEMPS
    bool ENTITY_MANAGER_SPEC::sync(const entity_t &entity) const {
        auto [local, status] = get_entity_and_status(entity);
        if (status == entity_status::DELETED) {
            return false;
        }
        entity.compTags = local->compTags;
        return true;
    }

    ENTITY_MANAGER_TEMPS
    void ENTITY_MANAGER_SPEC::destroy_entity(entity_t &entity) {
        assert_entity(entity);

        if (eventManager) {
            eventManager->broadcast(entity_destroyed<entity_t>{entity});
        }
        std::apply(
                [&](auto &...containers) {
                    auto broadcaster = [&, idx = 0](auto &container) mutable {
                        if (entity.compTags[idx]) {
                            auto comp = container.find(entity.id);
                            assert(comp != container.end());
                            if (eventManager) {
                                eventManager->broadcast(
                                        component_removed<
                                                entity_t, typename std::decay_t<decltype(container)>::mapped_type>{
                                                entity, comp->second});
                            }
                            container.erase(comp);
                        }
                        ++idx;
                    };
                    (broadcaster(containers), ...);
                },
                components);

        meta::for_each<ComponentCount>(entity.compTags, [&](std::size_t idx, auto type_holder) {
            if (entity.compTags[idx]) {
                if (eventManager) {
                    eventManager->broadcast(
                            tag_removed<entity_t, typename decltype(type_holder)::type>{entity});
                }
            }
        });

        for (auto &[id, grouping]: groupings) {
            auto &[groupingBitset, groupingContainer] = grouping;
            if ((groupingBitset & entity.compTags) == groupingBitset) {
                auto er = groupingContainer.erase(entity);
                assert(er == 1);
            }
        }

        auto er = entities.erase(entity);
        assert(er == 1);
    }

    ENTITY_MANAGER_TEMPS
    void ENTITY_MANAGER_SPEC::destroy_grouping(detail::entity_grouping_id_t id) {
        auto er = groupings.erase(id);
        assert(er == 1);
    }

    namespace detail {
        template<typename... Ts, typename Container, std::size_t... Is>
        void initialize_groupings(
                flat_map<entity_grouping_id_t,
                         std::pair<meta::type_bitset<meta::typelist<Ts...>>, Container>> &groupings,
                std::index_sequence<Is...>) {
            ((void) groupings.try_emplace(
                     Is, meta::make_key<meta::typelist<Ts>, meta::typelist<Ts...>>(), Container{}),
             ...);
        }
    }// namespace detail

    ENTITY_MANAGER_TEMPS
    ENTITY_MANAGER_SPEC::entity_manager() {
        detail::initialize_groupings(groupings, std::index_sequence_for<CTs..., TTs...>{});
    }

    ENTITY_MANAGER_TEMPS
    template<typename... Ts, typename... Us>
    auto ENTITY_MANAGER_SPEC::create_entity(Us &&...us) -> entity_t {
        constexpr bool areTagsValid = (meta::typelist_has_type_v<Ts, tag_t> && ...);
        constexpr bool areTagsUnique = meta::is_typelist_unique_v<meta::typelist<Ts...>>;

        constexpr bool areCompsValid =
                (meta::typelist_has_type_v<std::decay_t<Us>, component_t> && ...);
        constexpr bool areCompsUnique = meta::is_typelist_unique_v<meta::typelist<std::decay_t<Us>...>>;

        METADOT_ECS_CHECK(areTagsValid, "create_entity called with invalid tags")
        METADOT_ECS_CHECK_ALSO(areTagsUnique, "create_entity called with non-unique tags")
        METADOT_ECS_CHECK_ALSO(areCompsValid, "create_entity called with invalid components")
        METADOT_ECS_CHECK_ALSO(areCompsUnique, "create_entity called with non-unique tags")
        else {
            assert(std::numeric_limits<detail::entity_id_t>::max() != currentEntityId);
            auto [itr, emplaced] =
                    entities.emplace(typename entity_t::private_access{}, currentEntityId++, this);
            assert(emplaced);

            auto &ent = *itr;

            if (eventManager) {
                eventManager->broadcast(entity_created<entity_t>{ent});
            }

            set_tags<Ts...>(ent, true);
            add_components(ent, std::forward<Us>(us)...);

            return ent;
        }
    }

    ENTITY_MANAGER_TEMPS
    template<typename... Ts>
    auto ENTITY_MANAGER_SPEC::get_entities() -> return_container {
        using Typelist = meta::typelist<Ts...>;
        constexpr bool isTypelistValid = (meta::typelist_has_type_v<Ts, comp_tag_t> && ...);
        constexpr bool isTypelistUnique = meta::is_typelist_unique_v<Typelist>;

        METADOT_ECS_CHECK(isTypelistValid, "get_entitites called with invalid typelist")
        METADOT_ECS_CHECK_ALSO(isTypelistUnique, "get_entitites called with a non-unique typelist")
        else {
            auto [container, encompassing] = get_smallest_container<Ts...>();
            if (encompassing) {
                return {container.begin(), container.end()};
            }
            return_container ret;
            ret.reserve(container.size());

            auto key = meta::make_key<Typelist, comp_tag_t>();

            for (const auto &ent: container) {
                if ((ent.compTags & key) == key) {
                    ret.push_back(ent);
                }
            }
            return ret;
        }
    }

    namespace detail {
        template<typename T, typename Func, typename U>
        struct can_call_no_control;
        template<typename T, typename Func, typename... Ts>
        struct can_call_no_control<T, Func, meta::typelist<Ts...>>
        {
            static constexpr bool value = std::is_invocable_r_v<void, Func, T, Ts &...>;
        };

        template<typename T, typename Func, typename U>
        struct can_call_with_control;
        template<typename T, typename Func, typename... Ts>
        struct can_call_with_control<T, Func, meta::typelist<Ts...>>
        {
            static constexpr bool value =
                    std::is_invocable_r_v<void, Func, T, Ts &..., control_block_t &>;
        };

        template<typename Iter>
        struct data_t
        {
            Iter pos, end;
            bool useLinear;
        };

        template<typename Iter>
        auto make_data(Iter begin, Iter end, bool use) {
            return data_t<Iter>{begin, end, use};
        }

        template<typename T, typename U>
        struct make_iters;
        template<typename T, typename... Us>
        struct make_iters<T, meta::typelist<Us...>>
        {
            template<typename Container>
            auto operator()(Container &c, std::size_t smallestIdxSize,
                            std::size_t maxLinearSearchDistance) const {
                return std::make_tuple(make_data(meta::get<Us, T>(c).begin(), meta::get<Us, T>(c).end(),
                                                 meta::get<Us, T>(c).size() / smallestIdxSize <
                                                         maxLinearSearchDistance)...);
            }
        };
    }// namespace detail

    ENTITY_MANAGER_TEMPS
    template<typename... Ts, typename Func>
    void ENTITY_MANAGER_SPEC::for_each(Func &&func) {
        using Typelist = meta::typelist<Ts...>;
        using ComponentsPart = meta::typelist_intersection_t<Typelist, component_t>;
        constexpr bool canCallWithoutControl =
                detail::can_call_no_control<entity_t, Func, ComponentsPart>::value;
        constexpr bool canCallWithControl =
                detail::can_call_with_control<entity_t, Func, ComponentsPart>::value;
        constexpr bool isTypelistUnique = meta::is_typelist_unique_v<Typelist>;
        constexpr bool isTypelistValid = (meta::typelist_has_type_v<Ts, comp_tag_t> && ...);

        METADOT_ECS_CHECK(isTypelistValid, "for_each called with invalid typelist")
        METADOT_ECS_CHECK_ALSO(isTypelistUnique, "for_each called with a non-unique typelist")
        METADOT_ECS_CHECK_ALSO(canCallWithoutControl || canCallWithControl,
                               "for_each called with invalid callable")
        else {
            auto [container, encompassing] = get_smallest_container<Ts...>();
            if (container.size() == 0) {
                return;
            }
            auto iters = detail::make_iters<component_list_t, ComponentsPart>{}(
                    components, container.size(), maxLinearSearchDistance);
            auto key = meta::make_key<Typelist, comp_tag_t>();
            for (const auto &ent: container) {
                if (!encompassing && (ent.compTags & key) != key) {
                    continue;
                }

                std::apply(
                        [&](auto &...iters) {
                            auto increment_iter = [&](auto &iter) {
                                if (iter.useLinear) {
                                    while (iter.pos->first < ent.id) {
                                        ++iter.pos;
                                    }
                                } else {
                                    iter.pos = std::lower_bound(
                                            iter.pos, iter.end, ent.id,
                                            [](const auto &it, const auto &val) { return it.first < val; });
                                }
                            };
                            (increment_iter(iters), ...);
                        },
                        iters);

                if constexpr (canCallWithControl) {
                    control_block_t control;
                    std::apply([&](auto &...iters) { func(ent, iters.pos->second..., control); },
                               iters);
                    if (control.breakout) {
                        break;
                    }
                } else {
                    std::apply([&](auto &...iters) { func(ent, iters.pos->second...); }, iters);
                }
            }
        }
    }

    ENTITY_MANAGER_TEMPS
    template<typename... Ts>
    entity_grouping ENTITY_MANAGER_SPEC::create_grouping() {
        using Typelist = meta::typelist<Ts...>;
        constexpr bool isTypelistValid = (meta::typelist_has_type_v<Ts, comp_tag_t> && ...);
        constexpr bool isTypelistUnique = meta::is_typelist_unique_v<Typelist>;
        constexpr bool isGoodGrouping = sizeof...(Ts) > 1;

        METADOT_ECS_CHECK(isTypelistValid, "create_grouping called with invalid typelist")
        METADOT_ECS_CHECK_ALSO(isTypelistUnique, "create_grouping called with a non-unique typelist")
        METADOT_ECS_CHECK_ALSO(isGoodGrouping, "create_grouping called with 0 or 1 components/tags")
        else {
            assert(std::numeric_limits<detail::entity_grouping_id_t>::max() != currentGroupingId);
            auto [itr, emplaced] =
                    groupings.try_emplace(currentGroupingId++, meta::make_key<Typelist, comp_tag_t>(),
                                          entity_container{get_entities<Ts...>()});
            assert(emplaced);

            return {entity_grouping::private_access{}, *this, itr->first};
        }
    }

    ENTITY_MANAGER_TEMPS
    template<typename... Events>
    void ENTITY_MANAGER_SPEC::set_event_manager(
            const event_manager<component_list_t, tag_list_t, Events...> &em) {
        eventManager = &em.get_entity_event_manager();
    }

    ENTITY_MANAGER_TEMPS
    void ENTITY_MANAGER_SPEC::clear_event_manager() {
        eventManager = nullptr;
    }

#ifdef METADOT_ECS_NO_EXCEPTIONS
    ENTITY_MANAGER_TEMPS
    void ENTITY_MANAGER_SPEC::handle_error(error_code err, const char *msg) const {
        if (errorCallback) {
            errorCallback(err, msg);
        }
        assert(0);
        std::terminate();
    }

    ENTITY_MANAGER_TEMPS
    void ENTITY_MANAGER_SPEC::set_error_callback(std::function<error_callback_t> cb) {
        errorCallback = std::move(cb);
    }
#endif

#undef ENTITY_MANAGER_TEMPS
#undef ENTITY_MANAGER_SPEC

    template<typename CTs, typename TTs>
    inline entity_grouping::entity_grouping(private_access, entity_manager<CTs, TTs> &em,
                                            detail::entity_grouping_id_t id) noexcept
        : id(id), manager(&em), destroy_ptr(&destroy_impl<CTs, TTs>) {}

    inline entity_grouping::entity_grouping(entity_grouping &&other) noexcept {
        *this = std::move(other);
    }

    inline entity_grouping &entity_grouping::operator=(entity_grouping &&other) noexcept {
        id = other.id;
        manager = std::exchange(other.manager, nullptr);
        destroy_ptr = other.destroy_ptr;

        return *this;
    }

    inline bool entity_grouping::is_valid() const {
        return manager != nullptr;
    }

    inline bool entity_grouping::destroy() {
        if (manager) {
            destroy_ptr(this);
            manager = nullptr;
            return true;
        }
        return false;
    }
}// namespace MetaEngine::ECS
