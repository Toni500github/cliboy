#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "game.hpp"
#include "terminal_display.hpp"

using TetrominoShape = std::array<std::array<uint8_t, 4>, 4>;

// Tetromino shapes (4x4 grids)
enum class TetrominoType
{
    I,
    O,
    T,
    S,
    Z,
    J,
    L
};

struct Tetromino
{
    TetrominoType  type;
    TetrominoShape shape;
    int            x, y;  // Position on grid (top-left corner)
};

class TetrisGame : public BaseGame
{
public:
    TetrisGame()
        : m_lines_cleared(0),
          m_level(0),
          m_fall_timer(0),
          m_last_update(0),
          m_grid_x(0),
          m_grid_y(0),
          m_cell_size(1),
          m_grid_w(0),
          m_grid_h(0) {};
    ~TetrisGame() override = default;

    SceneResult handle_input(uint32_t key) override;
    int         frame_ms() override { return 16; }  // ~60 FPS for smooth input

protected:
    void        init_game() override;
    void        render_game() override;
    Result<>    on_game_begin() override;
    SceneResult scene_id() const override { return ScenesGame::Tetris; }

private:
    // Game state
    std::vector<std::vector<uint32_t>> m_grid;  // Color values for each cell
    Tetromino                          m_current_piece;
    Tetromino                          m_next_piece;
    int                                m_lines_cleared;
    int                                m_level;
    uint32_t                           m_fall_timer;
    uint32_t                           m_last_update;

    // Position and dimensions
    int m_grid_x;
    int m_grid_y;
    int m_cell_size;  // Character cells per Tetris cell
    int m_grid_w;
    int m_grid_h;

    // Helper functions
    Tetromino      spawn_piece(TetrominoType type);
    Tetromino      get_random_piece();
    TetrominoShape get_shape_for_type(TetrominoType type);
    uintattr_t     get_color_for_type(TetrominoType type);
    bool           collides(const Tetromino& piece, int dx = 0, int dy = 0) const;
    void           merge_piece();
    void           clear_lines_and_update_score();
    int            calculate_score(int lines);
    void           update_level();
    int            get_fall_delay_ms() const;
    bool           move_piece(int dx, int dy);
    void           rotate_piece();
    void           hard_drop();
    void           spawn_new_piece();

    // Drawing functions
    void draw_grid();
    void draw_current_piece();
    void draw_next_piece();
    void draw_hud();
    void draw_border();
};
