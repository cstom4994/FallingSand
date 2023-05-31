
#ifndef ME_STRUCT_HPP
#define ME_STRUCT_HPP

#include <type_traits>
#include <utility>

namespace MetaEngine::Struct {

namespace traits {

// Primary template which is specialized to register a type
template <typename T, typename ENABLE = void>
struct visitable;

// Helper template which checks if a type is registered
template <typename T, typename ENABLE = void>
struct is_visitable : std::false_type {};

template <typename T>
struct is_visitable<T, typename std::enable_if<traits::visitable<T>::value>::type> : std::true_type {};

// Helper template which removes cv and reference from a type (saves some typing)
template <typename T>
struct clean {
    typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
};

template <typename T>
using clean_t = typename clean<T>::type;

// Mini-version of std::common_type (we only require C++11)
template <typename T, typename U>
struct common_type {
    typedef decltype(true ? std::declval<T>() : std::declval<U>()) type;
};

}  // end namespace traits

// Tag for tag dispatch
template <typename T>
struct type_c {
    using type = T;
};

// Accessor type: function object encapsulating a pointer-to-member
template <typename MemPtr, MemPtr ptr>
struct accessor {
    template <typename T>
    constexpr auto operator()(T &&t) const -> decltype(std::forward<T>(t).*ptr) {
        return std::forward<T>(t).*ptr;
    }
};

//
// User-interface
//

// Return number of fields in a visitable struct
template <typename S>
constexpr std::size_t field_count() {
    return traits::visitable<traits::clean_t<S>>::field_count;
}

template <typename S>
constexpr std::size_t field_count(S &&) {
    return field_count<S>();
}

// apply_visitor (one struct instance)
template <typename S, typename V>
constexpr auto apply_visitor(V &&v, S &&s) -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
    traits::visitable<traits::clean_t<S>>::apply(std::forward<V>(v), std::forward<S>(s));
}

// apply_visitor (two struct instances)
template <typename S1, typename S2, typename V>
constexpr auto apply_visitor(V &&v, S1 &&s1, S2 &&s2) -> typename std::enable_if<traits::is_visitable<traits::clean_t<typename traits::common_type<S1, S2>::type>>::value>::type {
    using common_S = typename traits::common_type<S1, S2>::type;
    traits::visitable<traits::clean_t<common_S>>::apply(std::forward<V>(v), std::forward<S1>(s1), std::forward<S2>(s2));
}

// for_each (Alternate syntax for apply_visitor, reverses order of arguments)
template <typename V, typename S>
constexpr auto for_each(S &&s, V &&v) -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
    traits::visitable<traits::clean_t<S>>::apply(std::forward<V>(v), std::forward<S>(s));
}

// for_each with two structure instances
template <typename S1, typename S2, typename V>
constexpr auto for_each(S1 &&s1, S2 &&s2, V &&v) -> typename std::enable_if<traits::is_visitable<traits::clean_t<typename traits::common_type<S1, S2>::type>>::value>::type {
    using common_S = typename traits::common_type<S1, S2>::type;
    traits::visitable<traits::clean_t<common_S>>::apply(std::forward<V>(v), std::forward<S1>(s1), std::forward<S2>(s2));
}

// Visit the types (MetaEngine::Struct::type_c<...>) of the registered members
template <typename S, typename V>
constexpr auto visit_types(V &&v) -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
    traits::visitable<traits::clean_t<S>>::visit_types(std::forward<V>(v));
}

// Visit the member pointers (&S::a) of the registered members
template <typename S, typename V>
constexpr auto visit_pointers(V &&v) -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
    traits::visitable<traits::clean_t<S>>::visit_pointers(std::forward<V>(v));
}

// Visit the accessors (function objects) of the registered members
template <typename S, typename V>
constexpr auto visit_accessors(V &&v) -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
    traits::visitable<traits::clean_t<S>>::visit_accessors(std::forward<V>(v));
}

// Apply visitor (with no instances)
// This calls visit_pointers, for backwards compat reasons
template <typename S, typename V>
constexpr auto apply_visitor(V &&v) -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value>::type {
    MetaEngine::Struct::visit_pointers<S>(std::forward<V>(v));
}

