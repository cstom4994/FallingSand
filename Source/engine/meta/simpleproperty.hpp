

#pragma once
#ifndef META_SIMPLEPROPERTY_HPP
#define META_SIMPLEPROPERTY_HPP

#include "engine/meta/property.hpp"

namespace Meta
{
/**
 * \brief Base class for all simple types of properties
 *
 * This class actually does nothing more than its base, it's just a way to separate
 * simple properties from other types.
 *
 * \sa ArrayProperty, EnumProperty, ObjectProperty
 */
class SimpleProperty : public Property
{
public:

    /**
     * \brief Construct the property from its description
     *
     * \param name Name of the property
     * \param type Type of the property
     */
    SimpleProperty(IdRef name, ValueKind type);

    /**
     * \brief Destructor
     */
    virtual ~SimpleProperty();

    /**
     * \brief Accept the visitation of a ClassVisitor
     *
     * \param visitor Visitor to accept
     */
    void accept(ClassVisitor& visitor) const override;
};

} // namespace Meta

#endif // META_SIMPLEPROPERTY_HPP
