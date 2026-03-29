#pragma once

#include <deque>
#include <random>

#include "game.hpp"

enum class SnakeDir
{
    Up,
    Down,
    Left,
    Right
};

class SnakeGame : public BaseGame
{
public:
    SceneResult handle_input(uint32_t key) override;

    // Ticked frame loop, speed increases with score
    int frame_ms() override { return m_speed_ms; }

protected:
    void        init_game() override;
    void        render_game() override;
    Result<>    on_game_begin() override;
    SceneResult scene_id() const override { return ScenesGame::Snake; }

private:
    // board cell coordinate
    struct Point
    {
        int  x{}, y{};
        bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    };

    void update();
    void spawn_food();
    void draw_border();
    void draw_hud();
    void draw_game_over();
    void draw_paused();

    // board extents (terminal cells)
    int m_board_x{};  // left edge  (inclusive, drawn as border)
    int m_board_y{};  // top  edge  (inclusive, drawn as border)
    int m_board_w{};  // total width  including border
    int m_board_h{};  // total height including border

    // game state
    std::deque<Point> m_snake;
    Point             m_food{};

    SnakeDir m_dir      = SnakeDir::Right;
    SnakeDir m_next_dir = SnakeDir::Right;

    int m_speed_ms = 130;  // ms per tick; decreases every 5 pts

    std::mt19937 m_rng{ std::random_device{}() };
};
