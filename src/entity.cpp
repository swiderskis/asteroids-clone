#include "entity.hpp"

Entity::Entity(size_t id, EntityType type) : m_id(id), m_type(type)
{
}

size_t Entity::id() const
{
    return m_id;
}

EntityType Entity::type() const
{
    return m_type;
}

bool Entity::active() const
{
    return m_active;
}

void Entity::destroy()
{
    m_active = false;
}
