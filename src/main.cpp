#include <raylib-cpp.hpp>

constexpr auto WINDOW_TITLE = "Assignment 2";
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int TARGET_FPS = 60;

int main()
{
    raylib::Color text_color(BLACK);
    raylib::Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        window.ClearBackground(DARKBLUE);
        text_color.DrawText("Congrats! You created your first window!", 190, 200, 20);
        window.EndDrawing();
    }

    return 0;
}
