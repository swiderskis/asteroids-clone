#include "game.hpp"

#include "Color.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "Text.hpp"
#include "Window.hpp"

#include <fstream>
#include <memory>
#include <random>
#include <string>

constexpr float SIN_45 = 0.70710678119;
constexpr float MAX_ANGLE = 2 * PI;
constexpr int MAX_RGB_VALUE = 255;

constexpr int PAUSE_WIDTH = 10;
constexpr int PAUSE_HEIGHT = 35;
constexpr int PAUSE_X_OFFSET = 30;
constexpr int PAUSE_Y_OFFSET = 20;
constexpr int PAUSE_BAR_SPLIT = 20;

constexpr int SCORE_X = 10;
constexpr int SCORE_Y = 10;
constexpr int SCORE_FONT_SIZE = 32;
constexpr int SCORE_MULTIPLIER = 100;

constexpr int SPECIAL_LIFESPAN = 150;
constexpr int SPECIAL_RADIUS = 50;
constexpr int SPECIAL_SIDES = 1;
constexpr int SPECIAL_FILL_R = 13;
constexpr int SPECIAL_FILL_G = 3;
constexpr int SPECIAL_FILL_B = 16;
constexpr int SPECIAL_OUTLINE_THICKNESS = 20;
constexpr int SPECIAL_SPEED = 10;
constexpr float SPECIAL_SPEED_MULTIPLIER = 0.97;
constexpr float SPECIAL_STRENGTH = 10000.0;
constexpr int SPECIAL_COOLDOWN = 300;

void Game::user_input()
{
    if (m_entity_manager.entities(EntityType::Player).empty()) {
        return;
    }

    m_player->input->up = RKeyboard::IsKeyDown(KEY_W);
    m_player->input->down = RKeyboard::IsKeyDown(KEY_S);
    m_player->input->left = RKeyboard::IsKeyDown(KEY_A);
    m_player->input->right = RKeyboard::IsKeyDown(KEY_D);

    if (RMouse::IsButtonPressed(MOUSE_LEFT_BUTTON)) {
        this->spawn_bullet();
    }

    if (RMouse::IsButtonPressed(MOUSE_RIGHT_BUTTON) && m_frame - m_special_use_frame > SPECIAL_COOLDOWN) {
        m_special_use_frame = m_frame;
        this->spawn_special();
    }

    if (RKeyboard::IsKeyPressed(KEY_SPACE)) {
        m_paused = !m_paused;
    }
}

void Game::movement()
{
    if (m_entity_manager.entities(EntityType::Player).empty()) {
        return;
    }

    bool vertical_movement = m_player->input->up ^ m_player->input->down;
    bool horizontal_movement = m_player->input->left ^ m_player->input->right;

    for (const auto& entity : m_entity_manager.entities()) {
        RVector2 position = entity->transform->position;
        RVector2 speed;
        float radius = entity->collision->radius;

        switch (entity->type()) {
        case EntityType::Player:
            speed.x = vertical_movement ? m_player_config.speed * SIN_45 : m_player_config.speed;
            speed.y = horizontal_movement ? m_player_config.speed * SIN_45 : m_player_config.speed;

            if (horizontal_movement && entity->input->left && position.x - radius - speed.x > 0) {
                entity->transform->velocity.x = -speed.x;
            } else if (horizontal_movement && entity->input->right
                       && position.x + radius + speed.x < (float)m_window.GetWidth()) {
                entity->transform->velocity.x = speed.x;
            } else {
                entity->transform->velocity.x = 0.0;
            }

            if (vertical_movement && entity->input->up && position.y - radius - speed.y > 0) {
                entity->transform->velocity.y = -speed.y;
            } else if (vertical_movement && entity->input->down
                       && position.y + radius + speed.y < (float)m_window.GetHeight()) {
                entity->transform->velocity.y = speed.y;

            } else {
                entity->transform->velocity.y = 0.0;
            }

            break;
        case EntityType::Enemy:
        case EntityType::SmallEnemy:
            if (position.x + entity->transform->velocity.x - radius < 0
                || position.x + entity->transform->velocity.x + radius > (float)m_window.GetWidth()) {
                entity->transform->velocity.x *= -1;
            }
            if (position.y + entity->transform->velocity.y - radius < 0
                || position.y + entity->transform->velocity.y + radius > (float)m_window.GetHeight()) {
                entity->transform->velocity.y *= -1;
            }
            if (!m_entity_manager.entities(EntityType::Special).empty()) {
                for (const auto& special : m_entity_manager.entities(EntityType::Special)) {
                    RVector2 distance = special->transform->position - entity->transform->position;
                    float angle = atan2(distance.y, distance.x);
                    entity->transform->velocity += RVector2(cos(angle), sin(angle)) * SPECIAL_STRENGTH
                                                   / (distance.Length() * distance.Length());
                }
            }
            break;
        case EntityType::Bullet:
            break;
        case EntityType::Special:
            entity->transform->velocity *= SPECIAL_SPEED_MULTIPLIER;
            break;
        }

        entity->transform->position += entity->transform->velocity;
    }
}