// Get value by index (like std::get for tuples)
template <int idx, typename S>
constexpr auto get(S &&s) -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value,
                                                     decltype(traits::visitable<traits::clean_t<S>>::get_value(std::integral_constant<int, idx>{}, std::forward<S>(s)))>::type {
    return traits::visitable<traits::clean_t<S>>::get_value(std::integral_constant<int, idx>{}, std::forward<S>(s));
}

// Get name of field, by index
template <int idx, typename S>
constexpr auto get_name() ->
        typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value, decltype(traits::visitable<traits::clean_t<S>>::get_name(std::integral_constant<int, idx>{}))>::type {
    return traits::visitable<traits::clean_t<S>>::get_name(std::integral_constant<int, idx>{});
}

template <int idx, typename S>
constexpr auto get_name(S &&) -> decltype(get_name<idx, S>()) {
    return get_name<idx, S>();
}

// Get member pointer, by index
template <int idx, typename S>
constexpr auto get_pointer() ->
        typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value, decltype(traits::visitable<traits::clean_t<S>>::get_pointer(std::integral_constant<int, idx>{}))>::type {
    return traits::visitable<traits::clean_t<S>>::get_pointer(std::integral_constant<int, idx>{});
}

template <int idx, typename S>
constexpr auto get_pointer(S &&) -> decltype(get_pointer<idx, S>()) {
    return get_pointer<idx, S>();
}

// Get member accessor, by index
template <int idx, typename S>
constexpr auto get_accessor() ->
        typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value, decltype(traits::visitable<traits::clean_t<S>>::get_accessor(std::integral_constant<int, idx>{}))>::type {
    return traits::visitable<traits::clean_t<S>>::get_accessor(std::integral_constant<int, idx>{});
}

template <int idx, typename S>
constexpr auto get_accessor(S &&) -> decltype(get_accessor<idx, S>()) {
    return get_accessor<idx, S>();
}

// Get type, by index
template <int idx, typename S>
struct type_at_s {
    using type_c = decltype(traits::visitable<traits::clean_t<S>>::type_at(std::integral_constant<int, idx>{}));
    using type = typename type_c::type;
};

template <int idx, typename S>
using type_at = typename type_at_s<idx, S>::type;

// Get name of structure
template <typename S>
constexpr auto get_name() -> typename std::enable_if<traits::is_visitable<traits::clean_t<S>>::value, decltype(traits::visitable<traits::clean_t<S>>::get_name())>::type {
    return traits::visitable<traits::clean_t<S>>::get_name();
}

template <typename S>
constexpr auto get_name(S &&) -> decltype(get_name<S>()) {
    return get_name<S>();
}

static constexpr const int max_visitable_members = 69;

#define METAENGINE_STRUCT_EXPAND(x) x
#define METAENGINE_STRUCT_PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66,  \
                                   _67, _68, _69, N, ...)                                                                                                                                           \
    N
