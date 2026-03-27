#include "games/snake.hpp"

#include <algorithm>
#include <string>

// Characters
static uint32_t CH_SNAKE_HEAD = U'◉';
static uint32_t CH_SNAKE_BODY = U'█';
static uint32_t CH_FOOD       = U'●';
static uint32_t CH_BORDER_H   = U'═';
static uint32_t CH_BORDER_V   = U'║';
static uint32_t CH_CORNER_TL  = U'╔';
static uint32_t CH_CORNER_TR  = U'╗';
static uint32_t CH_CORNER_BL  = U'╚';
static uint32_t CH_CORNER_BR  = U'╝';

// Colors
static constexpr uintattr_t COL_BORDER     = TB_WHITE;
static constexpr uintattr_t COL_SNAKE_HEAD = TB_GREEN | TB_BOLD;
static constexpr uintattr_t COL_SNAKE_BODY = TB_GREEN;
static constexpr uintattr_t COL_FOOD       = TB_RED | TB_BOLD;
static constexpr uintattr_t COL_HUD        = TB_CYAN | TB_BOLD;
static constexpr uintattr_t COL_GAMEOVER   = TB_RED | TB_BOLD;
static constexpr uintattr_t COL_PAUSED     = TB_YELLOW | TB_BOLD;

// Speed
static constexpr int SPEED_STEP_MS   = 10;  // ms reduction per milestone
static constexpr int SPEED_MILESTONE = 5;   // points between speed-ups

Result<> SnakeGame::on_begin()
{
    set_footer("Arrows: Move | P: Pause | ESC: Back");

    init_game();
    return Ok();
}

void SnakeGame::render()
{
    update();

    draw_border();
    draw_hud();

    if (m_dead)
    {
        draw_game_over();
        return;
    }

    // Food
    display.setTextColor(COL_FOOD);
    display.drawPixel(m_food.x, m_food.y, CH_FOOD);

    // Snake (head first so its character sits on top)
    bool is_head = true;
    for (const auto& seg : m_snake)
    {
        display.setTextColor(is_head ? COL_SNAKE_HEAD : COL_SNAKE_BODY);
        display.drawPixel(seg.x, seg.y, is_head ? CH_SNAKE_HEAD : CH_SNAKE_BODY);
        is_head = false;
    }

    if (m_paused)
        draw_paused();
}

SceneResult SnakeGame::handle_input(uint32_t key)
{
    if (key == TB_KEY_ESC)
        return Scenes::GamesMenu;

    if (m_dead)
    {
        if (key == 'r' || key == 'R')
            init_game();
        return ScenesGame::Snake;
    }

    if (key == 'p' || key == 'P')
    {
        m_paused = !m_paused;
        return ScenesGame::Snake;
    }

    // Direction (prevent 180-degree reversal)
    switch (key)
    {
        case TB_KEY_ARROW_UP:
            if (m_dir != SnakeDir::Down)
                m_next_dir = SnakeDir::Up;
            break;
        case TB_KEY_ARROW_DOWN:
            if (m_dir != SnakeDir::Up)
                m_next_dir = SnakeDir::Down;
            break;
        case TB_KEY_ARROW_LEFT:
            if (m_dir != SnakeDir::Right)
                m_next_dir = SnakeDir::Left;
            break;
        case TB_KEY_ARROW_RIGHT:
            if (m_dir != SnakeDir::Left)
                m_next_dir = SnakeDir::Right;
            break;
        default: break;
    }

    return ScenesGame::Snake;
}

