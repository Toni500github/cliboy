#include "games/2048.hpp"

#include <algorithm>
#include <random>
#include <string>

// Border characters
static uint32_t CH_BORDER_H  = U'═';
static uint32_t CH_BORDER_V  = U'║';
static uint32_t CH_CORNER_TL = U'╔';
static uint32_t CH_CORNER_TR = U'╗';
static uint32_t CH_CORNER_BL = U'╚';
static uint32_t CH_CORNER_BR = U'╝';

// Colors
static constexpr uintattr_t COLOR_EMPTY    = TB_DEFAULT;
static constexpr uintattr_t COLOR_2        = TB_WHITE | TB_BOLD;
static constexpr uintattr_t COLOR_4        = TB_YELLOW | TB_BOLD;
static constexpr uintattr_t COLOR_8        = TB_RED | TB_BOLD;
static constexpr uintattr_t COLOR_16       = TB_MAGENTA | TB_BOLD;
static constexpr uintattr_t COLOR_32       = TB_BLUE | TB_BOLD;
static constexpr uintattr_t COLOR_64       = TB_CYAN | TB_BOLD;
static constexpr uintattr_t COLOR_128      = TB_GREEN | TB_BOLD;
static constexpr uintattr_t COLOR_256      = 0xff135a | TB_BOLD;
static constexpr uintattr_t COLOR_512      = 0x424b59 | TB_BOLD;
static constexpr uintattr_t COLOR_1024     = 0xff4f75 | TB_BOLD;
static constexpr uintattr_t COLOR_2048     = 0x808080 | TB_BOLD;
static constexpr uintattr_t COLOR_HUD      = TB_CYAN | TB_BOLD;
static constexpr uintattr_t COLOR_GAMEOVER = TB_RED | TB_BOLD;
static constexpr uintattr_t COLOR_WIN      = TB_GREEN | TB_BOLD;

Result<> Game2048::on_begin()
{
    set_footer("Arrows: Move | R: Restart | ESC: Back");

    init_game();
    return Ok();
}

void Game2048::init_game()
{
    // Calculate cell size based on terminal dimensions
    int max_cell_w = (display.getWidth() / 2) / GRID_SIZE;
    int max_cell_h = (display.getHeight() / 2) / GRID_SIZE;
    m_cell_w       = std::max(3, std::min(max_cell_w, max_cell_h));
    m_cell_h       = m_cell_w;

    // Center the grid
    m_grid_x = (display.getWidth() - (GRID_SIZE * m_cell_w)) / 2;
    m_grid_y = (display.getHeight() - (GRID_SIZE * m_cell_h)) / 2;

    if (settings.general.utf8)
    {
        CH_BORDER_H  = U'═';
        CH_BORDER_V  = U'║';
        CH_CORNER_TL = U'╔';
        CH_CORNER_TR = U'╗';
        CH_CORNER_BL = U'╚';
        CH_CORNER_BR = U'╝';
    }
    else
    {
        CH_BORDER_H  = '-';
        CH_BORDER_V  = '|';
        CH_CORNER_TL = '+';
        CH_CORNER_TR = '+';
        CH_CORNER_BL = '+';
        CH_CORNER_BR = '+';
    }

    m_grid      = {};
    m_score     = 0;
    m_game_over = false;
    m_won       = false;

    // Add starting tiles
    add_new_tile();
    add_new_tile();
}

void Game2048::add_new_tile()
{
    static std::mt19937 rng{ std::random_device{}() };

    // Find all empty cells
    std::vector<std::pair<int, int>> empty_cells;
    for_2d(GRID_SIZE, GRID_SIZE, [&](int row, int col) {
        if (m_grid[row][col] == 0)
            empty_cells.emplace_back(row, col);
    });

    if (empty_cells.empty())
        return;

    // Randomly choose an empty cell
    std::uniform_int_distribution<int> dist(0, empty_cells.size() - 1);
    auto [row, col] = empty_cells[dist(rng)];

    // 90% chance for 2, 10% chance for 4
    std::uniform_int_distribution<int> value_dist(0, 9);
    m_grid[row][col] = (value_dist(rng) == 0) ? 4 : 2;
}

