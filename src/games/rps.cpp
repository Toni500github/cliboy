#include <notcurses/nckeys.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <thread>

#include "games/rockpaperscissors.hpp"
#include "scenes.hpp"
#include "terminal_display.hpp"

using namespace std::chrono_literals;

enum Winner
{
    CPU = 0,
    DRAW,
    PLAYER
};

enum Moves
{
    NONE = 10,
    ROCK,
    PAPER,
    SCISSORS,
    DONE
};

Moves player_move = NONE;
bool  selected    = false;
bool  restart     = false;

static void print_count_down(uint8_t n)
{
    display.clearDisplay();
    char str[16];
    sprintf(str, "%hu", n);
    display.centerText(display.getHeight() / 2, str);
    display.display();
    std::this_thread::sleep_for(850ms);
}

static std::string get_move_ascii(Moves whos_move)
{
    switch (whos_move)
    {
        case ROCK:     return "Rock";
        case PAPER:    return "Paper";
        case SCISSORS: return "Scissors";
        default:       return "";
    }
}

static Moves get_cpu_move()
{
    int random = rand() % 3;
    switch (random)
    {
        case 0:  return ROCK;
        case 1:  return PAPER;
        case 2:  return SCISSORS;
        default: return ROCK;
    }
}

static void print_winner(Winner winner)
{
    // display.clearDisplay();
    display.setFont(FIGLET_FULL_WIDTH, "starwars");
    switch (winner)
    {
        case CPU:
            display.setTextColor(0xff0000);
            display.centerText(display.getHeight() / 10, "CPU Wins");
            break;
        case DRAW:
            display.setTextColor(0x9370DB);
            display.centerText(display.getHeight() / 10, "Draw");
            break;
        case PLAYER:
            display.setTextColor(0x00ff00);
            display.centerText(display.getHeight() / 10, "You Won");
            break;
    }

    display.resetFont();
    display.display();
}

static Winner calculate_winner(Moves cpu_move, Moves player_move)
{
    if (player_move == cpu_move)
        return DRAW;

    if ((cpu_move == ROCK && player_move == SCISSORS) || (cpu_move == SCISSORS && player_move == PAPER) ||
        (cpu_move == PAPER && player_move == ROCK))
        return CPU;

    return PLAYER;
}

static void print_player_move(Moves player_move)
{
    display.clearDisplay();
    display.centerText(display.getHeight() / 2.5, "Your move:");
    display.setFont(FIGLET_FULL_WIDTH, "Ogre");
    display.centerText(display.getCursorY() + 2, "{}", get_move_ascii(player_move));
    display.resetFont();
    display.centerText(display.getHeight() * 0.9, "Rock: r | Paper: p | Scissors: s | Play: ENTER | Exit: ESC");

    display.display();
}

static void print_moves(Moves computer_move, Moves player_move)
{
    display.clearDisplay();
    display.setFont(FIGLET_SMUSHED, "Doom");

    const int term_width  = display.getWidth();
    const int term_height = display.getHeight();

    int left_col  = term_width * 0.05;  // 5% from left
    int right_col = term_width * 0.8;   // 80% from left
    int header_y  = term_height * 0.4;  // 40% from top
    int moves_y   = term_height * 0.6;  // 60% from top

    display.setCursor(left_col + 3, header_y);
    display.print("You");

    display.setCursor(right_col - 3, header_y);
    display.print("CPU");

    display.setCursor(left_col, moves_y);
    display.print("{}", get_move_ascii(player_move));

    display.centerText(moves_y - 8, "VS");

    display.setCursor(right_col - 8, moves_y);
    display.print("{}", get_move_ascii(computer_move));

    display.display();
}

void RpsScene::render()
{
    display.clearDisplay();

    print_player_move(player_move);
    if (player_move == NONE || !selected)
        return;

    Moves computer_move = get_cpu_move();
    display.setFont(FIGLET_FULL_WIDTH, "Stop");
    print_count_down(3);
    print_count_down(2);
    print_count_down(1);
    Winner winner = calculate_winner(computer_move, player_move);
    print_moves(computer_move, player_move);
    print_winner(winner);

    display.display();

    std::this_thread::sleep_for(2s);

    display.clearDisplay();
    print_player_move(NONE);
    selected = false;
}

SceneResult RpsScene::handle_input(uint32_t key)
{
    switch (key)
    {
        case NCKEY_ESC: return Scenes::Games;
        case 'r':       player_move = ROCK; break;
        case 'p':       player_move = PAPER; break;
        case 's':       player_move = SCISSORS; break;

        case NCKEY_ENTER:
        case '\n':
        case '\r':
            if (player_move != NONE)
                selected = true;
            break;
    }
    return ScenesGame::RockPaperScissors;
}
