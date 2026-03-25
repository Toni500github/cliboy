#include "games/tetris.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <random>

#include "audio_player.hpp"

static constexpr int GRID_WIDTH  = 10;
static constexpr int GRID_HEIGHT = 20;
static constexpr int NEXT_SIZE   = 4;  // Size of next piece preview area

static uint32_t CH_BORDER_H  = U'═';
static uint32_t CH_BORDER_V  = U'║';
static uint32_t CH_CORNER_TL = U'╔';
static uint32_t CH_CORNER_TR = U'╗';
static uint32_t CH_CORNER_BL = U'╚';
static uint32_t CH_CORNER_BR = U'╝';
static uint32_t CH_BLOCK     = U'█';

// Colors (using termbox2 color constants)
static constexpr uintattr_t COLOR_I        = TB_CYAN | TB_BOLD;
static constexpr uintattr_t COLOR_O        = TB_YELLOW | TB_BOLD;
static constexpr uintattr_t COLOR_T        = TB_MAGENTA | TB_BOLD;
static constexpr uintattr_t COLOR_S        = TB_GREEN | TB_BOLD;
static constexpr uintattr_t COLOR_Z        = TB_RED | TB_BOLD;
static constexpr uintattr_t COLOR_J        = TB_BLUE | TB_BOLD;
static constexpr uintattr_t COLOR_L        = TB_YELLOW | TB_BOLD;  // Orange-ish
static constexpr uintattr_t COLOR_HUD      = TB_CYAN | TB_BOLD;
static constexpr uintattr_t COLOR_GAMEOVER = TB_RED | TB_BOLD;
static constexpr uintattr_t COLOR_PAUSED   = TB_YELLOW | TB_BOLD;

// Scoring
static constexpr int SCORES[] = { 0, 40, 100, 300, 1200 };  // 1, 2, 3, 4 lines

Result<> TetrisGame::on_begin()
{
    set_footer("← →: Move | ↑: Rotate | ↓: Soft Drop | Space: Hard Drop | P: Pause | ESC: Back");
    playback.playMusic(TetrisSounds::BGM);

    init_game();
    return Ok();
}

void TetrisGame::init_game()
{
    m_grid.assign(GRID_HEIGHT, std::vector<uint32_t>(GRID_WIDTH, 0));

    // Choose cell size based on terminal dimensions
    // We need at least GRID_WIDTH * cell_size width and GRID_HEIGHT * cell_size height
    m_cell_size         = 1;
    int max_cell_width  = display.getWidth() / (GRID_WIDTH + NEXT_SIZE + 4);  // +4 for padding
    int max_cell_height = display.getHeight() / (GRID_HEIGHT + 2);            // +2 for border

    m_cell_size = std::max(1, std::min(max_cell_width, max_cell_height));

    // Calculate grid dimensions in characters
    m_grid_w = GRID_WIDTH * m_cell_size;
    m_grid_h = GRID_HEIGHT * m_cell_size;

    // Center the grid
    m_grid_x = (display.getWidth() - m_grid_w - NEXT_SIZE * m_cell_size - 8) / 2;
    m_grid_y = (display.getHeight() - m_grid_h) / 2;

    if (settings.general.utf8)
    {
        CH_BORDER_H  = U'═';
        CH_BORDER_V  = U'║';
        CH_CORNER_TL = U'╔';
        CH_CORNER_TR = U'╗';
        CH_CORNER_BL = U'╚';
        CH_CORNER_BR = U'╝';
        CH_BLOCK     = U'█';
    }
    else
    {
        CH_BORDER_H  = '-';
        CH_BORDER_V  = '|';
        CH_CORNER_TL = '+';
        CH_CORNER_TR = '+';
        CH_CORNER_BL = '+';
        CH_CORNER_BR = '+';
        CH_BLOCK     = '#';
    }

    m_score         = 0;
    m_lines_cleared = 0;
    m_level         = 0;
    m_game_over     = false;
    m_paused        = false;
    m_fall_timer    = 0;
    m_last_update   = 0;

    // Create initial pieces
    m_next_piece = get_random_piece();
    spawn_new_piece();
}