void SnakeGame::init_game()
{
    // ~75% of the terminal, centred.
    // Inner playfield is (board_w-2) × (board_h-2) after the border.
    m_board_w = std::max(10, (display.getWidth() * 3) / 4);
    m_board_h = std::max(8, (display.getHeight() * 3) / 4);

    m_board_x = (display.getWidth() - m_board_w) / 2;
    m_board_y = (display.getHeight() - m_board_h) / 2;

    if (settings.general.utf8)
    {
        CH_SNAKE_HEAD = U'◉';
        CH_SNAKE_BODY = U'█';
        CH_FOOD       = U'●';
        CH_BORDER_H   = U'═';
        CH_BORDER_V   = U'║';
        CH_CORNER_TL  = U'╔';
        CH_CORNER_TR  = U'╗';
        CH_CORNER_BL  = U'╚';
        CH_CORNER_BR  = U'╝';
    }
    else
    {
        CH_SNAKE_HEAD = 'O';
        CH_SNAKE_BODY = '#';
        CH_FOOD       = '*';
        CH_BORDER_H   = '-';
        CH_BORDER_V   = '|';
        CH_CORNER_TL  = '+';
        CH_CORNER_TR  = '+';
        CH_CORNER_BL  = '+';
        CH_CORNER_BR  = '+';
    }

    m_snake.clear();
    m_score    = 0;
    m_dead     = false;
    m_paused   = false;
    m_dir      = SnakeDir::Right;
    m_next_dir = SnakeDir::Right;
    m_speed_ms = static_cast<int>(settings.game_snake.snake_max_speed);

    // Start with a 3-segment snake centred in the playfield
    const int sx = m_board_x + m_board_w / 2;
    const int sy = m_board_y + m_board_h / 2;
    m_snake.push_back({ sx, sy });
    m_snake.push_back({ sx - 1, sy });
    m_snake.push_back({ sx - 2, sy });

    spawn_food();
}

void SnakeGame::update()
{
    if (m_dead || m_paused)
        return;

    m_dir = m_next_dir;

    // Compute new head position
    Point head = m_snake.front();
    switch (m_dir)
    {
        case SnakeDir::Up:    head.y--; break;
        case SnakeDir::Down:  head.y++; break;
        case SnakeDir::Left:  head.x--; break;
        case SnakeDir::Right: head.x++; break;
    }

    // Wall collision (border cells are at the edge, playfield is 1 inside)
    const int inner_x0 = m_board_x + 1;
    const int inner_y0 = m_board_y + 1;
    const int inner_x1 = m_board_x + m_board_w - 2;
    const int inner_y1 = m_board_y + m_board_h - 2;

    if (head.x < inner_x0 || head.x > inner_x1 || head.y < inner_y0 || head.y > inner_y1)
    {
        m_dead = true;
        return;
    }

    // Self collision (skip the tail tip - it will move away this tick)
    for (auto it = m_snake.begin(); it != std::prev(m_snake.end()); ++it)
    {
        if (*it == head)
        {
            m_dead = true;
            return;
        }
    }

    m_snake.push_front(head);

    if (head == m_food)
    {
        // Grow (don't pop tail)
        m_score++;

        // Speed up every SPEED_MILESTONE points
        if (m_score % SPEED_MILESTONE == 0)
            m_speed_ms = std::max(static_cast<int>(settings.game_snake.snake_min_speed), m_speed_ms - SPEED_STEP_MS);

        spawn_food();
        playback.playSfx(SnakeSounds::FOOD);
    }
    else
    {
        m_snake.pop_back();
    }
}

void SnakeGame::spawn_food()
{
    const int inner_x0 = m_board_x + 1;
    const int inner_y0 = m_board_y + 1;
    const int inner_x1 = m_board_x + m_board_w - 2;
    const int inner_y1 = m_board_y + m_board_h - 2;

    // Bias food toward the snake head so it never spawns unreachably far away.
    // Radius scales with the board: ~25% of the smaller dimension, min 4 cells.
    const int radius = std::max(4, std::min(inner_x1 - inner_x0, inner_y1 - inner_y0) / 4);

    const Point& head = m_snake.front();

    const int wx0 = std::max(inner_x0, head.x - radius);
    const int wx1 = std::min(inner_x1, head.x + radius);
    const int wy0 = std::max(inner_y0, head.y - radius);
    const int wy1 = std::min(inner_y1, head.y + radius);

    std::uniform_int_distribution<int> rx_near(wx0, wx1);
    std::uniform_int_distribution<int> ry_near(wy0, wy1);
    std::uniform_int_distribution<int> rx_full(inner_x0, inner_x1);
    std::uniform_int_distribution<int> ry_full(inner_y0, inner_y1);

    auto occupied = [&](const Point& p) {
        return std::any_of(m_snake.begin(), m_snake.end(), [&](const Point& s) { return s == p; });
    };

    // Try up to 16 times inside the proximity window first
    for (int i = 0; i < 16; ++i)
    {
        Point candidate{ rx_near(m_rng), ry_near(m_rng) };
        if (!occupied(candidate))
        {
            m_food = candidate;
            return;
        }
    }

    // Fallback: full-board random (snake may be very long)
    Point candidate;
    do
    {
        candidate = { rx_full(m_rng), ry_full(m_rng) };
    } while (occupied(candidate));

    m_food = candidate;
}

