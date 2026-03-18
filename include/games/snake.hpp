#pragma once

#include <deque>
#include <random>

#include "scenes.hpp"

enum class SnakeDir
{
    Up,
    Down,
    Left,
    Right
};

class SnakeGame : public Scene
{
public:
    Result<>    on_begin() override;
    void        render() override;
    SceneResult handle_input(uint32_t key) override;

    // Ticked frame loop, speed increases with score
    int frame_ms() override { return m_speed_ms; }

private:
    // board cell coordinate
    struct Point
    {
        int  x{}, y{};
        bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    };

    void init_game();
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

    bool m_dead   = false;
    bool m_paused = false;

    int m_score    = 0;
    int m_speed_ms = 130;  // ms per tick; decreases every 5 pts

    std::mt19937 m_rng{ std::random_device{}() };
};
