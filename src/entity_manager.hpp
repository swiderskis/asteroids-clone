#pragma once

#include "entity.hpp"

#include <map>
#include <vector>

using EntityVector = std::vector<std::shared_ptr<Entity>>;
using EntityMap = std::map<EntityType, std::vector<std::shared_ptr<Entity>>>;

class EntityManager
{
    size_t m_next_id = 0;
    EntityVector m_entities;
    EntityVector m_entities_to_add;
    EntityMap m_entities_map;

public:
    std::shared_ptr<Entity> add_entity(EntityType type);
    EntityVector& entities();
    EntityVector& entities(EntityType type);
    void update();
    void remove_inactive_entities();
};
