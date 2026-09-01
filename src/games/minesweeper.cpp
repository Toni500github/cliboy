#include "games/minesweeper.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <format>
#include <random>
#include <utility>

#include "audio_player.hpp"
#include "terminal_display.hpp"
#include "util.hpp"

namespace
{
    constexpr const struct DiffConf
    {
        int cols;
        int rows;
        int mines;
    } k_diff[] = { { 9, 9, 10 }, { 16, 16, 40 }, { 30, 16, 99 } };

    constexpr int k_max_secs = 999;  // seconds, capped for display

    constexpr uintattr_t k_num_fg[] = {
        0x000000,  // 0  unused
        0x4499FF,  // 1  blue
        0x44BB44,  // 2  green
        0xFF5555,  // 3  red
        0x4444CC,  // 4  dark blue
        0xBB4444,  // 5  maroon
        0x44BBBB,  // 6  teal
        0xBBBBBB,  // 7  light grey
        0x888888,  // 8  dark grey
    };

    // Palette
    enum Colors : uintattr_t
    {
        HiddenFg   = 0x8888aa,
        HiddenBg   = 0x2e2e50,
        FlagFg     = 0xFF7777,
        FlagBg     = 0x2e2e50,
        RevealedBg = 0x1a1a2e,
        MineFg     = 0xFF4444,
        MineBg     = 0x4a0000,
        CursorFg   = 0x1a1a2e,
        CursorBg   = 0xCCCCFF,
        Accent     = 0xFFAA33,
        Hud        = 0xDDDDDD,
    };

    // 8-directional neighbour offsets (same ordering used throughout)
    constexpr int k_dx[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    constexpr int k_dy[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
}  // namespace

Result<> Minesweeper::onGameBegin()
{
    setFooter("Arrows:Move | Space:Reveal | F:Flag | R:Restart | P:Pause");
    return Ok();
}

void Minesweeper::initGame()
{
    m_cols        = k_diff[idx(m_diff)].cols;
    m_rows        = k_diff[idx(m_diff)].rows;
    m_total_mines = k_diff[idx(m_diff)].mines;

    m_grid.assign(m_cols * m_rows, Cell{});
    m_cursor_x             = m_cols / 2;
    m_cursor_y             = m_rows / 2;
    m_flags_placed         = 0;
    m_cells_revealed       = 0;
    m_first_click          = true;
    m_timer_running        = false;
    m_elapsed_before_pause = {};

    // Re-derived grid origin from the current terminal dimensions so it
    // stays centered after any resize that happens between sessions.
    m_grid_x = std::max(0, (display.getWidth() - m_cols * k_cell_w) / 2);
    m_grid_y = std::max(2, (display.getHeight() - m_rows) / 2);
}

// HUD

int Minesweeper::drawHud()
{
    auto elapsed = m_elapsed_before_pause;
    if (m_timer_running)
        elapsed += std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_segment_start);

    const int elapsed_secs = std::min(int(elapsed.count()), k_max_secs);
    const int mines_left   = m_total_mines - m_flags_placed;

    display.setTextColor(Colors::Accent);
    display.centerText(m_grid_y - 2, "Minesweeper");

    display.setTextColor(Colors::Hud);
    display.centerText(m_grid_y - 1, "Mines: {:2}    Time: {:03}", mines_left, elapsed_secs);

    display.resetColors();

    return elapsed_secs;
}

void Minesweeper::drawCell(int gx, int gy)
{
    const Cell& c      = at(gx, gy);
    const bool  cursor = (gx == m_cursor_x && gy == m_cursor_y);
    const int   sx     = m_grid_x + gx * k_cell_w;
    const int   sy     = m_grid_y + gy;

    uintattr_t fg, bg;
    uint32_t   ch0, ch1;

    if (!c.is_revealed)
    {
        if (c.is_flagged)
        {
            fg  = cursor ? Colors::CursorFg : Colors::FlagFg;
            bg  = cursor ? Colors::FlagFg : Colors::FlagBg;
            ch0 = 'F';
            ch1 = ' ';
        }
        else
        {
            fg  = cursor ? Colors::CursorFg : Colors::HiddenFg;
            bg  = cursor ? Colors::CursorBg : Colors::HiddenBg;
            ch0 = '#';
            ch1 = '#';
        }
    }
    else if (c.is_mine)
    {
        fg  = Colors::MineFg;
        bg  = cursor ? 0xFF2222 : Colors::MineBg;
        ch0 = '*';
        ch1 = ' ';
    }
    else if (c.adjacent == 0)
    {
        fg  = cursor ? Colors::CursorFg : Colors::RevealedBg;
        bg  = cursor ? Colors::CursorBg : Colors::RevealedBg;
        ch0 = ' ';
        ch1 = ' ';
    }
    else
    {
        fg  = cursor ? Colors::CursorFg : k_num_fg[c.adjacent];
        bg  = cursor ? Colors::CursorBg : Colors::RevealedBg;
        ch0 = ' ';
        ch1 = static_cast<uint32_t>('0' + c.adjacent);
    }

    display.setTextColor(fg);
    display.setTextBgColor(bg);
    display.drawPixel(sx, sy, ch0);
    display.drawPixel(sx + 1, sy, ch1);
    display.resetColors();
}

void Minesweeper::drawDifficultMenu()
{
    display.clearDisplay();

    display.setFont(FigletType::FullWidth, "starwars");
    display.setTextColor(TB_MAGENTA | TB_BOLD);
    display.centerText(display.pctY(0.3), "Difficulty");
    display.resetFont();

    if (m_diff == Difficulty::Beginner)
    {
        display.setTextColor(TB_GREEN | TB_BOLD);
        display.centerText(display.pctY(0.5), "< Beginner (9x9, 10 mines) >");
    }
    else if (m_diff == Difficulty::Intermediate)
    {
        display.setTextColor(TB_YELLOW | TB_BOLD);
        display.centerText(display.pctY(0.5), "< Intermediate (16x16, 40 mines) >");
    }
    else if (m_diff == Difficulty::Expert)
    {
        display.setTextColor(TB_RED | TB_BOLD);
        display.centerText(display.pctY(0.5), "< Expert (30x16, 99 mines) >");
    }

    display.resetColors();
    display.display();
}

void Minesweeper::drawGrid()
{
    for_2d(m_cols, m_rows, [&](int gx, int gy) { drawCell(gx, gy); });
}

void Minesweeper::renderGame()
{
    if (m_choosing_diff)
    {
        drawDifficultMenu();
        return;
    }

    if (!isPlaying() && m_timer_running)
    {
        m_elapsed_before_pause +=
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_segment_start);
        m_timer_running = false;
    }

