#pragma once

#include <array>
#include <cstdint>

#include "scenes.hpp"
#include "terminal_display.hpp"

static constexpr int GRID_SIZE = 4;
static constexpr int WIN_VALUE = 2048;

using Grid = std::array<std::array<int, GRID_SIZE>, GRID_SIZE>;

enum class Direction
{
    Left,
    Right,
    Up,
    Down
};

class Game2048 : public Scene
{
public:
    Game2048()
        : m_score(0),
          m_best_score(0),
          m_game_over(false),
          m_won(false),
          m_grid_x(0),
          m_grid_y(0),
          m_cell_w(0),
          m_cell_h(0),
          m_cell_padding(0) {};
    ~Game2048() override = default;

    void        render() override;
    SceneResult handle_input(uint32_t key) override;

protected:
    Result<> on_begin() override;

private:
    // Game state
    Grid m_grid;
    int  m_score;
    int  m_best_score;
    bool m_game_over;
    bool m_won;

    // Position and dimensions
    int m_grid_x;
    int m_grid_y;
    int m_cell_w;
    int m_cell_h;
    int m_cell_padding;

    // Helper functions
    void        init_game();
    void        add_new_tile();
    bool        move(Direction d);
    bool        is_move_possible() const;
    bool        check_win() const;
    uintattr_t  get_color_for_value(int value) const;
    std::string format_number(int value) const;

    // Drawing functions
    void draw_grid();
    void draw_cell(int row, int col, int value);
    void draw_hud();
    void draw_game_over();
    void draw_win();
    void draw_border();
};
