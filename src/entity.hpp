#pragma once

#include "components.hpp"

#include <memory>

enum class EntityType {
    Player,
    Enemy,
    Bullet,
    SmallEnemy,
};

class Entity
{
    size_t m_id;
    EntityType m_type;
    bool m_active = true;

    friend class EntityManager;
    Entity(size_t id, EntityType type);

public:
    std::unique_ptr<CCollision> collision;
    std::unique_ptr<CLifespan> lifespan;
    std::unique_ptr<CInput> input;
    std::unique_ptr<CScore> score;
    std::unique_ptr<CShape> shape;
    std::unique_ptr<CTransform> transform;

    Entity() = delete;
    [[nodiscard]] size_t id() const;
    [[nodiscard]] EntityType type() const;
    [[nodiscard]] bool active() const;
    void destroy();
};
