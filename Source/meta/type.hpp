

#ifndef META_TYPE_HPP
#define META_TYPE_HPP

#include <typeindex>

#include "meta/config.hpp"

/**
 * \namespace Meta
 * \brief Root namespace that encapsulates all of Meta.
 *
 * \namespace Meta::detail
 * \brief Meta hidden implementation details.
 *
 * \namespace Meta::policy
 * \brief Meta policy options.
 */

namespace Meta {

class Value;

// Note: This may have issues with DLLs?
typedef std::type_index TypeId;  // Used to uniquely identify a type.

/**
 * \brief Special empty type associated to \c noType
 *
 * This type is mainly used when writing custom \c Value visitors
 */
struct NoType {};

/**
 * \brief Enumeration of abstract value types supported by Meta Values
 *
 * \sa Value ValueMapper
 */
enum class ValueKind {
    None,       ///< No type has been defined yet
    Boolean,    ///< Boolean type (`bool`)
    Integer,    ///< Integer types (`unsigned`,`signed`, `char`, `short`, `int`, `long`)
    Real,       ///< Real types (`float`, `double`)
    String,     ///< String types (`char*`, `Meta::String`)
    Enum,       ///< Enumerated types
    Array,      ///< Array types (`T[]`, `std::vector`, `std::list`)
    Reference,  ///< Reference types (`T*`, `const T*`, `T&`, `const T&`)
    User        ///< User-defined classes
};

/**
 * \brief Enumeration of ways to reference an object
 *
 * \sa Value ValueMapper
 */
enum class ReferenceKind {
    None,          ///< not an object
    Instance,      ///< an object instance, e.g. `int`, `T`
    Pointer,       ///< pointer to an object, e.g. `T*`
    Reference,     ///< reference to an object, e.g. T&
    SmartPointer,  ///< smart pointer reference, e.g. `std::shared_ptr<T>`
    BuiltinArray,  ///< builtin array, e.g. `T[N]`
};

/**
 * \brief Enumeration of the kinds of function recognised
 *
 * \sa Function
 */
enum class FunctionKind {
    None,             ///< not a function
    Function,         ///< a function
    MemberFunction,   ///< function in a class or struct
    FunctionWrapper,  ///< `std::function<>`
    BindExpression,   ///< `std::bind()`
    Lambda            ///< lambda function `[](){}`
};

/**
 * \brief Enumeration of the kinds of property exposed
 *
 * \sa Property, Function
 */
enum class PropertyKind {
    Function,     ///< a function
    MemberObject  ///< member object in a class or struct
};

/**
 * \brief Enumeration of the kinds of Property accessors use
 *
 * \sa Property
 */
enum class PropertyAccessKind { Simple, Enum, Container, User };

/**
 * \brief Base class for all supported types.
 */
class Type {
public:
    virtual ~Type() {}
};

namespace policy {

/**
 * \brief Enumeration of the kinds of return policy
 */
enum class ReturnKind {
    NoReturn,  // void, returns nothing
    Copy,
    InternalRef,
    Multiple
};

/**
 * \brief Enumeration of the kinds of parameter type
 *
 * Parameters are the types in a function definition. Arguments are what you pass to them.
 */
enum class ParameterKind {
    PassByValue,      ///< Pass by value, e.g. `foo(T)`
    PassByReference,  ///< Pass by reference, e.g. `foo(const T*, const U&)`
    //    ReturnObject        ///< Return object, e.g. `foo(T**, T*&)`
};

/**
 * \brief Call return copy policy
 *
 * When added to a function declaration this sets the call return policy to copy. Any
 * values returned by a function are copied. This is the default behaviour.
 */
struct ReturnCopy {
    static constexpr ReturnKind kind = ReturnKind::Copy;  ///< The policy enum kind.
};

/**
 * \brief Call return internal reference policy
 *
 * When added to a function declaration this sets the call return policy to internal
 * reference. References returned by the function are returned as references values. This
 * is useful for declaring things like singletons, which return values that we do not
 * want to copy.
 */
struct ReturnInternalRef {
    static constexpr ReturnKind kind = ReturnKind::InternalRef;  ///< The policy enum kind.
};

/**
 * \brief Return multiple values
 *
 * When added to a function declaration this sets the call return policy to support multiple
 * value returning. The function should return a `std::tuple<...>` containing the multiple
 * values. Note that the tuple needs to be declared as a type.
 */
struct ReturnMultiple {
    static constexpr ReturnKind kind = ReturnKind::Multiple;  ///< The policy enum kind.
};

struct Parameter {};

// struct PassByValue
//{
//     static constexpr ParameterKind kind = ParameterKind::PassByValue; ///< The policy enum kind.
// };
//
// struct PassByReference
//{
//     static constexpr ParameterKind kind = ParameterKind::PassByReference; ///< The policy enum kind.
// };

}  // namespace policy

template <typename T, typename IT>
class ViewIterator {
private:
    IT m_iter;
    class Holder {
        const T m_value;

    public:
        Holder(IT value) : m_value(value) {}
        T operator*() { return m_value; }
    };

public:
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef std::input_iterator_tag iterator_category;

    explicit ViewIterator(IT value) : m_iter(value) {}
    value_type operator*() const { return *m_iter->second; }
    bool operator==(const ViewIterator& other) const { return m_iter == other.m_iter; }
    bool operator!=(const ViewIterator& other) const { return !(*this == other); }
    Holder operator++(int) {
        Holder ret(m_iter);
        ++m_iter;
        return ret;
    }
    ViewIterator& operator++() {
        ++m_iter;
        return *this;
    }
};

template <typename T, typename IT>
class View {
public:
    typedef ViewIterator<T, IT> Iterator;

    View(IT b, IT e) : m_begin(b), m_end(e) {}

    Iterator begin() { return Iterator(m_begin); }
    Iterator end() { return Iterator(m_end); }

private:
    IT m_begin, m_end;
};

}  // namespace Meta

#endif  // META_TYPE_HPP
