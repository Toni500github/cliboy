#pragma once

#include <chrono>
#include <vector>

#include "game.hpp"

class Minesweeper : public BaseGame
{
public:
    enum class Difficulty
    {
        Beginner,      // 9x9, 10 mines
        Intermediate,  // 16x16, 40 mines
        Expert,        // 30x16, 99 mines
        Count
    };

    SceneResult handleInput(uint32_t key) override;
    int         frameMs() override { return 100; };  // 10fps
    SceneResult sceneID() const override { return ScenesGame::Minesweeper; }

protected:
    void     initGame() override;
    Result<> onGameBegin() override;
    void     renderGame() override;

private:
    struct Cell
    {
        bool is_mine     = false;
        bool is_revealed = false;
        bool is_flagged  = false;
        int  adjacent    = 0;  // number of neighbouring (0-8)
    };

    // terminal columns per grid column
    static constexpr int k_cell_w = 2;

    // Config, fixed per difficulty
    Difficulty m_diff = Difficulty::Beginner;
    int        m_cols;
    int        m_rows;
    int        m_total_mines;

    int m_grid_x = 0;
    int m_grid_y = 0;

    // Mutable game state
    std::vector<Cell> m_grid;
    int               m_cursor_x       = 0;
    int               m_cursor_y       = 0;
    int               m_flags_placed   = 0;
    int               m_cells_revealed = 0;
    bool              m_first_click    = true;  // mine placement deferred to first reveal
    bool              m_choosing_diff  = true;

    std::chrono::steady_clock::time_point m_segment_start;
    std::chrono::seconds                  m_elapsed_before_pause{};
    bool                                  m_timer_running = false;

    // Grid helpers
    Cell&       at(int x, int y) { return m_grid[y * m_cols + x]; }
    const Cell& at(int x, int y) const { return m_grid[y * m_cols + x]; }
    bool        ok(int x, int y) { return x >= 0 && x < m_cols && y >= 0 && y < m_rows; }

    // Game logic
    void placeMine(int safe_x, int safe_y);  // first-click safe zone
    void computeAdjacency();
    void reveal(int x, int y);
    void chord(int x, int y);  // reveal around satisfied number
    void toggleFlag(int x, int y);
    bool checkWin() const;
    void triggerLoss();
    void handleReveal();  // Space/Enter handler

    int  drawHud();
    void drawDifficultMenu();
    void drawGrid();
    void drawCell(int gx, int gy);
};
