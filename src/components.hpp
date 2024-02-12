#pragma once

#include "Color.hpp"
#include "Vector2.hpp"

namespace component
{
struct Collision {
    float radius;

    explicit Collision(float radius);
};

struct Input {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

struct Lifespan {
    unsigned current;
    unsigned max;

    explicit Lifespan(unsigned lifespan);
};

struct Score {
    int score;

    explicit Score(int score);
};

struct Shape {
    int radius;
    int sides;
    RColor fill;
    RColor outline;
    int outline_thickness;

    explicit Shape(int radius, int sides, RColor fill, RColor outline, int outline_thickness);
};

struct Transform {
    RVector2 position;
    RVector2 velocity;
    float angle;

    Transform(RVector2 position, RVector2 velocity, float angle);
};
} // namespace component

using CCollision = component::Collision;
using CInput = component::Input;
using CLifespan = component::Lifespan;
using CScore = component::Score;
using CShape = component::Shape;
using CTransform = component::Transform;