Tetromino TetrisGame::get_random_piece()
{
    static std::mt19937               rng{ std::random_device{}() };
    static std::vector<TetrominoType> bag;

    if (bag.empty())
    {
        bag = { TetrominoType::I, TetrominoType::O, TetrominoType::T, TetrominoType::S,
                TetrominoType::Z, TetrominoType::J, TetrominoType::L };
        std::shuffle(bag.begin(), bag.end(), rng);
    }

    TetrominoType type = bag.back();
    bag.pop_back();
    return spawn_piece(type);
}

Tetromino TetrisGame::spawn_piece(TetrominoType type)
{
    Tetromino piece;
    piece.type  = type;
    piece.shape = get_shape_for_type(type);
    piece.x     = (GRID_WIDTH - static_cast<int>(piece.shape[0].size())) / 2;
    piece.y     = 0;

    return piece;
}

TetrominoShape TetrisGame::get_shape_for_type(TetrominoType type)
{
    // clang-format off
    switch (type)
    {
        case TetrominoType::I:
            return {{
                {0, 0, 0, 0},
                {1, 1, 1, 1},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
            }};
        case TetrominoType::O:
            return {{
                {0, 0, 0, 0},
                {0, 1, 1, 0},
                {0, 1, 1, 0},
                {0, 0, 0, 0}
            }};
        case TetrominoType::T:
            return {{
                {0, 0, 0, 0},
                {0, 1, 0, 0},
                {1, 1, 1, 0},
                {0, 0, 0, 0}
            }};
        case TetrominoType::S:
            return {{
                {0, 0, 0, 0},
                {0, 1, 1, 0},
                {1, 1, 0, 0},
                {0, 0, 0, 0}
            }};
        case TetrominoType::Z:
            return {{
                {0, 0, 0, 0},
                {1, 1, 0, 0},
                {0, 1, 1, 0},
                {0, 0, 0, 0}
            }};
        case TetrominoType::J:
            return {{
                {0, 0, 0, 0},
                {1, 0, 0, 0},
                {1, 1, 1, 0},
                {0, 0, 0, 0}
            }};
        case TetrominoType::L:
            return {{
                {0, 0, 0, 0},
                {0, 0, 1, 0},
                {1, 1, 1, 0},
                {0, 0, 0, 0}
            }};
        default:
            return {};
    }
    // clang-format on
}

uintattr_t TetrisGame::get_color_for_type(TetrominoType type)
{
    switch (type)
    {
        case TetrominoType::I: return COLOR_I;
        case TetrominoType::O: return COLOR_O;
        case TetrominoType::T: return COLOR_T;
        case TetrominoType::S: return COLOR_S;
        case TetrominoType::Z: return COLOR_Z;
        case TetrominoType::J: return COLOR_J;
        case TetrominoType::L: return COLOR_L;
        default:               return TB_WHITE;
    }
}

bool TetrisGame::collides(const Tetromino& piece, int dx, int dy) const
{
    bool collides = false;
    for_2d(piece.shape.size(), piece.shape[0].size(), [&](int row, int col) {
        if (!piece.shape[row][col])
            return;

        int new_x = piece.x + static_cast<int>(col) + dx;
        int new_y = piece.y + static_cast<int>(row) + dy;

        if ((new_x < 0 || new_x >= GRID_WIDTH || new_y >= GRID_HEIGHT) || (new_y >= 0 && m_grid[new_y][new_x] != 0))
        {
            collides = true;
        }
    });
    return collides;
}

bool TetrisGame::move_piece(int dx, int dy)
{
    if (!collides(m_current_piece, dx, dy))
    {
        m_current_piece.x += dx;
        m_current_piece.y += dy;
        return true;
    }
    return false;
}