bool Game2048::move(Direction dir)
{
    const bool by_row   = (dir == Direction::Left || dir == Direction::Right);
    const bool reversed = (dir == Direction::Right || dir == Direction::Down);
    bool       changed  = false;

    for (int i = 0; i < GRID_SIZE; ++i)
    {
        // Collect non-zero values along the line
        std::vector<int> line;
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            // When reversed, walk the line back-to-front during collection
            // so that merging always collapses toward the leading edge.
            const int jj  = reversed ? (GRID_SIZE - 1 - j) : j;
            const int row = by_row ? i : jj;
            const int col = by_row ? jj : i;
            if (m_grid[row][col] != 0)
                line.push_back(m_grid[row][col]);
        }

        // Merge adjacent equal values
        for (size_t k = 0; k + 1 < line.size(); ++k)
        {
            if (line[k] == line[k + 1])
            {
                line[k] *= 2;
                m_score += line[k];
                line.erase(line.begin() + k + 1);
                changed = true;
            }
        }

        // Pad
        while (line.size() < static_cast<size_t>(GRID_SIZE))
            line.push_back(0);

        // Restore natural order before writing back
        if (reversed)
            std::reverse(line.begin(), line.end());

        // Write back and detect positional changes
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            const int row = by_row ? i : j;
            const int col = by_row ? j : i;
            if (m_grid[row][col] != line[j])
            {
                changed          = true;
                m_grid[row][col] = line[j];
            }
        }
    }

    return changed;
}

bool Game2048::is_move_possible() const
{
    // Check for any empty cell
    if (for_2d_until(GRID_SIZE, GRID_SIZE, [&](int row, int col) { return m_grid[row][col] == 0; }))
        return true;

    // Check for adjacent equal values
    return for_2d_until(GRID_SIZE, GRID_SIZE, [&](int row, int col) {
        int val = m_grid[row][col];
        if ((row + 1 < GRID_SIZE && m_grid[row + 1][col] == val) ||
            (col + 1 < GRID_SIZE && m_grid[row][col + 1] == val))
        {
            return true;
        }
        return false;
    });
}

bool Game2048::check_win() const
{
    return for_2d_until(GRID_SIZE, GRID_SIZE, [&](int row, int col) { return m_grid[row][col] >= WIN_VALUE; });
}

uintattr_t Game2048::get_color_for_value(int value) const
{
    switch (value)
    {
        case 0:    return COLOR_EMPTY;
        case 2:    return COLOR_2;
        case 4:    return COLOR_4;
        case 8:    return COLOR_8;
        case 16:   return COLOR_16;
        case 32:   return COLOR_32;
        case 64:   return COLOR_64;
        case 128:  return COLOR_128;
        case 256:  return COLOR_256;
        case 512:  return COLOR_512;
        case 1024: return COLOR_1024;
        case 2048: return COLOR_2048;
        default:   return TB_WHITE | TB_BOLD;
    }
}

std::string Game2048::format_number(int value) const
{
    if (value == 0)
        return "";
    return std::format("{:^{}}", value, m_cell_w);
}

void Game2048::render()
{
    if (!playback.isMusicPlaying())
        playback.playMusic(Game2048Sounds::BGM);

    draw_border();
    draw_grid();
    draw_hud();

    if (m_game_over)
        draw_game_over();
    else if (m_won)
        draw_win();
}

void Game2048::draw_border()
{
    display.setTextColor(COLOR_HUD);

    int x1 = m_grid_x - 1;
    int y1 = m_grid_y - 1;
    int x2 = m_grid_x + (GRID_SIZE * m_cell_w);
    int y2 = m_grid_y + (GRID_SIZE * m_cell_h);

    // Top border
    for (int x = x1 + 1; x < x2; ++x)
        display.drawPixel(x, y1, CH_BORDER_H);

    // Bottom border
    for (int x = x1 + 1; x < x2; ++x)
        display.drawPixel(x, y2, CH_BORDER_H);

    // Left border
    for (int y = y1 + 1; y < y2; ++y)
        display.drawPixel(x1, y, CH_BORDER_V);

    // Right border
    for (int y = y1 + 1; y < y2; ++y)
        display.drawPixel(x2, y, CH_BORDER_V);

    // Corners
    display.drawPixel(x1, y1, CH_CORNER_TL);
    display.drawPixel(x2, y1, CH_CORNER_TR);
    display.drawPixel(x1, y2, CH_CORNER_BL);
    display.drawPixel(x2, y2, CH_CORNER_BR);
}

