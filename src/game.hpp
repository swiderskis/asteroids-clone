#pragma once

#include "entity_manager.hpp"
#include "Window.hpp"

struct PlayerConfig {
    int shape_radius = 0;
    int collision_radius = 0;
    float speed = 0.0;
    int fill_r = 0;
    int fill_g = 0;
    int fill_b = 0;
    int outline_r = 0;
    int outline_g = 0;
    int outline_b = 0;
    int outline_thickness = 0;
    int sides = 0;
};

struct EnemyConfig {
    int shape_radius = 0;
    int collision_radius = 0;
    float min_speed = 0.0;
    float max_speed = 0.0;
    int outline_r = 0;
    int outline_g = 0;
    int outline_b = 0;
    int outline_thickness = 0;
    int min_sides = 0;
    int max_sides = 0;
    int small_enemy_lifespan = 0;
    int spawn_interval = 0;
};

struct BulletConfig {
    int shape_radius = 0;
    int collision_radius = 0;
    float speed = 0.0;
    int fill_r = 0;
    int fill_g = 0;
    int fill_b = 0;
    int outline_r = 0;
    int outline_g = 0;
    int outline_b = 0;
    int outline_thickness = 0;
    int sides = 0;
    int lifespan = 0;
};

class Game
{
    RWindow m_window;
    int m_frame = 0;
    int m_enemy_spawn_frame = 0;
    int m_special_use_frame;
    PlayerConfig m_player_config;
    EnemyConfig m_enemy_config;
    BulletConfig m_bullet_config;
    EntityManager m_entity_manager;
    std::shared_ptr<Entity> m_player;
    int m_score = 0;
    bool m_paused = false;

    void user_input();
    void movement();
    void lifespan();
    void collision(EntityType enemy_type);
    void player_spawner();
    void enemy_spawner();
    void spawn_player();
    void spawn_enemy();
    void spawn_small_enemies(const std::shared_ptr<Entity>& enemy);
    void spawn_bullet();
    void spawn_special();
    void render();
    void toggle_paused();

public:
    Game();
    void run();
};