#define METAENGINE_STRUCT_PP_NARG(...)                                                                                                                                                               \
    METAENGINE_STRUCT_EXPAND(METAENGINE_STRUCT_PP_ARG_N(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, \
                                                        37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

/* need extra level to force extra eval */
#define METAENGINE_STRUCT_CONCAT_(a, b) a##b
#define METAENGINE_STRUCT_CONCAT(a, b) METAENGINE_STRUCT_CONCAT_(a, b)

#define METAENGINE_STRUCT_APPLYF0(f)
#define METAENGINE_STRUCT_APPLYF1(f, _1) f(_1)
#define METAENGINE_STRUCT_APPLYF2(f, _1, _2) f(_1) f(_2)
#define METAENGINE_STRUCT_APPLYF3(f, _1, _2, _3) f(_1) f(_2) f(_3)
#define METAENGINE_STRUCT_APPLYF4(f, _1, _2, _3, _4) f(_1) f(_2) f(_3) f(_4)
#define METAENGINE_STRUCT_APPLYF5(f, _1, _2, _3, _4, _5) f(_1) f(_2) f(_3) f(_4) f(_5)
#define METAENGINE_STRUCT_APPLYF6(f, _1, _2, _3, _4, _5, _6) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6)
#define METAENGINE_STRUCT_APPLYF7(f, _1, _2, _3, _4, _5, _6, _7) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7)
#define METAENGINE_STRUCT_APPLYF8(f, _1, _2, _3, _4, _5, _6, _7, _8) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8)
#define METAENGINE_STRUCT_APPLYF9(f, _1, _2, _3, _4, _5, _6, _7, _8, _9) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9)
#define METAENGINE_STRUCT_APPLYF10(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10)
#define METAENGINE_STRUCT_APPLYF11(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)
#define METAENGINE_STRUCT_APPLYF12(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12)
#define METAENGINE_STRUCT_APPLYF13(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)
#define METAENGINE_STRUCT_APPLYF14(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14)
#define METAENGINE_STRUCT_APPLYF15(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15)
#define METAENGINE_STRUCT_APPLYF16(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16)
#define METAENGINE_STRUCT_APPLYF17(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17)
#define METAENGINE_STRUCT_APPLYF18(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18)
#define METAENGINE_STRUCT_APPLYF19(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19)
#define METAENGINE_STRUCT_APPLYF20(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20)
#define METAENGINE_STRUCT_APPLYF21(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21)
#define METAENGINE_STRUCT_APPLYF22(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22)
#define METAENGINE_STRUCT_APPLYF23(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23)
#define METAENGINE_STRUCT_APPLYF24(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)
#define METAENGINE_STRUCT_APPLYF25(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25)
#define METAENGINE_STRUCT_APPLYF26(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26)
#define METAENGINE_STRUCT_APPLYF27(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27)
#define METAENGINE_STRUCT_APPLYF28(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28)
#define METAENGINE_STRUCT_APPLYF29(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)
#define METAENGINE_STRUCT_APPLYF30(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)                    \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) \
            f(_30)
#define METAENGINE_STRUCT_APPLYF31(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)               \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) \
            f(_30) f(_31)
#define METAENGINE_STRUCT_APPLYF32(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)          \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) \
            f(_30) f(_31) f(_32)
#define METAENGINE_STRUCT_APPLYF33(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)     \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) \
            f(_30) f(_31) f(_32) f(_33)
#define METAENGINE_STRUCT_APPLYF34(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34)
#define METAENGINE_STRUCT_APPLYF35(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35)                                                                                                                                                                \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)
#define METAENGINE_STRUCT_APPLYF36(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36)                                                                                                                                                           \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36)
#define METAENGINE_STRUCT_APPLYF37(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37)                                                                                                                                                      \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37)
#define METAENGINE_STRUCT_APPLYF38(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38)                                                                                                                                                 \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38)
#define METAENGINE_STRUCT_APPLYF39(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39)                                                                                                                                            \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39)
#define METAENGINE_STRUCT_APPLYF40(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40)                                                                                                                                       \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40)
#define METAENGINE_STRUCT_APPLYF41(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41)                                                                                                                                  \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41)
#define METAENGINE_STRUCT_APPLYF42(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42)                                                                                                                             \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42)
#define METAENGINE_STRUCT_APPLYF43(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43)                                                                                                                        \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43)
#define METAENGINE_STRUCT_APPLYF44(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44)                                                                                                                   \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44)
#define METAENGINE_STRUCT_APPLYF45(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45)                                                                                                              \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)
#define METAENGINE_STRUCT_APPLYF46(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46)                                                                                                         \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46)
#define METAENGINE_STRUCT_APPLYF47(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47)                                                                                                    \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47)
#define METAENGINE_STRUCT_APPLYF48(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48)                                                                                               \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48)
#define METAENGINE_STRUCT_APPLYF49(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)                                                                                          \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49)
#define METAENGINE_STRUCT_APPLYF50(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50)                                                                                     \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50)
#define METAENGINE_STRUCT_APPLYF51(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51)                                                                                \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51)
#define METAENGINE_STRUCT_APPLYF52(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52)                                                                           \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52)
#define METAENGINE_STRUCT_APPLYF53(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53)                                                                      \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53)
#define METAENGINE_STRUCT_APPLYF54(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54)                                                                 \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54)
#define METAENGINE_STRUCT_APPLYF55(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55)                                                            \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)
#define METAENGINE_STRUCT_APPLYF56(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56)                                                       \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56)
#define METAENGINE_STRUCT_APPLYF57(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57)                                                  \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57)
#define METAENGINE_STRUCT_APPLYF58(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58)                                             \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58)
#define METAENGINE_STRUCT_APPLYF59(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59)                                        \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59)
#define METAENGINE_STRUCT_APPLYF60(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60)                                   \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60)
#define METAENGINE_STRUCT_APPLYF61(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61)                              \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61)
#define METAENGINE_STRUCT_APPLYF62(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62)                         \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62)
#define METAENGINE_STRUCT_APPLYF63(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63)                    \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63)
#define METAENGINE_STRUCT_APPLYF64(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64)               \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)
#define METAENGINE_STRUCT_APPLYF65(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65)          \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65)
#define METAENGINE_STRUCT_APPLYF66(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66)     \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66)
#define METAENGINE_STRUCT_APPLYF67(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66,     \
                                   _67)                                                                                                                                                                \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67)