    const int elapsed_secs = drawHud();
    drawGrid();

    // Endgame overlays will be drawn kinda up of the footer,
    // rather than in the center of the grid, so that mines can be visible.
    const int center_y = display.pctY(0.9);

    if (isGameOver())
    {
        drawGameOverOverlay({ { "Time", std::format("{}s", elapsed_secs) } }, center_y);
    }
    else if (isWon())
    {
        drawWinOverlay({ { "Time", std::format("{}s", elapsed_secs) }, { "Mines", std::to_string(m_total_mines) } },
                       center_y);
    }
    else if (isPaused())
    {
        drawPausedOverlay();
    }
    else if (!m_first_click && !m_timer_running)
    {
        m_segment_start = std::chrono::steady_clock::now();
        m_timer_running = true;
    }

    if (!playback.isMusicPlaying())
        playback.playMusic(MinesweeperSounds::BGM);
}

// Mine placement
void Minesweeper::placeMine(int safe_x, int safe_y)
{
    // Exclude the 3x3 zone around the first click so it's always safe
    std::vector<int> pool;
    pool.reserve(m_cols * m_rows);
    for_2d(m_cols, m_rows, [&](int x, int y) {
        if (std::abs(x - safe_x) > 1 || std::abs(y - safe_y) > 1)
            pool.push_back(y * m_cols + x);
    });

    std::mt19937 rng{ std::random_device{}() };
    std::shuffle(pool.begin(), pool.end(), rng);

    int n = std::min(m_total_mines, static_cast<int>(pool.size()));
    for (int i = 0; i < n; ++i)
        m_grid[pool[i]].is_mine = true;
}

void Minesweeper::computeAdjacency()
{
    for_2d(m_cols, m_rows, [&](int x, int y) {
        if (at(x, y).is_mine)
            return;
        int cnt = 0;
        for (int d = 0; d < 8; ++d)
        {
            int nx = x + k_dx[d];
            int ny = y + k_dy[d];
            if (ok(nx, ny) && at(nx, ny).is_mine)
                ++cnt;
        }
        at(x, y).adjacent = cnt;
    });
}

// Game Logic
void Minesweeper::reveal(int start_x, int start_y)
{
    // Use a growing vector as queue (indices walk forward, pushes append)
    std::vector<std::pair<int, int>> queue;
    queue.reserve(64);
    queue.push_back({ start_x, start_y });

    for (size_t i = 0; i < queue.size(); ++i)
    {
        const auto [x, y] = queue[i];
        if (!ok(x, y))
            continue;

        Cell& c = at(x, y);
        if (c.is_revealed || c.is_flagged || c.is_mine)
            continue;

        c.is_revealed = true;
        ++m_cells_revealed;

        // Expand into neighbours only if this cell has no adjacent mines
        if (c.adjacent == 0)
            for (int d = 0; d < 8; ++d)
                queue.push_back({ x + k_dx[d], y + k_dy[d] });
    }
}

