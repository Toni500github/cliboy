#include <cstdint>
#include <cstdlib>
#include <thread>

#include "terminal_display.hpp"
#include "util.hpp"

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
    display.clearDisplay();
    switch (winner)
    {
        case CPU:    display.centerText(display.getHeight() / 2, "CPU Wins"); break;
        case DRAW:   display.centerText(display.getHeight() / 2, "Draw!"); break;
        case PLAYER: display.centerText(display.getHeight() / 2, "Player Wins!"); break;
    }

    display.display();
    std::this_thread::sleep_for(1.5s);
}

static Winner calculate_winner(Moves cpu_move, Moves player_move)
{
    if (player_move == cpu_move)
        return DRAW;

    if ((cpu_move == ROCK && player_move == SCISSORS) ||
        (cpu_move == SCISSORS && player_move == PAPER) ||
        (cpu_move == PAPER && player_move == ROCK))
        return CPU;

    return PLAYER;
}

static void print_player_move(Moves player_move)
{
    display.clearDisplay();
    display.centerText((display.getHeight() + 5) / 2, "Your move: {}", get_move_ascii(player_move));
    display.centerText(display.getHeight() * 0.9, "Rock: {:c} || Paper: {:c} || Scissors: {:c} || Play: ENTER", settings.ch_up,
                       settings.ch_left, settings.ch_down);

    display.display();
}

static void print_moves(Moves computer_move, Moves player_move)
{
    display.clearDisplay();
    display.setCursor(display.getWidth() * 0.4, display.getHeight() * 0.4);
    display.print("You");
    display.setCursor(display.getWidth() * 0.55, display.getHeight() * 0.4);
    display.print("CPU");

    display.setCursor(display.getWidth() * 0.4, display.getCursorY() + 5);
    display.print("{}", get_move_ascii(player_move));

    display.centerText(display.getCursorY(), "VS");

    display.setCursor(display.getWidth() * 0.55, display.getCursorY());
    display.print("{}", get_move_ascii(computer_move));

    display.display();
    std::this_thread::sleep_for(1.5s);
}

void play_singlep_rps()
{
    while (true)
    {
        std::this_thread::sleep_for(33ms);
        Moves player_move = NONE;

        while (true)
        {
            update_button();
            if (button_state & KEY_QUIT)
            {
                reset_to_main_menu();
                return;
            }

            if (!(button_state & KEY_SELECTED))
            {
                if (button_state & KEY_UP_BIT)
                    player_move = ROCK;
                else if (button_state & KEY_LEFT_BIT)
                    player_move = PAPER;
                else if (button_state & KEY_DOWN_BIT)
                    player_move = SCISSORS;
            }
            else if (button_state & KEY_SELECTED && player_move != NONE)
            {
                break;
            }
            print_player_move(player_move);
            std::this_thread::sleep_for(33ms);
        }

        Moves computer_move = get_cpu_move();
        print_count_down(3);
        print_count_down(2);
        print_count_down(1);
        Winner winner = calculate_winner(computer_move, player_move);
        print_moves(computer_move, player_move);
        print_winner(winner);
    }
}
