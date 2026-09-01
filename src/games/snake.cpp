#include "games/snake.hpp"

#include <algorithm>
#include <string>

// Colors
enum Colors : uintattr_t
{
    Border    = TB_WHITE,
    SnakeHead = TB_GREEN | TB_BOLD,
    SnakeBody = TB_GREEN,
    Food      = TB_RED | TB_BOLD,
    Hud       = TB_CYAN | TB_BOLD,
};

// Speed
static constexpr int SPEED_STEP_MS   = 10;  // ms reduction per milestone
static constexpr int SPEED_MILESTONE = 5;   // points between speed-ups

Result<> SnakeGame::onGameBegin()
{
    setFooter("Arrows: Move | P: Pause | ESC: Back");
    return Ok();
}

void SnakeGame::renderGame()
{
    update();

    drawBorder();
    drawHud();

    if (isGameOver())
    {
        drawGameOverOverlay({
            { "Score", std::to_string(score()) },
            { "Length", std::to_string(m_snake.size()) },
        });
        return;
    }
    else if (isPaused())
        drawPausedOverlay();
    else if (!playback.isMusicPlaying())
        playback.playMusic(MenuSounds::BGM, true);

    // Food
    display.setTextColor(Colors::Food);
    display.drawPixel(m_food.x, m_food.y, characters.FOOD);

    // Snake (head first so its character sits on top)
    bool is_head = true;
    for (const auto& seg : m_snake)
    {
        display.setTextColor(is_head ? Colors::SnakeHead : Colors::SnakeBody);
        display.drawPixel(seg.x, seg.y, is_head ? characters.SNAKE_HEAD : characters.SNAKE_BODY);
        is_head = false;
    }
}

void SnakeGame::initGame()
{
    // ~75% of the terminal, centred.
    // Inner playfield is (board_w-2) × (board_h-2) after the border.
    m_board_w = std::max(10, (display.getWidth() * 3) / 4);
    m_board_h = std::max(8, (display.getHeight() * 3) / 4);

    m_board_x = (display.getWidth() - m_board_w) / 2;
    m_board_y = (display.getHeight() - m_board_h) / 2;

    m_snake.clear();
    resetScore();

    m_dir      = SnakeDir::Right;
    m_next_dir = SnakeDir::Right;
    m_speed_ms = static_cast<int>(settings.game_snake.snake_max_speed);

    // Start with a 3-segment snake centred in the playfield
    const int sx = m_board_x + m_board_w / 2;
    const int sy = m_board_y + m_board_h / 2;
    m_snake.push_back({ sx, sy });
    m_snake.push_back({ sx - 1, sy });
    m_snake.push_back({ sx - 2, sy });

    spawnFood();
}

void SnakeGame::update()
{
    if (isGameOver() || isPaused())
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
        setGameState(GameState::GameOver);
        return;
    }

    // Self collision (skip the tail tip - it will move away this tick)
    for (auto it = m_snake.begin(); it != std::prev(m_snake.end()); ++it)
    {
        if (*it == head)
        {
            setGameState(GameState::GameOver);
            return;
        }
    }

    m_snake.push_front(head);

    if (head == m_food)
    {
        // Grow (don't pop tail)
        addScore(1);

        // Speed up every SPEED_MILESTONE points
        if (score() % SPEED_MILESTONE == 0)
            m_speed_ms = std::max(static_cast<int>(settings.game_snake.snake_min_speed), m_speed_ms - SPEED_STEP_MS);

        spawnFood();
        playback.playSfx(SnakeSounds::FOOD);
    }
    else
    {
        m_snake.pop_back();
    }
}

void SnakeGame::spawnFood()
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

void SnakeGame::drawBorder()
{
    display.setTextColor(Colors::Border);

    const int x = m_board_x;
    const int y = m_board_y;
    const int w = m_board_w;
    const int h = m_board_h;

    // Horizontal edges
    for (int col = x + 1; col < x + w - 1; ++col)
    {
        display.drawPixel(col, y, borders.H);
        display.drawPixel(col, y + h - 1, borders.H);
    }

    // Vertical edges
    for (int row = y + 1; row < y + h - 1; ++row)
    {
        display.drawPixel(x, row, borders.V);
        display.drawPixel(x + w - 1, row, borders.V);
    }

    // Corners
    display.drawPixel(x, y, corners.TL);
    display.drawPixel(x + w - 1, y, corners.TR);
    display.drawPixel(x, y + h - 1, corners.BL);
    display.drawPixel(x + w - 1, y + h - 1, corners.BR);
}

void SnakeGame::drawHud()
{
    display.setTextColor(Colors::Hud);

    // Score line above the board (or at row 0 if board is near the top)
    const int hud_y = std::max(0, m_board_y - 2);
    display.setCursor(m_board_x, hud_y);
    display.print(" Score: {}   Length: {} ", score(), m_snake.size());
}

SceneResult SnakeGame::handleInput(uint32_t key)
{
    if (auto r = handleCommonInput(key))
        return *r;
    if (isGameOver())
        return sceneID();

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

    return sceneID();
}
