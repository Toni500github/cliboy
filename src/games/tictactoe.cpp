#include "games/tictactoe.hpp"

#include <thread>

#include "settings.hpp"
#include "terminal_display.hpp"

// Tic-Tac-Toe board lines
// These will be calculated dynamically in TTTScene constructor
static int BOARD_SIZE     = 0;
static int CELL_SIZE      = 0;
static int BOARD_OFFSET_X = 0;
static int BOARD_OFFSET_Y = 0;

bool TTTGame::isBoardFull()
{
    return !iterateBoard([](char& c, int, int) -> bool { return c == ' '; });
}

void TTTGame::drawPiece(int row, int col, char piece)
{
    int x = BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 4;
    int y = BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 4;

    // draw cursor marker
    int cx = BOARD_OFFSET_X + m_cursor_x * CELL_SIZE + CELL_SIZE / 2;
    int cy = BOARD_OFFSET_Y + m_cursor_y * CELL_SIZE + CELL_SIZE / 2;
    display.setCursor(cx, cy);
    display.print("*");

    display.setTextBgColor(TB_WHITE);
    switch (piece)
    {
        case 'X':
            display.drawLine(x, y, x + CELL_SIZE / 2, y + CELL_SIZE / 2, ' ');
            display.drawLine(x + CELL_SIZE / 2, y, x, y + CELL_SIZE / 2, ' ');
            break;
        case 'O': display.drawCircle(x + CELL_SIZE / 4, y + CELL_SIZE / 4, CELL_SIZE / 4, ' '); break;
    }
    display.resetColors();
}

void TTTGame::drawGameScreen()
{
    display.setTextBgColor(TB_WHITE);

    // Vertical lines
    display.drawLine(
        BOARD_OFFSET_X + CELL_SIZE, BOARD_OFFSET_Y, BOARD_OFFSET_X + CELL_SIZE, BOARD_OFFSET_Y + BOARD_SIZE, ' ');

    display.drawLine(BOARD_OFFSET_X + 2 * CELL_SIZE,
                     BOARD_OFFSET_Y,
                     BOARD_OFFSET_X + 2 * CELL_SIZE,
                     BOARD_OFFSET_Y + BOARD_SIZE,
                     ' ');

    // Horizontal lines
    display.drawLine(
        BOARD_OFFSET_X, BOARD_OFFSET_Y + CELL_SIZE, BOARD_OFFSET_X + BOARD_SIZE, BOARD_OFFSET_Y + CELL_SIZE, ' ');

    display.drawLine(BOARD_OFFSET_X,
                     BOARD_OFFSET_Y + 2 * CELL_SIZE,
                     BOARD_OFFSET_X + BOARD_SIZE,
                     BOARD_OFFSET_Y + 2 * CELL_SIZE,
                     ' ');

    display.resetColors();

    iterateBoard([&](char& c, int r, int col) {
        if (c != ' ')
            drawPiece(r, col, c);
    });

    const int cx = BOARD_OFFSET_X + m_cursor_x * CELL_SIZE + CELL_SIZE / 2;
    const int cy = BOARD_OFFSET_Y + m_cursor_y * CELL_SIZE + CELL_SIZE / 2;
    display.setCursor(cx, cy);
    display.print("*");

    // Place player indicator in the left margin, vertically centered on the board
    display.setCursor(BOARD_OFFSET_X / 4, BOARD_OFFSET_Y + BOARD_SIZE / 2 - 2);
    display.setFont(FigletType::FullWidth, "Soft");
    display.print("{}", static_cast<char>(m_current_player));
    display.resetFont();

    display.display();
}

void TTTGame::animateLine(int x0, int y0, int x1, int y1)
{
    const int steps = 16;  // smoothness
    for (int i = 1; i <= steps; ++i)
    {
        int xi = x0 + (x1 - x0) * i / steps;
        int yi = y0 + (y1 - y0) * i / steps;
        drawGameScreen();  // redraw board and pieces
        display.setTextBgColor(TB_WHITE);
        display.drawLine(x0, y0, xi, yi, ' ');
        display.display();
        sleep_for(duration<float>(settings.game_ttt.delay_strike_anim));
    }
    display.resetColors();
}