void Game::lifespan()
{
    for (const auto& entity : m_entity_manager.entities()) {
        if (entity->lifespan == nullptr) {
            continue;
        }

        entity->lifespan->current--;
        if (entity->lifespan->current == 0) {
            entity->destroy();
        }
    }
}

void Game::collision(EntityType enemy_type)
{
    for (const auto& enemy : m_entity_manager.entities(enemy_type)) {
        float distance_x = enemy->transform->position.x - m_player->transform->position.x;
        float distance_y = enemy->transform->position.y - m_player->transform->position.y;
        float distance = enemy->collision->radius + m_player->collision->radius;
        if ((distance_x * distance_x) + (distance_y * distance_y) < distance * distance) {
            m_player->destroy();
            enemy->destroy();
        }

        for (const auto& bullet : m_entity_manager.entities(EntityType::Bullet)) {
            if (!bullet->active()) {
                continue;
            }

            distance_x = enemy->transform->position.x - bullet->transform->position.x;
            distance_y = enemy->transform->position.y - bullet->transform->position.y;
            distance = enemy->collision->radius + bullet->collision->radius;
            if ((distance_x * distance_x) + (distance_y * distance_y) < distance * distance) {
                bullet->destroy();
                enemy->destroy();
                m_score += enemy->score->score;

                if (enemy_type == EntityType::Enemy) {
                    this->spawn_small_enemies(enemy);
                }
            }
        }

        for (const auto& special : m_entity_manager.entities(EntityType::Special)) {
            distance_x = enemy->transform->position.x - special->transform->position.x;
            distance_y = enemy->transform->position.y - special->transform->position.y;
            distance = enemy->collision->radius + special->collision->radius;
            if ((distance_x * distance_x) + (distance_y * distance_y) < distance * distance) {
                enemy->destroy();
                m_score += enemy->score->score;

                if (enemy_type == EntityType::Enemy) {
                    this->spawn_small_enemies(enemy);
                }
            }
        }
    }
}

void Game::player_spawner()
{
    if (m_entity_manager.entities(EntityType::Player).empty()) {
        this->spawn_player();
    }
}

void Game::enemy_spawner()
{
    if (m_frame - m_enemy_spawn_frame > m_enemy_config.spawn_interval) {
        this->spawn_enemy();
        m_enemy_spawn_frame = m_frame;
    }
}

void Game::spawn_player()
{
    m_player = m_entity_manager.add_entity(EntityType::Player);
    m_player->collision = std::make_unique<CCollision>(m_player_config.collision_radius);
    m_player->input = std::make_unique<CInput>();
    RColor fill(m_player_config.fill_r, m_player_config.fill_g, m_player_config.fill_b);
    RColor outline(m_player_config.outline_r, m_player_config.outline_g, m_player_config.outline_b);
    m_player->shape = std::make_unique<CShape>(
        m_player_config.shape_radius, m_player_config.sides, fill, outline, m_player_config.outline_thickness);
    RVector2 window_centre((float)m_window.GetWidth() / 2, (float)m_window.GetHeight() / 2);
    m_player->transform = std::make_unique<CTransform>(window_centre, RVector2(), 0.0);
}

