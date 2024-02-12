#include "components.hpp"

CCollision::Collision(float radius) : radius(radius)
{
}

CLifespan::Lifespan(unsigned lifespan) : current(lifespan), max(lifespan)
{
}

CScore::Score(int score) : score(score)
{
}

CShape::Shape(int radius, int sides, RColor fill, RColor outline, int outline_thickness) :
    radius(radius), sides(sides), fill(fill), outline(outline), outline_thickness(outline_thickness)
{
}

CTransform::Transform(RVector2 position, RVector2 velocity, float angle) :
    position(position), velocity(velocity), angle(angle)
{
}