void Game2048::draw_grid()
{
    for_2d(GRID_SIZE, GRID_SIZE, [&](int row, int col) { draw_cell(row, col, m_grid[row][col]); });
}

void Game2048::draw_cell(int row, int col, int value)
{
    int x = m_grid_x + col * m_cell_w;
    int y = m_grid_y + row * m_cell_h;

    // Background
    display.setTextBgColor(get_color_for_value(value));
    display.drawFilledRect(x, y, m_cell_w, m_cell_h, ' ');

    // Value
    if (value != 0)
    {
        const std::string& str = format_number(value);
        display.setTextColor(TB_BLACK | TB_BOLD);
        display.setCursor(x + (m_cell_w / 2) - (str.length() / 2), y + (m_cell_h / 2));
        display.print(str);
    }
    display.resetColors();
}

void Game2048::draw_hud()
{
    display.setTextColor(COLOR_HUD);

    int hud_x = m_grid_x;
    int hud_y = m_grid_y - 3;

    display.setCursor(hud_x, hud_y);
    display.print("Score: {}", m_score);

    if (m_best_score > 0)
    {
        display.setCursor(hud_x + 15, hud_y);
        display.print("Best: {}", m_best_score);
    }
}

void Game2048::draw_game_over()
{
    display.setTextColor(COLOR_GAMEOVER);
    int mid_y = m_grid_y + (GRID_SIZE * m_cell_h) / 2;

    if (settings.general.utf8)
    {
        display.centerText(mid_y - 1, "╔══════════════════╗");
        display.centerText(mid_y - 0, "║    GAME  OVER    ║");
        display.centerText(mid_y + 1, "╚══════════════════╝");
    }
    else
    {
        display.centerText(mid_y - 1, "+------------------+");
        display.centerText(mid_y - 0, "|    GAME  OVER    |");
        display.centerText(mid_y + 1, "+------------------+");
    }

    display.setTextColor(TB_WHITE);
    display.centerText(mid_y + 3, "R: Restart   ESC: Menu");
}

void Game2048::draw_win()
{
    display.setTextColor(COLOR_WIN);
    int mid_y = m_grid_y + (GRID_SIZE * m_cell_h) / 2;

    if (settings.general.utf8)
    {
        display.centerText(mid_y - 2, "╔══════════════════╗");
        display.centerText(mid_y - 1, "║     YOU WIN!     ║");
        display.centerText(mid_y - 0, "║   Score: {:>6}  ║", m_score);
        display.centerText(mid_y + 1, "╚══════════════════╝");
    }
    else
    {
        display.centerText(mid_y - 2, "+------------------+");
        display.centerText(mid_y - 1, "|     YOU WIN!     |");
        display.centerText(mid_y - 0, "|   Score: {:>6}  |", m_score);
        display.centerText(mid_y + 1, "+------------------+");
    }

    display.setTextColor(TB_WHITE);
    display.centerText(mid_y + 3, "R: Restart   ESC: Menu");
}

SceneResult Game2048::handle_input(uint32_t key)
{
    if (key == TB_KEY_ESC)
        return Scenes::GamesMenu;

    if (key == 'r' || key == 'R')
    {
        init_game();
        return ScenesGame::Game2048;
    }

    if (m_game_over)
        return ScenesGame::Game2048;

    bool moved = false;

    switch (key)
    {
        case TB_KEY_ARROW_LEFT:  moved = move(Direction::Left); break;
        case TB_KEY_ARROW_RIGHT: moved = move(Direction::Right); break;
        case TB_KEY_ARROW_UP:    moved = move(Direction::Up); break;
        case TB_KEY_ARROW_DOWN:  moved = move(Direction::Down); break;
        default:                 break;
    }

    if (moved)
    {
        add_new_tile();

        if (!m_won && check_win())
        {
            m_won = true;
            // Update best score
            if (m_score > m_best_score)
                m_best_score = m_score;
        }

        if (!is_move_possible())
            m_game_over = true;
    }

    return ScenesGame::Game2048;
}