#define METAENGINE_STRUCT_APPLYF68(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66,     \
                                   _67, _68)                                                                                                                                                           \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67) f(_68)
#define METAENGINE_STRUCT_APPLYF69(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, \
                                   _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66,     \
                                   _67, _68, _69)                                                                                                                                                      \
    f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)  \
            f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)      \
                    f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67) f(_68) f(_69)

#define METAENGINE_STRUCT_APPLY_F_(M, ...) METAENGINE_STRUCT_EXPAND(M(__VA_ARGS__))
#define METAENGINE_STRUCT_PP_MAP(f, ...) \
    METAENGINE_STRUCT_EXPAND(METAENGINE_STRUCT_APPLY_F_(METAENGINE_STRUCT_CONCAT(METAENGINE_STRUCT_APPLYF, METAENGINE_STRUCT_PP_NARG(__VA_ARGS__)), f, __VA_ARGS__))

/*** End generated code ***/

/***
 * These macros are used with METAENGINE_STRUCT_PP_MAP
 */

#define METAENGINE_STRUCT_FIELD_COUNT(MEMBER_NAME) +1

#define METAENGINE_STRUCT_MEMBER_HELPER(MEMBER_NAME) std::forward<V>(visitor)(#MEMBER_NAME, std::forward<S>(struct_instance).MEMBER_NAME);

#define METAENGINE_STRUCT_MEMBER_HELPER_PTR(MEMBER_NAME) std::forward<V>(visitor)(#MEMBER_NAME, &this_type::MEMBER_NAME);

#define METAENGINE_STRUCT_MEMBER_HELPER_TYPE(MEMBER_NAME) std::forward<V>(visitor)(#MEMBER_NAME, MetaEngine::Struct::type_c<decltype(this_type::MEMBER_NAME)>{});

#define METAENGINE_STRUCT_MEMBER_HELPER_ACC(MEMBER_NAME) std::forward<V>(visitor)(#MEMBER_NAME, MetaEngine::Struct::accessor<decltype(&this_type::MEMBER_NAME), &this_type::MEMBER_NAME>{});

#define METAENGINE_STRUCT_MEMBER_HELPER_PAIR(MEMBER_NAME) std::forward<V>(visitor)(#MEMBER_NAME, std::forward<S1>(s1).MEMBER_NAME, std::forward<S2>(s2).MEMBER_NAME);

