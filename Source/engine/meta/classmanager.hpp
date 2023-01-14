

#ifndef META_DETAIL_CLASSMANAGER_HPP
#define META_DETAIL_CLASSMANAGER_HPP

#include <map>

#include "engine/meta/type.hpp"
#include "observernotifier.hpp"

namespace Meta {

class Class;

namespace detail {

/**
 * \brief Manages creation, storage, retrieval and destruction of metaclasses
 *
 * Meta::ClassManager is the place where all metaclasses are stored and accessed.
 * It consists of a singleton which is created on first use and destroyed at global exit.
 *
 * \sa Class
 */
class ClassManager : public ObserverNotifier {
    // No need for shared pointers in here, we're the one and only instance holder
    typedef std::map<TypeId, Class*> ClassTable;
    typedef std::map<Id, Class*> NameTable;

public:
    typedef View<const Class&, ClassTable::const_iterator> ClassView;

    /**
     * \brief Get the unique instance of the class
     *
     * \return Reference to the unique instance of ClassManager
     */
    static ClassManager& instance();

    /**
     * \brief Create and register a new metaclass
     *
     * This is the entry point for every metaclass creation. This
     * function also notifies registered observers after successful creations.
     *
     * \param id Identifier of the C++ class bound to the metaclass (unique)
     *
     * \return Reference to the new metaclass
     *
     * \throw ClassAlreadyCreated \a name or \a id already exists
     */
    Class& addClass(TypeId const& id, IdRef name);

    /**
     * \brief Unregister an existing metaclass
     *
     * Use this to unregister a class that was declared with addClass(). This might be
     * useful if declaring classes temporarily from a dynamic module.
     *
     * \param id Identifier of the C++ class bound to the metaclass
     *
     * \throw ClassNotFound \a id not found
     */
    void removeClass(TypeId const& id);

    /**
     * \brief Get the total number of metaclasses
     *
     * \return Number of metaclasses that have been registered
     */
    size_t count() const;

    /**
     * \brief Get a metaclass from a C++ type
     *
     * \param id Identifier of the C++ type
     *
     * \return Reference to the requested metaclass
     *
     * \throw ClassNotFound id is not the name of an existing metaclass
     */
    const Class& getById(TypeId const& id) const;

    /**
     * \brief Get a metaclass from a C++ type
     *
     * This version returns a null pointer if no metaclass is found, instead
     * of throwing an exception.
     *
     * \param id Identifier of the C++ type
     *
     * \return Pointer to the requested metaclass, or null pointer if not found
     */
    const Class* getByIdSafe(TypeId const& id) const;

    /**
     * \brief Get a metaclass by name
     *
     * \param name Name of the metaclass
     *
     * \return Reference to the requested metaclass
     *
     * \throw ClassNotFound id is not the name of an existing metaclass
     */
    const Class& getByName(const IdRef name) const;

    /**
     * \brief Get a metaclass by name
     *
     * This version returns a null pointer if no metaclass is found, instead
     * of throwing an exception.
     *
     * \param name Name of the metaclass
     *
     * \return Pointer to the requested metaclass, or null pointer if not found
     */
    const Class* getByNameSafe(const IdRef name) const;

    /**
     * \brief Check if a given type has a metaclass
     *
     * \param id Identifier of the C++ type
     *
     * \return True if the class exists, false otherwise
     */
    bool classExists(TypeId const& id) const;

    /**
     * \brief Default constructor
     */
    ClassManager();

    /**
     * \brief Destructor
     *
     * The destructor destroys all the registered metaclasses and notifies the observers.
     */
    ~ClassManager();

    ClassView getClasses() const;

private:
    ClassTable m_classes;  // Table storing classes indexed by their ID
    NameTable m_names;     // Name look up of classes
};

}  // namespace detail
}  // namespace Meta

#endif  // META_DETAIL_CLASSMANAGER_HPP