void SnakeGame::draw_border()
{
    display.setTextColor(COL_BORDER);

    const int x = m_board_x;
    const int y = m_board_y;
    const int w = m_board_w;
    const int h = m_board_h;

    // Horizontal edges
    for (int col = x + 1; col < x + w - 1; ++col)
    {
        display.drawPixel(col, y, CH_BORDER_H);
        display.drawPixel(col, y + h - 1, CH_BORDER_H);
    }

    // Vertical edges
    for (int row = y + 1; row < y + h - 1; ++row)
    {
        display.drawPixel(x, row, CH_BORDER_V);
        display.drawPixel(x + w - 1, row, CH_BORDER_V);
    }

    // Corners
    display.drawPixel(x, y, CH_CORNER_TL);
    display.drawPixel(x + w - 1, y, CH_CORNER_TR);
    display.drawPixel(x, y + h - 1, CH_CORNER_BL);
    display.drawPixel(x + w - 1, y + h - 1, CH_CORNER_BR);
}

void SnakeGame::draw_hud()
{
    display.setTextColor(COL_HUD);

    // Score line above the board (or at row 0 if board is near the top)
    const int hud_y = std::max(0, m_board_y - 2);
    display.setCursor(m_board_x, hud_y);
    display.print(" Score: {}   Length: {} ", m_score, m_snake.size());
}

void SnakeGame::draw_game_over()
{
    display.setTextColor(COL_GAMEOVER);
    const int mid_y = m_board_y + m_board_h / 2;

    // clang-format off
    if (settings.general.utf8)
    {
        display.centerText(mid_y - 2, "╔══════════════════╗");
        display.centerText(mid_y - 1, "║    GAME  OVER    ║");
        display.centerText(mid_y,     "║  Score:  {:>6}  ║", m_score);
        display.centerText(mid_y + 1, "║  Length: {:>6}  ║", m_snake.size());
        display.centerText(mid_y + 2, "╚══════════════════╝");
    }
    else
    {
        display.centerText(mid_y - 2, "+------------------+");
        display.centerText(mid_y - 1, "|    GAME  OVER    |");
        display.centerText(mid_y,     "|  Score:  {:>6}  |", m_score);
        display.centerText(mid_y + 1, "|  Length: {:>6}  |", m_snake.size());
        display.centerText(mid_y + 2, "+------------------+");
    }
    // clang-format on

    display.setTextColor(TB_WHITE);
    display.centerText(mid_y + 4, "R: Restart   ESC: Menu");
}

void SnakeGame::draw_paused()
{
    display.setTextColor(COL_PAUSED);
    const int mid_y = m_board_y + m_board_h / 2;

    // clang-format off
    if (settings.general.utf8)
    {
        display.centerText(mid_y - 1, "╔════════════╗");
        display.centerText(mid_y,     "║   PAUSED   ║");
        display.centerText(mid_y + 1, "╚════════════╝");
    }
    else
    {
        display.centerText(mid_y - 1, "+------------+");
        display.centerText(mid_y,     "|   PAUSED   |");
        display.centerText(mid_y + 1, "+------------+");
    }
    // clang-format on
}