void TetrisGame::rotate_piece()
{
    // Create rotated shape
    const size_t size = m_current_piece.shape.size();

    TetrominoShape rotated;
    for_2d(size, size, [&](int row, int col) { rotated[col][size - 1 - row] = m_current_piece.shape[row][col]; });

    // Save original shape and position
    auto original_shape = m_current_piece.shape;
    int  original_x     = m_current_piece.x;

    m_current_piece.shape = rotated;

    // Wall kick: try shifting left/right if rotated piece collides
    if (!collides(m_current_piece))
        return;  // Rotation successful

    // Try shifting left
    m_current_piece.x = original_x - 1;
    if (!collides(m_current_piece))
        return;

    // Try shifting right
    m_current_piece.x = original_x + 1;
    if (!collides(m_current_piece))
        return;

    // Try shifting up (rare, but for I piece sometimes)
    if (!collides(m_current_piece, 0, -1))
    {
        m_current_piece.y -= 1;
        return;
    }

    // Restore original shape and position
    m_current_piece.shape = original_shape;
    m_current_piece.x     = original_x;
}

void TetrisGame::hard_drop()
{
    while (move_piece(0, 1))
    {
        // Keep dropping
    }
    merge_piece();
}

void TetrisGame::merge_piece()
{
    const Tetromino&      piece = m_current_piece;
    const TetrominoShape& shape = piece.shape;

    // Add piece to grid
    for_2d(shape.size(), shape[0].size(), [&](int row, int col) {
        if (!shape[row][col])
            return;
        int grid_y = piece.y + row;
        int grid_x = piece.x + col;

        if (grid_y >= 0 && grid_y < GRID_HEIGHT && grid_x >= 0 && grid_x < GRID_WIDTH)
            m_grid[grid_y][grid_x] = get_color_for_type(piece.type);
    });

    clear_lines_and_update_score();

    spawn_new_piece();

    if (collides(piece))
        m_game_over = true;
}

void TetrisGame::clear_lines_and_update_score()
{
    int lines_cleared = 0;

    for (int row = GRID_HEIGHT - 1; row >= 0; --row)
    {
        bool full = true;
        for (int col = 0; col < GRID_WIDTH; ++col)
        {
            if (m_grid[row][col] == 0)
            {
                full = false;
                break;
            }
        }

        if (full)
        {
            // Move all rows above down
            for (int r = row; r > 0; --r)
                m_grid[r] = m_grid[r - 1];
            // Clear top row
            m_grid[0].assign(GRID_WIDTH, 0);
            lines_cleared++;
            row++;  // Check the same row again (now it's the row that shifted down)
        }
    }

    if (lines_cleared > 0)
    {
        m_lines_cleared += lines_cleared;
        m_score += calculate_score(lines_cleared);
        update_level();
    }
}

int TetrisGame::calculate_score(int lines)
{
    int lines_idx = std::min(lines, 4);
    return SCORES[lines_idx] * (m_level + 1);
}

void TetrisGame::update_level()
{
    int new_level = m_lines_cleared / 10;
    if (new_level > m_level)
        m_level = new_level;
}

int TetrisGame::get_fall_delay_ms() const
{
    // Classic Tetris speed curve
    // Level 0: 500ms, Level 1: 400ms, etc.
    int delay = 500 - (m_level * 40);
    return std::max(50, delay);
}

void TetrisGame::spawn_new_piece()
{
    m_current_piece = m_next_piece;
    m_next_piece    = get_random_piece();

    // Reset position
    m_current_piece.x = (GRID_WIDTH - static_cast<int>(m_current_piece.shape[0].size())) / 2;
    m_current_piece.y = 0;
}

