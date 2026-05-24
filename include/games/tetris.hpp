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

    SceneResult handleInput(uint32_t key) override;
    int         frameMs() override { return 16; }  // ~60 FPS for smooth input

protected:
    void        initGame() override;
    void        renderGame() override;
    Result<>    onGameBegin() override;
    SceneResult sceneID() const override { return ScenesGame::Tetris; }

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
    Tetromino      spawnPiece(TetrominoType type);
    Tetromino      getRandomPiece();
    TetrominoShape getShapeForType(TetrominoType type);
    uintattr_t     getColorForType(TetrominoType type);
    bool           collides(const Tetromino& piece, int dx = 0, int dy = 0) const;
    void           mergePiece();
    void           clearLinesAndUpdateScore();
    int            calculateScore(int lines);
    void           updateLevel();
    int            getFallDelayMs() const;
    bool           movePiece(int dx, int dy);
    void           rotatePiece();
    void           hardDrop();
    void           spawnNewPiece();

    // Drawing functions
    void drawGrid();
    void drawCurrentPiece();
    void drawNextPiece();
    void drawHud();
    void drawBorder();
};
