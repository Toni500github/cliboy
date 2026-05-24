#pragma once

#include <array>
#include <cstdint>

#include "game.hpp"
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

class Game2048 : public BaseGame
{
public:
    Game2048() : m_grid_x(0), m_grid_y(0), m_cell_w(0), m_cell_h(0), m_cell_padding(0) {};
    ~Game2048() override = default;

    SceneResult handleInput(uint32_t key) override;

protected:
    void        initGame() override;
    void        renderGame() override;
    Result<>    onGameBegin() override;
    SceneResult sceneID() const override { return ScenesGame::Game2048; }

private:
    Grid m_grid;

    // Position and dimensions
    int m_grid_x;
    int m_grid_y;
    int m_cell_w;
    int m_cell_h;
    int m_cell_padding;

    // Helper functions
    void        addNewTile();
    bool        move(Direction d);
    bool        isMovePossible() const;
    bool        checkWin() const;
    uintattr_t  getColorForValue(int value) const;
    std::string formatNumber(int value) const;

    // Drawing functions
    void drawGrid();
    void drawCell(int row, int col, int value);
    void drawHud();
    void drawBorder();
};