void TetrisGame::render()
{
    auto now =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();

    // Update fall timer (only if not game over and not paused)
    if (!m_game_over && !m_paused)
    {
        if (m_last_update == 0)
        {
            m_last_update = now;
            m_fall_timer  = now;
        }
        else
        {
            if (now - m_fall_timer >= static_cast<uint32_t>(get_fall_delay_ms()))
            {
                if (!move_piece(0, 1))
                    merge_piece();
                m_fall_timer = now;
            }
        }
        m_last_update = now;
    }

    draw_border();
    draw_grid();
    draw_current_piece();
    draw_next_piece();
    draw_hud();

    if (m_game_over)
        draw_game_over();
    else if (m_paused)
        draw_paused();
    else if (!playback.isMusicPlaying())
        playback.resumeMusic();
}

void TetrisGame::draw_border()
{
    display.setTextColor(COLOR_HUD);

    // Main grid border
    int x1 = m_grid_x - 1;
    int y1 = m_grid_y - 1;
    int x2 = m_grid_x + m_grid_w;
    int y2 = m_grid_y + m_grid_h;

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

void TetrisGame::draw_grid()
{
    for_2d(GRID_WIDTH, GRID_HEIGHT, [&](int col, int row) {
        uint32_t cell = m_grid[row][col];
        if (cell == 0)
            return;

        display.setTextColor(static_cast<uintattr_t>(cell));
        for_2d(m_cell_size, m_cell_size, [&](int dx, int dy) {
            int x = m_grid_x + col * m_cell_size + dx;
            int y = m_grid_y + row * m_cell_size + dy;
            display.drawPixel(x, y, CH_BLOCK);
        });
    });
}

void TetrisGame::draw_current_piece()
{
    const Tetromino&      piece = m_current_piece;
    const TetrominoShape& shape = piece.shape;

    display.setTextColor(get_color_for_type(piece.type));

    for_2d(shape.size(), shape[0].size(), [&](int col, int row) {
        if (shape[row][col])
        {
            for_2d(m_cell_size, m_cell_size, [&](int dx, int dy) {
                int x = m_grid_x + (piece.x + col) * m_cell_size + dx;
                int y = m_grid_y + (piece.y + row) * m_cell_size + dy;

                if (y >= m_grid_y && y < m_grid_y + m_grid_h)
                    display.drawPixel(x, y, CH_BLOCK);
            });
        };
    });
}

void TetrisGame::draw_next_piece()
{
    const Tetromino&      piece = m_next_piece;
    const TetrominoShape& shape = piece.shape;

    display.setTextColor(COLOR_HUD);

    int next_x = m_grid_x + m_grid_w + m_cell_size * 2;
    int next_y = m_grid_y + m_cell_size;

    // Draw "NEXT" label
    display.setCursor(next_x, next_y - 1);
    display.print("NEXT");

    // Draw border around next piece area
    int next_w = NEXT_SIZE * m_cell_size;
    int next_h = NEXT_SIZE * m_cell_size;

    int next_x1 = next_x - 1;
    int next_y1 = next_y - 1;
    int next_x2 = next_x + next_w;
    int next_y2 = next_y + next_h;

    // Simple border
    for (int x = next_x1 + 1; x < next_x2; ++x)
    {
        display.drawPixel(x, next_y1, CH_BORDER_H);
        display.drawPixel(x, next_y2, CH_BORDER_H);
    }
    for (int y = next_y1 + 1; y < next_y2; ++y)
    {
        display.drawPixel(next_x1, y, CH_BORDER_V);
        display.drawPixel(next_x2, y, CH_BORDER_V);
    }
    display.drawPixel(next_x1, next_y1, CH_CORNER_TL);
    display.drawPixel(next_x2, next_y1, CH_CORNER_TR);
    display.drawPixel(next_x1, next_y2, CH_CORNER_BL);
    display.drawPixel(next_x2, next_y2, CH_CORNER_BR);

    // Draw the piece
    display.setTextColor(get_color_for_type(piece.type));

    for_2d(shape.size(), shape[0].size(), [&](int col, int row) {
        if (shape[row][col])
        {
            for_2d(m_cell_size, m_cell_size, [&](int dx, int dy) {
                int x = next_x + col * m_cell_size + dx;
                int y = next_y + row * m_cell_size + dy;
                display.drawPixel(x, y, CH_BLOCK);
            });
        };
    });
}

void TetrisGame::draw_hud()
{
    display.setTextColor(COLOR_HUD);

    int next_y = m_grid_y + m_cell_size;

    int hud_x = m_grid_x + m_grid_w + m_cell_size * 2;
    int hud_y = next_y + NEXT_SIZE * m_cell_size + m_cell_size * 2;

    display.setCursor(hud_x, hud_y);
    display.print("Score: {}", m_score);

    display.setCursor(hud_x, hud_y + 1);
    display.print("Lines: {}", m_lines_cleared);

    display.setCursor(hud_x, hud_y + 2);
    display.print("Level: {}", m_level);
}

void TetrisGame::draw_game_over()
{
    display.setTextColor(COLOR_GAMEOVER);
    int mid_y = m_grid_y + m_grid_h / 2;

    if (settings.general.utf8)
    {
        display.centerText(mid_y - 2, "╔══════════════════╗");
        display.centerText(mid_y - 1, "║    GAME  OVER    ║");
        display.centerText(mid_y - 0, "║   Score: {:>6}  ║", m_score);
        display.centerText(mid_y + 1, "║   Lines: {:>6}  ║", m_lines_cleared);
        display.centerText(mid_y + 2, "╚══════════════════╝");
    }
    else
    {
        display.centerText(mid_y - 2, "+------------------+");
        display.centerText(mid_y - 1, "|    GAME  OVER    |");
        display.centerText(mid_y - 0, "|   Score: {:>6}  |", m_score);
        display.centerText(mid_y + 1, "|   Lines: {:>6}  |", m_lines_cleared);
        display.centerText(mid_y + 2, "+------------------+");
    }

    display.setTextColor(TB_WHITE);
    display.centerText(mid_y + 4, "R: Restart   ESC: Menu");
}

void TetrisGame::draw_paused()
{
    display.setTextColor(COLOR_PAUSED);
    int mid_y = m_grid_y + m_grid_h / 2;

    if (settings.general.utf8)
    {
        display.centerText(mid_y - 1, "╔════════════╗");
        display.centerText(mid_y - 0, "║   PAUSED   ║");
        display.centerText(mid_y + 1, "╚════════════╝");
    }
    else
    {
        display.centerText(mid_y - 1, "+------------+");
        display.centerText(mid_y - 0, "|   PAUSED   |");
        display.centerText(mid_y + 1, "+------------+");
    }
}

SceneResult TetrisGame::handle_input(uint32_t key)
{
    if (key == TB_KEY_ESC)
    {
        playback.pauseMusic();
        return Scenes::GamesMenu;
    }

    if (m_game_over)
    {
        if (key == 'r' || key == 'R')
            init_game();
        return ScenesGame::Tetris;
    }

    if (key == 'p' || key == 'P')
    {
        m_paused = !m_paused;
        if (m_paused)
            playback.pauseMusic();
        else
            playback.resumeMusic();
        return ScenesGame::Tetris;
    }

    if (m_paused)
        return ScenesGame::Tetris;

    // Game controls
    switch (key)
    {
        case TB_KEY_ARROW_LEFT:  move_piece(-1, 0); break;
        case TB_KEY_ARROW_RIGHT: move_piece(1, 0); break;
        case TB_KEY_ARROW_DOWN:  move_piece(0, 1); break;
        case TB_KEY_ARROW_UP:    rotate_piece(); break;
        case TB_KEY_SPACE:       hard_drop(); break;
        default:                 break;
    }

    return ScenesGame::Tetris;
}