void Game::spawn_enemy()
{
    auto enemy = m_entity_manager.add_entity(EntityType::Enemy);
    int collision_radius = m_enemy_config.collision_radius;
    enemy->collision = std::make_unique<CCollision>(collision_radius);

    std::random_device random_device;
    std::mt19937 gen(random_device());
    std::uniform_int_distribution<> random_rgb_value(0, MAX_RGB_VALUE);
    std::uniform_int_distribution<> random_sides(m_enemy_config.min_sides, m_enemy_config.max_sides);
    int sides = random_sides(gen);
    RColor fill(random_rgb_value(gen), random_rgb_value(gen), random_rgb_value(gen));
    RColor outline(m_enemy_config.outline_r, m_enemy_config.outline_g, m_enemy_config.outline_b);
    enemy->shape
        = std::make_unique<CShape>(m_enemy_config.shape_radius, sides, fill, outline, m_enemy_config.outline_thickness);
    enemy->score = std::make_unique<CScore>(sides * SCORE_MULTIPLIER);

    std::uniform_int_distribution<> random_position_x(0 + collision_radius, m_window.GetWidth() - collision_radius);
    std::uniform_int_distribution<> random_position_y(0 + collision_radius, m_window.GetHeight() - collision_radius);
    std::uniform_real_distribution<> random_speed(m_enemy_config.min_speed, m_enemy_config.max_speed);
    std::uniform_real_distribution<> random_angle(0, MAX_ANGLE);
    RVector2 spawn_position((float)random_position_x(gen), (float)random_position_y(gen));
    auto speed = random_speed(gen);
    auto angle = random_angle(gen);
    RVector2 velocity((float)(speed * cos(angle)), (float)(speed * sin(angle)));
    enemy->transform = std::make_unique<CTransform>(spawn_position, velocity, 0.0);
}

void Game::spawn_small_enemies(const std::shared_ptr<Entity>& enemy)
{
    std::random_device random_device;
    std::mt19937 gen(random_device());
    std::uniform_real_distribution<> random_speed(m_enemy_config.min_speed, m_enemy_config.max_speed);
    auto speed = random_speed(gen);
    float angle = MAX_ANGLE / (float)enemy->shape->sides;
    float current_angle = angle;

    for (int i = 0; i < enemy->shape->sides; i++) {
        auto small_enemy = m_entity_manager.add_entity(EntityType::SmallEnemy);
        small_enemy->collision = std::make_unique<CCollision>(enemy->collision->radius / 2);
        small_enemy->lifespan = std::make_unique<CLifespan>(m_enemy_config.small_enemy_lifespan);
        small_enemy->shape = std::make_unique<CShape>(enemy->shape->radius / 2,
                                                      enemy->shape->sides,
                                                      enemy->shape->fill,
                                                      enemy->shape->outline,
                                                      enemy->shape->outline_thickness);
        small_enemy->score = std::make_unique<CScore>(enemy->score->score);
        RVector2 velocity((float)speed * cos(current_angle), (float)speed * sin(current_angle));
        small_enemy->transform = std::make_unique<CTransform>(enemy->transform->position, velocity, 0.0);

        current_angle += angle;
    }
}

void Game::spawn_bullet()
{
    auto bullet = m_entity_manager.add_entity(EntityType::Bullet);
    bullet->collision = std::make_unique<CCollision>(m_bullet_config.collision_radius);
    bullet->lifespan = std::make_unique<CLifespan>(m_bullet_config.lifespan);

    RColor fill(m_bullet_config.fill_r, m_bullet_config.fill_g, m_bullet_config.fill_b);
    RColor outline(m_bullet_config.outline_r, m_bullet_config.outline_g, m_bullet_config.outline_b);
    bullet->shape = std::make_unique<CShape>(
        m_bullet_config.shape_radius, m_bullet_config.sides, fill, outline, m_bullet_config.outline_thickness);

    RVector2 player_position = m_player->transform->position;
    auto diff = RMouse::GetPosition() - player_position;
    float angle = atan2(diff.y, diff.x);
    RVector2 velocity(m_bullet_config.speed * cos(angle), m_bullet_config.speed * sin(angle));
    bullet->transform = std::make_unique<CTransform>(player_position, velocity, 0.0);
}

void Game::spawn_special()
{
    auto special = m_entity_manager.add_entity(EntityType::Special);
    special->collision = std::make_unique<CCollision>(SPECIAL_RADIUS);
    special->lifespan = std::make_unique<CLifespan>(SPECIAL_LIFESPAN);
    RColor fill(SPECIAL_FILL_R, SPECIAL_FILL_G, SPECIAL_FILL_B);
    special->shape = std::make_unique<CShape>(SPECIAL_RADIUS, SPECIAL_SIDES, fill, WHITE, SPECIAL_OUTLINE_THICKNESS);
    RVector2 player_position = m_player->transform->position;
    auto diff = RMouse::GetPosition() - player_position;
    float angle = atan2(diff.y, diff.x);
    RVector2 velocity(SPECIAL_SPEED * cos(angle), SPECIAL_SPEED * sin(angle));
    special->transform = std::make_unique<CTransform>(m_player->transform->position, velocity, 0.0);
}