// If the cursor is on a revealed number and the correct number of
// adjacent flags is placed, reveal all non-flagged neighbours
void Minesweeper::chord(int x, int y)
{
    const Cell& c = at(x, y);
    if (!c.is_revealed || c.adjacent == 0)
        return;

    int flags = 0;
    for (int d = 0; d < 8; ++d)
    {
        int nx = x + k_dx[d];
        int ny = y + k_dy[d];
        if (ok(nx, ny) && at(nx, ny).is_flagged)
            ++flags;
    }

    if (flags != c.adjacent)
        return;

    for (int d = 0; d < 8; ++d)
    {
        int nx = x + k_dx[d];
        int ny = y + k_dy[d];
        if (!ok(nx, ny))
            continue;

        Cell& nc = at(nx, ny);
        if (nc.is_flagged || nc.is_revealed)
            continue;

        if (nc.is_mine)
        {
            triggerLoss();
            return;
        }
        reveal(nx, ny);
    }
}

void Minesweeper::toggleFlag(int x, int y)
{
    Cell& c = at(x, y);
    if (c.is_revealed)
        return;
    if (c.is_flagged)
    {
        c.is_flagged = false;
        --m_flags_placed;
    }
    else
    {
        c.is_flagged = true;
        ++m_flags_placed;
    }
}

bool Minesweeper::checkWin() const
{
    return m_cells_revealed == (m_cols * m_rows - m_total_mines);
}

void Minesweeper::triggerLoss()
{
    // Reveal every mine (cursor cell will have is_revealed set by caller)
    for (Cell& c : m_grid)
        if (c.is_mine)
            c.is_revealed = true;
    setGameState(GameState::GameOver);
}

void Minesweeper::handleReveal()
{
    Cell& c = at(m_cursor_x, m_cursor_y);
    if (c.is_flagged)
        return;

    if (m_first_click)
    {
        m_first_click   = false;
        m_timer_running = true;
        m_segment_start = std::chrono::steady_clock::now();
        placeMine(m_cursor_x, m_cursor_y);
        computeAdjacency();
    }

    if (c.is_revealed)
    {
        chord(m_cursor_x, m_cursor_y);
    }
    else if (c.is_mine)
    {
        c.is_revealed = true;  // highlight this mine before sweeping
        triggerLoss();
        return;
    }
    else
    {
        reveal(m_cursor_x, m_cursor_y);
    }

    if (isPlaying() && checkWin())
        setGameState(GameState::Won);
}

SceneResult Minesweeper::handleInput(uint32_t key)
{
    if (m_choosing_diff)
    {
        switch (key)
        {
            case TB_KEY_ARROW_UP:
            case TB_KEY_ARROW_LEFT:
                m_diff = Difficulty((idx(m_diff) - 1 + idx(Difficulty::Count)) % idx(Difficulty::Count));
                break;
            case TB_KEY_ARROW_DOWN:
            case TB_KEY_ARROW_RIGHT: m_diff = Difficulty((idx(m_diff) + 1) % idx(Difficulty::Count)); break;
            case TB_KEY_ENTER:
                m_choosing_diff = false;
                initGame();
                break;
            case TB_KEY_ESC: return Scenes::GamesMenu;
        }
        return sceneID();
    }

    if (const auto r = handleCommonInput(key))
        return *r;

    if (tolower(key) == 'r' && isPlaying())
    {
        initGame();
        return sceneID();
    }

    if (!isPlaying())
        return sceneID();

    switch (key)
    {
        case TB_KEY_ARROW_UP:    m_cursor_y = std::max(0, m_cursor_y - 1); break;
        case TB_KEY_ARROW_DOWN:  m_cursor_y = std::min(m_rows - 1, m_cursor_y + 1); break;
        case TB_KEY_ARROW_LEFT:  m_cursor_x = std::max(0, m_cursor_x - 1); break;
        case TB_KEY_ARROW_RIGHT: m_cursor_x = std::min(m_cols - 1, m_cursor_x + 1); break;
    }

    if (key == TB_KEY_SPACE || key == TB_KEY_ENTER)
        handleReveal();

    if (tolower(key) == 'f')
        toggleFlag(m_cursor_x, m_cursor_y);

    return sceneID();
}