Player TTTGame::checkWinner()
{
    // check rows
    for (uint8_t row = 0; row < 3; ++row)
        if (m_board[row][0] != ' ' && m_board[row][0] == m_board[row][1] && m_board[row][1] == m_board[row][2])
        {
            animateLine(BOARD_OFFSET_X,
                        BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2,
                        BOARD_OFFSET_X + BOARD_SIZE,
                        BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2);
            return (Player)m_board[row][0];
        }

    // check columns
    for (uint8_t col = 0; col < 3; ++col)
        if (m_board[0][col] != ' ' && m_board[0][col] == m_board[1][col] && m_board[1][col] == m_board[2][col])
        {
            animateLine(BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2,
                        BOARD_OFFSET_Y,
                        BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2,
                        BOARD_OFFSET_Y + BOARD_SIZE);
            return (Player)m_board[0][col];
        }

    // check diagonals
    if (m_board[0][0] != ' ' && m_board[0][0] == m_board[1][1] && m_board[1][1] == m_board[2][2])
    {
        animateLine(BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_OFFSET_X + BOARD_SIZE, BOARD_OFFSET_Y + BOARD_SIZE);
        return (Player)m_board[0][0];
    }

    if (m_board[0][2] != ' ' && m_board[0][2] == m_board[1][1] && m_board[1][1] == m_board[2][0])
    {
        animateLine(BOARD_OFFSET_X + BOARD_SIZE, BOARD_OFFSET_Y, BOARD_OFFSET_X, BOARD_OFFSET_Y + BOARD_SIZE);
        return (Player)m_board[0][2];
    }

    return Player::None;
}

void TTTGame::drawWinner(Player winner)
{
    display.clearDisplay();
    display.setFont(FigletType::FullWidth, "starwars");
    display.centerText(display.pctY(0.10f), "Player");
    display.centerText(display.getCursorY() + 4, winner == Player::X ? "X" : "O");
    display.centerText(display.getCursorY() + 3, "Wins");

    display.resetFont();
    display.display();
}

void TTTGame::resetGame()
{
    display.clearDisplay();
    display.resetColors();
    display.resetFont();

    iterateBoard([](char& c, int, int) { c = ' '; });

    m_moves          = 0;
    m_cursor_y       = 0;
    m_cursor_x       = 0;
    m_old_pos_x      = 0;
    m_old_pos_y      = 0;
    m_choose_pos     = false;
    m_current_player = Player::X;
}

void TTTGame::render()
{
    display.clearDisplay();

    {
        // board size = 67% of the smaller terminal dimension, snapped to a multiple of 3
        BOARD_SIZE = (std::min(display.pctX(0.67f), display.pctY(0.67f)) / 3) * 3;
        CELL_SIZE  = BOARD_SIZE / 3;

        // center horizontally, sit at 33% from top vertically
        BOARD_OFFSET_X = (display.getWidth() - BOARD_SIZE) / 2;
        BOARD_OFFSET_Y = display.pctY(0.1f);
    }

    Player player_to_place = (m_moves % 2 == 0) ? Player::X : Player::O;

    if (m_choose_pos)
    {
        if (m_board[m_cursor_y][m_cursor_x] == ' ')
        {
            m_board[m_cursor_y][m_cursor_x] = static_cast<char>(player_to_place);
            m_moves++;
            player_to_place = (m_moves % 2 == 0) ? Player::X : Player::O;
        }
        m_choose_pos = false;
    }

    m_current_player = player_to_place;

    drawGameScreen();

    const Player winner = checkWinner();
    if (winner != Player::None)
    {
        drawWinner(winner);
        display.display();
        sleep_for(duration<float>(settings.game_ttt.delay_show_endgame));
        resetGame();
        render();
        return;
    }

    if (isBoardFull())
    {
        sleep_for(500ms);
        display.clearDisplay();
        display.setFont(FigletType::Kerning, "starwars");
        display.centerText(display.pctY(0.50f), "Board Full");
        display.resetFont();
        display.display();
        sleep_for(duration<float>(settings.game_ttt.delay_show_endgame));
        resetGame();
        render();
        return;
    }

    display.display();
}

SceneResult TTTGame::handleInput(uint32_t key)
{
    switch (key)
    {
        case TB_KEY_ESC: return Scenes::GamesMenu;

        case TB_KEY_ARROW_DOWN:
            if (m_cursor_y < 2)
                m_cursor_y++;
            break;

        case TB_KEY_ARROW_UP:
            if (m_cursor_y > 0)
                m_cursor_y--;
            break;

        case TB_KEY_ARROW_RIGHT:
            if (m_cursor_x < 2)
                m_cursor_x++;
            break;

        case TB_KEY_ARROW_LEFT:
            if (m_cursor_x > 0)
                m_cursor_x--;
            break;

        case TB_KEY_ENTER:
        case '\n':         m_choose_pos = true; break;
    }

    return sceneID();
}
