#include <thread>

#include "terminal_display.hpp"
#include "util.hpp"

// I'm not singing for your XO... I'm singing cuz it's oveeeerrrrr
enum Player
{
    NO_PLAYER = 0,
    X_PLAYER  = 'X',
    O_PLAYER  = 'O',
};

static char board[3][3]   = { { ' ', ' ', ' ' }, { ' ', ' ', ' ' }, { ' ', ' ', ' ' } };
static bool choose_pos    = false;
static char currentPlayer = X_PLAYER;
static int  currentPosX, oldPosX, cursorX = 0;
static int  currentPosY, oldPosY, cursorY = 0;
static int  moves = 0;

// Tic-Tac-Toe board lines
// These will be calculated dynamically in play_multip_ttt()
static int BOARD_SIZE     = 0;
static int CELL_SIZE      = 0;
static int BOARD_OFFSET_X = 0;
static int BOARD_OFFSET_Y = 0;

static bool is_board_full()
{
    for (uint8_t row = 0; row < 3; ++row)
        for (uint8_t col = 0; col < 3; ++col)
            if (board[row][col] == ' ' || board[row][col] == '*')
                return false;
    return true;
}

void draw_piece(int row, int col, char piece)
{
    int x = BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 4;
    int y = BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 4;

    // draw cursor marker
    int cx = BOARD_OFFSET_X + cursorX * CELL_SIZE + CELL_SIZE / 2;
    int cy = BOARD_OFFSET_Y + cursorY * CELL_SIZE + CELL_SIZE / 2;
    display.setCursor(cx, cy);
    display.print("*");

    switch (piece)
    {
        case 'X':
            display.drawLine(x, y, x + CELL_SIZE / 2, y + CELL_SIZE / 2);
            display.drawLine(x + CELL_SIZE / 2, y, x, y + CELL_SIZE / 2);
            break;
        case 'O': display.drawCircle(x + CELL_SIZE / 4, y + CELL_SIZE / 4, CELL_SIZE / 4); break;
    }
}

void draw_game_screen()
{
    display.clearDisplay();

    // Vertical lines
    display.drawLine(BOARD_OFFSET_X + CELL_SIZE, BOARD_OFFSET_Y, BOARD_OFFSET_X + CELL_SIZE,
                     BOARD_OFFSET_Y + BOARD_SIZE);
    display.drawLine(BOARD_OFFSET_X + 2 * CELL_SIZE, BOARD_OFFSET_Y, BOARD_OFFSET_X + 2 * CELL_SIZE,
                     BOARD_OFFSET_Y + BOARD_SIZE);

    // Horizontal lines
    display.drawLine(BOARD_OFFSET_X, BOARD_OFFSET_Y + CELL_SIZE, BOARD_OFFSET_X + BOARD_SIZE,
                     BOARD_OFFSET_Y + CELL_SIZE);
    display.drawLine(BOARD_OFFSET_X, BOARD_OFFSET_Y + 2 * CELL_SIZE, BOARD_OFFSET_X + BOARD_SIZE,
                     BOARD_OFFSET_Y + 2 * CELL_SIZE);

    for (uint8_t row = 0; row < 3; ++row)
        for (uint8_t col = 0; col < 3; ++col)
            if (board[row][col] != ' ')
                draw_piece(row, col, board[row][col]);

    display.setCursor(15, 10);
    display.setFont(FIGLET_FULL_WIDTH, "Soft");
    display.print("{}", currentPlayer);
    display.resetFont();

    display.centerText(display.getHeight() * 0.9,
                       "Up: {:c} || Left: {:c} || Down: {:c} || Right: {:c} || Place: ENTER || Exit: ESC",
                       settings.ch_up, settings.ch_left, settings.ch_down, settings.ch_right);

    display.display();
}

void animate_line(int x0, int y0, int x1, int y1)
{
    const int steps = 16;  // smoothness
    for (int i = 1; i <= steps; ++i)
    {
        int xi = x0 + (x1 - x0) * i / steps;
        int yi = y0 + (y1 - y0) * i / steps;
        draw_game_screen();  // redraw board and pieces
        display.drawLine(x0, y0, xi, yi);
        display.display();
        std::this_thread::sleep_for(50ms);
    }
}