#define METAENGINE_STRUCT_MAKE_GETTERS(MEMBER_NAME)                                                                                                                                                    \
    template <typename S>                                                                                                                                                                              \
    static constexpr auto get_value(std::integral_constant<int, fields_enum::MEMBER_NAME>, S &&s)->decltype((std::forward<S>(s).MEMBER_NAME)) {                                                        \
        return std::forward<S>(s).MEMBER_NAME;                                                                                                                                                         \
    }                                                                                                                                                                                                  \
                                                                                                                                                                                                       \
    static constexpr auto get_name(std::integral_constant<int, fields_enum::MEMBER_NAME>)->decltype(#MEMBER_NAME) { return #MEMBER_NAME; }                                                             \
                                                                                                                                                                                                       \
    static constexpr auto get_pointer(std::integral_constant<int, fields_enum::MEMBER_NAME>)->decltype(&this_type::MEMBER_NAME) { return &this_type::MEMBER_NAME; }                                    \
                                                                                                                                                                                                       \
    static constexpr auto get_accessor(std::integral_constant<int, fields_enum::MEMBER_NAME>)->MetaEngine::Struct::accessor<decltype(&this_type::MEMBER_NAME), &this_type::MEMBER_NAME> { return {}; } \
                                                                                                                                                                                                       \
    static auto type_at(std::integral_constant<int, fields_enum::MEMBER_NAME>)->MetaEngine::Struct::type_c<decltype(this_type::MEMBER_NAME)>;

// This macro specializes the trait, provides "apply" method which does the work.
// Below, template parameter S should always be the same as STRUCT_NAME modulo const and reference.
// The interface defined above ensures that STRUCT_NAME is clean_t<S> basically.
//
// Note: The code to make the indexed getters work is more convoluted than I'd like.
//       PP_MAP doesn't give you the index of each member. And rather than hack it so that it will
//       do that, what we do instead is:
//       1: Declare an enum `field_enum` in the scope of visitable, which maps names to indices.
//          This gives an easy way for the macro to get the index from the name token.
//       2: Intuitively we'd like to use template partial specialization to make indices map to
//          values, and have a new specialization for each member. But, specializations can only
//          be made at namespace scope. So to keep things tidy and contained within this trait,
//          we use tag dispatch with std::integral_constant<int> instead.

#define METADOT_STRUCT(STRUCT_NAME, ...)                                                                                       \
    namespace MetaEngine::Struct {                                                                                               \
    namespace traits {                                                                                                           \
                                                                                                                                 \
    template <>                                                                                                                  \
    struct visitable<STRUCT_NAME, void> {                                                                                        \
                                                                                                                                 \
        using this_type = STRUCT_NAME;                                                                                           \
                                                                                                                                 \
        static constexpr auto get_name() -> decltype(#STRUCT_NAME) { return #STRUCT_NAME; }                                      \
                                                                                                                                 \
        static constexpr const std::size_t field_count = 0 METAENGINE_STRUCT_PP_MAP(METAENGINE_STRUCT_FIELD_COUNT, __VA_ARGS__); \
                                                                                                                                 \
        template <typename V, typename S>                                                                                        \
        constexpr static void apply(V &&visitor, S &&struct_instance) {                                                          \
            METAENGINE_STRUCT_PP_MAP(METAENGINE_STRUCT_MEMBER_HELPER, __VA_ARGS__)                                               \
        }                                                                                                                        \
                                                                                                                                 \
        template <typename V, typename S1, typename S2>                                                                          \
        constexpr static void apply(V &&visitor, S1 &&s1, S2 &&s2) {                                                             \
            METAENGINE_STRUCT_PP_MAP(METAENGINE_STRUCT_MEMBER_HELPER_PAIR, __VA_ARGS__)                                          \
        }                                                                                                                        \
                                                                                                                                 \
        template <typename V>                                                                                                    \
        constexpr static void visit_pointers(V &&visitor) {                                                                      \
            METAENGINE_STRUCT_PP_MAP(METAENGINE_STRUCT_MEMBER_HELPER_PTR, __VA_ARGS__)                                           \
        }                                                                                                                        \
                                                                                                                                 \
        template <typename V>                                                                                                    \
        constexpr static void visit_types(V &&visitor) {                                                                         \
            METAENGINE_STRUCT_PP_MAP(METAENGINE_STRUCT_MEMBER_HELPER_TYPE, __VA_ARGS__)                                          \
        }                                                                                                                        \
                                                                                                                                 \
        template <typename V>                                                                                                    \
        constexpr static void visit_accessors(V &&visitor) {                                                                     \
            METAENGINE_STRUCT_PP_MAP(METAENGINE_STRUCT_MEMBER_HELPER_ACC, __VA_ARGS__)                                           \
        }                                                                                                                        \
                                                                                                                                 \
        struct fields_enum {                                                                                                     \
            enum index { __VA_ARGS__ };                                                                                          \
        };                                                                                                                       \
                                                                                                                                 \
        METAENGINE_STRUCT_PP_MAP(METAENGINE_STRUCT_MAKE_GETTERS, __VA_ARGS__)                                                    \
                                                                                                                                 \
        static constexpr const bool value = true;                                                                                \
    };                                                                                                                           \
    }                                                                                                                            \
    }                                                                                                                            \
    static_assert(true, "")

namespace detail {

/***
 * Poor man's mpl vector
 */

template <class... Ts>
struct TypeList {
    static constexpr const unsigned int size = sizeof...(Ts);
};

// Append metafunction
template <class List, class T>
struct Append;

template <class... Ts, class T>
struct Append<TypeList<Ts...>, T> {
    typedef TypeList<Ts..., T> type;
};

template <class L, class T>
using Append_t = typename Append<L, T>::type;

// Cdr metafunction (cdr is a lisp function which returns the tail of a list)
template <class List>
struct Cdr;

template <typename T, typename... Ts>
struct Cdr<TypeList<T, Ts...>> {
    typedef TypeList<Ts...> type;
};

template <class List>
using Cdr_t = typename Cdr<List>::type;

// Find metafunction (get the idx'th element)
template <class List, unsigned idx>
struct Find : Find<Cdr_t<List>, idx - 1> {};

template <typename T, typename... Ts>
struct Find<TypeList<T, Ts...>, 0> {
    typedef T type;
};

template <class List, unsigned idx>
using Find_t = typename Find<List, idx>::type;

// Alias used when capturing references to string literals

template <int N>
using char_array = const char[N];

/***
 * The "rank" template is a trick which can be used for
 * certain metaprogramming techniques. It creates
 * an inheritance hierarchy of trivial classes.
 */

template <int N>
struct Rank : Rank<N - 1> {};

template <>
struct Rank<0> {};

static constexpr const int max_visitable_members_intrusive = 100;

/***
 * To create a "compile-time" TypeList whose members are accumulated one-by-one,
 * the basic idea is to define a function, which takes a `Rank` object, and
 * whose return type is the type representing the current value of the list.
 *
 * That function is not a template function -- it is defined as taking a
 * particular rank object. Initially, it is defined only for `Rank<0>`.
 *
 * To add an element to the list, we define an overload of the function, which
 * takes the next higher `Rank` as it's argument. It's return value is,
 * the new value of the list, formed by using `Append_t` with the old value.
 *
 * To obtain the current value of the list, we use decltype with the name of the
 * function, and `Rank<100>`, or some suitably large integer. The C++ standard
 * specifies that overload resolution is in this case unambiguous and must
 * select the overload for the "most-derived" type which matches.
 *
 * The upshot is that `decltype(my_function(Rank<100>{}))` is a single well-formed
 * expression, which, because of C++ overload resolution rules, can be a
 * "mutable" value from the point of view of metaprogramming.
 *
 *
 * Attribution:
 * I first learned this trick from a stackoverflow post by Roman Perepelitsa:
 * http://stackoverflow.com/questions/4790721/c-type-registration-at-compile-time-trick
 *
 * He attributes it to a talk from Matt Calabrese at BoostCon 2011.
 *
 *
 * The expression is inherently dangerous if you are using it inside the body
 * of a struct -- obviously, it has different values at different points of the
 * structure definition. The "END_VISITABLES" macro is important in that this
 * finalizes the list, typedeffing `decltype(my_function(Rank<100>{}))` to some
 * fixed name in your struct at a specific point in the definition. That
 * typedef can only ultimately have one meaning, no matter where else the name
 * may be used (even implicitly) in your structure definition. That typedef is
 * what the trait defined in this header ultimately hooks into to find the
 * visitable members.
 */

// A tag inserted into a structure to mark it as visitable

struct intrusive_tag {};

/***
 * Helper structures which perform pack expansion in order to visit a structure.
 */

// In MSVC 2015, sometimes a pointer to member cannot be constexpr, for instance
// I had trouble with code like this:
//
// struct S {
//   int a;
//   static constexpr auto a_ptr = &S::a;
// };
//
// This works fine in gcc and clang.
// MSVC is okay with it if instead it is a template parameter it seems, so we
// use `member_ptr_helper` below as a workaround, a bit like so:
//
// struct S {
//   int a;
//   using a_helper = member_ptr_helper<S, int, &S::a>;
// };

template <typename S, typename T, T S::*member_ptr>
struct member_ptr_helper {
    static constexpr T S::*get_ptr() { return member_ptr; }
    using value_type = T;

    using accessor_t = MetaEngine::Struct::accessor<T S::*, member_ptr>;
};

// M should be derived from a member_ptr_helper
template <typename M>
struct member_helper {
    template <typename V, typename S>
    constexpr static void apply_visitor(V &&visitor, S &&structure_instance) {
        std::forward<V>(visitor)(M::member_name(), std::forward<S>(structure_instance).*M::get_ptr());
    }

    template <typename V, typename S1, typename S2>
    constexpr static void apply_visitor(V &&visitor, S1 &&s1, S2 &&s2) {
        std::forward<V>(visitor)(M::member_name(), std::forward<S1>(s1).*M::get_ptr(), std::forward<S2>(s2).*M::get_ptr());
    }

    template <typename V>
    constexpr static void visit_pointers(V &&visitor) {
        std::forward<V>(visitor)(M::member_name(), M::get_ptr());
    }

    template <typename V>
    constexpr static void visit_accessors(V &&visitor) {
        std::forward<V>(visitor)(M::member_name(), typename M::accessor_t());
    }

    template <typename V>
    constexpr static void visit_types(V &&visitor) {
        std::forward<V>(visitor)(M::member_name(), MetaEngine::Struct::type_c<typename M::value_type>{});
    }
};

template <typename Mlist>
struct structure_helper;

template <typename... Ms>
struct structure_helper<TypeList<Ms...>> {
    template <typename V, typename S>
    constexpr static void apply_visitor(V &&visitor, S &&structure_instance) {
        // Use parameter pack expansion to force evaluation of the member helper for each member in the list.
        // Inside parens, a comma operator is being used to discard the void value and produce an integer, while
        // not being an unevaluated context. The order of evaluation here is enforced by the compiler.
        // Extra zero at the end is to avoid UB for having a zero-size array.
        int dummy[] = {(member_helper<Ms>::apply_visitor(std::forward<V>(visitor), std::forward<S>(structure_instance)), 0)..., 0};
        // Suppress unused warnings, even in case of empty parameter pack
        static_cast<void>(dummy);
        static_cast<void>(visitor);
        static_cast<void>(structure_instance);
    }

    template <typename V, typename S1, typename S2>
    constexpr static void apply_visitor(V &&visitor, S1 &&s1, S2 &&s2) {
        int dummy[] = {(member_helper<Ms>::apply_visitor(std::forward<V>(visitor), std::forward<S1>(s1), std::forward<S2>(s2)), 0)..., 0};
        static_cast<void>(dummy);
        static_cast<void>(visitor);
        static_cast<void>(s1);
        static_cast<void>(s2);
    }

    template <typename V>
    constexpr static void visit_pointers(V &&visitor) {
        int dummy[] = {(member_helper<Ms>::visit_pointers(std::forward<V>(visitor)), 0)..., 0};
        static_cast<void>(dummy);
        static_cast<void>(visitor);
    }

    template <typename V>
    constexpr static void visit_accessors(V &&visitor) {
        int dummy[] = {(member_helper<Ms>::visit_accessors(std::forward<V>(visitor)), 0)..., 0};
        static_cast<void>(dummy);
        static_cast<void>(visitor);
    }

    template <typename V>
    constexpr static void visit_types(V &&visitor) {
        int dummy[] = {(member_helper<Ms>::visit_types(std::forward<V>(visitor)), 0)..., 0};
        static_cast<void>(dummy);
        static_cast<void>(visitor);
    }
};

}  // end namespace detail

/***
 * Implement trait
 */

namespace traits {

template <typename T>
struct visitable<T, typename std::enable_if<std::is_same<typename T::MetaEngine_Struct_Visitable_Structure_Tag__, ::MetaEngine::Struct::detail::intrusive_tag>::value>::type> {
    static constexpr const std::size_t field_count = T::MetaEngine_Struct_Registered_Members_List__::size;

    // Apply to an instance
    // S should be the same type as T modulo const and reference
    template <typename V, typename S>
    static constexpr void apply(V &&v, S &&s) {
        detail::structure_helper<typename T::MetaEngine_Struct_Registered_Members_List__>::apply_visitor(std::forward<V>(v), std::forward<S>(s));
    }

    // Apply with two instances
    template <typename V, typename S1, typename S2>
    static constexpr void apply(V &&v, S1 &&s1, S2 &&s2) {
        detail::structure_helper<typename T::MetaEngine_Struct_Registered_Members_List__>::apply_visitor(std::forward<V>(v), std::forward<S1>(s1), std::forward<S2>(s2));
    }

    // Apply with no instance
    template <typename V>
    static constexpr void visit_pointers(V &&v) {
        detail::structure_helper<typename T::MetaEngine_Struct_Registered_Members_List__>::visit_pointers(std::forward<V>(v));
    }

    template <typename V>
    static constexpr void visit_types(V &&v) {
        detail::structure_helper<typename T::MetaEngine_Struct_Registered_Members_List__>::visit_types(std::forward<V>(v));
    }

    template <typename V>
    static constexpr void visit_accessors(V &&v) {
        detail::structure_helper<typename T::MetaEngine_Struct_Registered_Members_List__>::visit_accessors(std::forward<V>(v));
    }

    // Get pointer
    template <int idx>
    static constexpr auto get_pointer(std::integral_constant<int, idx>) -> decltype(detail::Find_t<typename T::MetaEngine_Struct_Registered_Members_List__, idx>::get_ptr()) {
        return detail::Find_t<typename T::MetaEngine_Struct_Registered_Members_List__, idx>::get_ptr();
    }

    // Get accessor
    template <int idx>
    static constexpr auto get_accessor(std::integral_constant<int, idx>) -> typename detail::Find_t<typename T::MetaEngine_Struct_Registered_Members_List__, idx>::accessor_t {
        return {};
    }

    // Get value
    template <int idx, typename S>
    static constexpr auto get_value(std::integral_constant<int, idx> tag, S &&s) -> decltype(std::forward<S>(s).*get_pointer(tag)) {
        return std::forward<S>(s).*get_pointer(tag);
    }

    // Get name
    template <int idx>
    static constexpr auto get_name(std::integral_constant<int, idx>) -> decltype(detail::Find_t<typename T::MetaEngine_Struct_Registered_Members_List__, idx>::member_name()) {
        return detail::Find_t<typename T::MetaEngine_Struct_Registered_Members_List__, idx>::member_name();
    }

    // Get type
    template <int idx>
    static auto type_at(std::integral_constant<int, idx>) -> MetaEngine::Struct::type_c<typename detail::Find_t<typename T::MetaEngine_Struct_Registered_Members_List__, idx>::value_type>;

    // Get name of structure
    static constexpr decltype(T::MetaEngine_Struct_Get_Name__()) get_name() { return T::MetaEngine_Struct_Get_Name__(); }

    static constexpr const bool value = true;
};

}  // namespace traits

}  // namespace MetaEngine::Struct

#define METAENGINE_STRUCT_GET_REGISTERED_MEMBERS decltype(MetaEngine_Struct_Get_Visitables__(::MetaEngine::Struct::detail::Rank<MetaEngine::Struct::detail::max_visitable_members_intrusive>{}))

#define METAENGINE_STRUCT_MAKE_MEMBER_NAME(NAME) MetaEngine_Struct_Member_Record__##NAME

#define BEGIN_VISITABLES(NAME)                                                                                                   \
    typedef NAME METAENGINE_STRUCT_CURRENT_TYPE;                                                                                 \
    static constexpr decltype(#NAME) MetaEngine_Struct_Get_Name__() { return #NAME; }                                                 \
    ::MetaEngine::Struct::detail::TypeList<> static inline MetaEngine_Struct_Get_Visitables__(::MetaEngine::Struct::detail::Rank<0>); \
    static_assert(true, "")

#define VISITABLE(TYPE, NAME)                                                                                                                                                      \
    TYPE NAME;                                                                                                                                                                     \
    struct METAENGINE_STRUCT_MAKE_MEMBER_NAME(NAME) : MetaEngine::Struct::detail::member_ptr_helper<METAENGINE_STRUCT_CURRENT_TYPE, TYPE, &METAENGINE_STRUCT_CURRENT_TYPE::NAME> { \
        static constexpr const ::MetaEngine::Struct::detail::char_array<sizeof(#NAME)> &member_name() { return #NAME; }                                                            \
    };                                                                                                                                                                             \
    static inline ::MetaEngine::Struct::detail::Append_t<METAENGINE_STRUCT_GET_REGISTERED_MEMBERS, METAENGINE_STRUCT_MAKE_MEMBER_NAME(NAME)> MetaEngine_Struct_Get_Visitables__(        \
            ::MetaEngine::Struct::detail::Rank<METAENGINE_STRUCT_GET_REGISTERED_MEMBERS::size + 1>);                                                                               \
    static_assert(true, "")

#define END_VISITABLES                                                                          \
    typedef METAENGINE_STRUCT_GET_REGISTERED_MEMBERS MetaEngine_Struct_Registered_Members_List__;    \
    typedef ::MetaEngine::Struct::detail::intrusive_tag MetaEngine_Struct_Visitable_Structure_Tag__; \
    static_assert(true, "")

#endif