void Game::render()
{
    for (const auto& entity : m_entity_manager.entities()) {
        int outline_thickness = 0;
        switch (entity->type()) {
        case EntityType::Player:
            outline_thickness = m_player_config.outline_thickness;
            break;
        case EntityType::Enemy:
        case EntityType::SmallEnemy:
            outline_thickness = m_enemy_config.outline_thickness;
            break;
        case EntityType::Bullet:
        case EntityType::Special:
            outline_thickness = m_bullet_config.outline_thickness;
            break;
        }

        RColor fill = entity->shape->fill;
        RColor outline = entity->shape->outline;
        fill.a = entity->lifespan == nullptr ? MAX_RGB_VALUE
                                             : MAX_RGB_VALUE * entity->lifespan->current / entity->lifespan->max;
        outline.a = entity->lifespan == nullptr ? MAX_RGB_VALUE
                                                : MAX_RGB_VALUE * entity->lifespan->current / entity->lifespan->max;
        int radius = entity->shape->radius;
        int sides = entity->shape->sides;

        if (!m_paused) {
            entity->transform->angle += 1.0;
        }

        if (sides == 1) {
            entity->transform->position.DrawCircle((float)radius, outline);
            entity->transform->position.DrawCircle((float)(radius - outline_thickness), fill);
        } else {
            entity->transform->position.DrawPoly(sides, (float)radius, entity->transform->angle, outline);
            entity->transform->position.DrawPoly(
                sides, (float)(radius - outline_thickness), entity->transform->angle, fill);
        }
    }

    if (m_paused) {
        int pause_x = m_window.GetWidth() - PAUSE_X_OFFSET;
        int pause_y = PAUSE_Y_OFFSET;
        DrawRectangle(pause_x, pause_y, PAUSE_WIDTH, PAUSE_HEIGHT, DARKGREEN);
        DrawRectangle(pause_x - PAUSE_BAR_SPLIT, pause_y, PAUSE_WIDTH, PAUSE_HEIGHT, DARKGREEN);
    }

    std::string score = "Score: " + std::to_string(m_score);
    RText::Draw(score, SCORE_X, SCORE_Y, SCORE_FONT_SIZE, WHITE);
}

void Game::toggle_paused()
{
    m_paused = !m_paused;
}

Game::Game() : m_special_use_frame(-SPECIAL_COOLDOWN)
{
    std::string config_line_title;
    int window_width = 0;
    int window_height = 0;
    int framerate = 0;
    bool fullscreen = false;

    std::ifstream file("config.txt");
    file >> config_line_title >> window_width >> window_height >> framerate >> fullscreen;

    m_window.Init(window_width, window_height, "Assignment 2");
    m_window.SetTargetFPS(framerate);
    m_window.SetExitKey(KEY_NULL);

    file >> config_line_title >> m_player_config.shape_radius >> m_player_config.collision_radius
        >> m_player_config.speed >> m_player_config.fill_r >> m_player_config.fill_g >> m_player_config.fill_b
        >> m_player_config.outline_r >> m_player_config.outline_g >> m_player_config.outline_b
        >> m_player_config.outline_thickness >> m_player_config.sides;

    file >> config_line_title >> m_enemy_config.shape_radius >> m_enemy_config.collision_radius
        >> m_enemy_config.min_speed >> m_enemy_config.max_speed >> m_enemy_config.outline_r >> m_enemy_config.outline_g
        >> m_enemy_config.outline_b >> m_enemy_config.outline_thickness >> m_enemy_config.min_sides
        >> m_enemy_config.max_sides >> m_enemy_config.small_enemy_lifespan >> m_enemy_config.spawn_interval;

    file >> config_line_title >> m_bullet_config.shape_radius >> m_bullet_config.collision_radius
        >> m_bullet_config.speed >> m_bullet_config.fill_r >> m_bullet_config.fill_g >> m_bullet_config.fill_b
        >> m_bullet_config.outline_r >> m_bullet_config.outline_g >> m_bullet_config.outline_b
        >> m_bullet_config.outline_thickness >> m_bullet_config.sides >> m_bullet_config.lifespan;
}

void Game::run()
{
    while (!m_window.ShouldClose()) {
        m_window.BeginDrawing();
        m_window.ClearBackground(BLACK);

        this->user_input();

        if (!m_paused) {
            this->movement();
            this->lifespan();
            this->collision(EntityType::Enemy);
            this->collision(EntityType::SmallEnemy);
            this->player_spawner();
            this->enemy_spawner();
            m_entity_manager.update();
            m_frame++;
        }

        this->render();

        m_window.EndDrawing();
    }
}