Player check_winner()
{
    // check rows
    for (uint8_t row = 0; row < 3; ++row)
        if (board[row][0] != ' ' && board[row][0] == board[row][1] && board[row][1] == board[row][2])
        {
            animate_line(BOARD_OFFSET_X, BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2, BOARD_OFFSET_X + BOARD_SIZE,
                         BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2);
            return (Player)board[row][0];
        }

    // check columns
    for (uint8_t col = 0; col < 3; ++col)
        if (board[0][col] != ' ' && board[0][col] == board[1][col] && board[1][col] == board[2][col])
        {
            animate_line(BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2, BOARD_OFFSET_Y,
                         BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2, BOARD_OFFSET_Y + BOARD_SIZE);
            return (Player)board[0][col];
        }

    // check diagonals
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2])
    {
        animate_line(BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_OFFSET_X + BOARD_SIZE, BOARD_OFFSET_Y + BOARD_SIZE);
        return (Player)board[0][0];
    }

    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0])
    {
        animate_line(BOARD_OFFSET_X + BOARD_SIZE, BOARD_OFFSET_Y, BOARD_OFFSET_X, BOARD_OFFSET_Y + BOARD_SIZE);
        return (Player)board[0][2];
    }

    return NO_PLAYER;
}

void draw_winner(Player winner)
{
    display.clearDisplay();
    display.setFont(FIGLET_FULL_WIDTH, "starwars");
    display.centerText(5, "Player");
    display.centerText(display.getCursorY() + 4, winner == X_PLAYER ? "X" : "O");
    display.centerText(display.getCursorY() + 3, "Wins");

    display.resetFont();
    display.display();
}

void reset_game()
{
    display.clearDisplay();
    display.resetFont();

    for (uint8_t row = 0; row < 3; ++row)
        for (uint8_t col = 0; col < 3; ++col)
            board[row][col] = ' ';

    choose_pos = moves = currentPosY = currentPosX = oldPosY = oldPosX = 0;
    currentPlayer                                                      = X_PLAYER;
}

void play_multip_ttt()
{
    reset_game();

    // board size should be about 2/3 of the smaller dimension
    BOARD_SIZE = std::min(display.getWidth(), display.getHeight()) * 2 / 3;

    // ensure BOARD_SIZE is divisible by 3 for clean cell boundaries
    BOARD_SIZE = (BOARD_SIZE / 3) * 3;

    CELL_SIZE = BOARD_SIZE / 3;

    // center the board
    BOARD_OFFSET_X = (display.getWidth() - BOARD_SIZE) / 2;
    BOARD_OFFSET_Y = (display.getHeight() - BOARD_SIZE) / 3;  // slightly higher than center

    std::this_thread::sleep_for(200ms);

    while (true)
    {
        if (is_board_full())
        {
            std::this_thread::sleep_for(100ms);
            display.clearDisplay();
            display.setFont(FIGLET_KERNING, "starwars");
            display.centerText(display.getHeight() / 2, "Board Full");
            display.resetFont();
            display.display();
            std::this_thread::sleep_for(2s);
            reset_game();
            continue;
        }

        choose_pos    = false;
        oldPosX       = currentPosX;
        oldPosY       = currentPosY;
        currentPlayer = (moves % 2 == 0) ? X_PLAYER : O_PLAYER;

        update_button();
        if (button_state & KEY_QUIT)
        {
            reset_to_main_menu();
            return;
        }

        if (button_state & KEY_DOWN_BIT)
        {
            if (currentPosY < 2)
                currentPosY++;
        }
        if (button_state & KEY_UP_BIT)
        {
            if (currentPosY > 0)
                currentPosY--;
        }
        if (button_state & KEY_RIGHT_BIT)
        {
            if (currentPosX < 2)
                ++currentPosX;
        }
        if (button_state & KEY_LEFT_BIT)
        {
            if (currentPosX > 0)
                --currentPosX;
        }

        if (button_state & KEY_SELECTED)
        {
            choose_pos = true;
        }

        std::this_thread::sleep_for(50ms);
        cursorX = currentPosX;
        cursorY = currentPosY;

        if (board[currentPosY][currentPosX] == ' ')
            board[currentPosY][currentPosX] = '*';

        draw_game_screen();

        if (choose_pos && board[currentPosY][currentPosX] == '*')
        {
            board[currentPosY][currentPosX] = currentPlayer;
            // currentPosX = currentPosY = 0;
            ++moves;
        }
        else
        {
            if (board[oldPosX][oldPosY] == '*')
                board[oldPosX][oldPosY] = ' ';
            continue;
        }

        Player winner = check_winner();
        if (winner != NO_PLAYER)
        {
            draw_winner(winner);
            std::this_thread::sleep_for(3s);
            reset_game();
        }
    }
}
