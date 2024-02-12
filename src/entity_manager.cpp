#include "entity_manager.hpp"

#include <algorithm>
#include <memory>

std::shared_ptr<Entity> EntityManager::add_entity(EntityType type)
{
    auto entity = std::shared_ptr<Entity>(new Entity(m_next_id++, type));
    m_entities_to_add.push_back(entity);
    m_entities_map[type].push_back(entity);

    return entity;
}

EntityVector& EntityManager::entities()
{
    return m_entities;
}

EntityVector& EntityManager::entities(EntityType type)
{
    return m_entities_map[type];
}

void EntityManager::update()
{
    this->remove_inactive_entities();
    m_entities.insert(m_entities.end(), m_entities_to_add.begin(), m_entities_to_add.end());
    m_entities_to_add.clear();
}

void EntityManager::remove_inactive_entities()
{
    auto erase_begin =
        std::remove_if(m_entities.begin(), m_entities.end(), [](auto entity) { return !entity->m_active; });
    m_entities.erase(erase_begin, m_entities.end());

    for (auto& [type, entities] : m_entities_map) {
        erase_begin = std::remove_if(entities.begin(), entities.end(), [](auto entity) { return !entity->m_active; });
        entities.erase(erase_begin, entities.end());
    }
}
