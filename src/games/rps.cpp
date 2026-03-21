#include <cstdint>
#include <random>

#include "games/rockpaperscissors.hpp"
#include "scenes.hpp"
#include "settings.hpp"
#include "terminal_display.hpp"

std::string RpsGame::get_move_str(Moves move)
{
    switch (move)
    {
        case Moves::Rock:     return "Rock";
        case Moves::Paper:    return "Paper";
        case Moves::Scissors: return "Scissors";
        default:              return "";
    }
}

Moves RpsGame::get_cpu_move()
{
    static std::mt19937                rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, 2);
    switch (dist(rng))
    {
        case 0:  return Moves::Rock;
        case 1:  return Moves::Paper;
        case 2:  return Moves::Scissors;
        default: return Moves::Rock;
    }
}

Winner RpsGame::calculate_winner(Moves cpu_move, Moves player_move)
{
    if (player_move == cpu_move)
        return Winner::DRAW;

    if ((cpu_move == Moves::Rock && player_move == Moves::Scissors) ||
        (cpu_move == Moves::Scissors && player_move == Moves::Paper) ||
        (cpu_move == Moves::Paper && player_move == Moves::Rock))
        return Winner::CPU;

    return Winner::PLAYER;
}

void RpsGame::print_countdown(uint8_t n)
{
    display.clearDisplay();
    char str[16];
    sprintf(str, "%hu", n);
    display.centerText(display.pctY(0.50f), str);
    display.display();
    sleep_for(duration<float>(settings.game_rps.delay_countdown));
}

void RpsGame::print_player_move(Moves player_move)
{
    display.clearDisplay();
    display.centerText(display.pctY(0.40f), "Your move:");
    display.setFont(FigletType::FullWidth, "Ogre");
    display.centerText(display.getCursorY() + 2, "{}", get_move_str(player_move));
    display.display();
}

void RpsGame::print_moves(Moves computer_move, Moves player_move)
{
    display.clearDisplay();
    display.setFont(FigletType::Smushed, "Doom");

    int left_col  = display.pctX(0.05f);
    int right_col = display.pctX(0.85f);
    int header_y  = display.pctY(0.40f);
    int moves_y   = display.pctY(0.60f);

    display.setCursor(left_col + 3, header_y);
    display.print("You");

    display.setCursor(right_col - 3, header_y);
    display.print("CPU");

    display.setCursor(left_col, moves_y);
    display.print("{}", get_move_str(player_move));

    display.centerText((header_y + moves_y) / 2, "VS");

    display.setCursor(right_col - 8, moves_y);
    display.print("{}", get_move_str(computer_move));

    display.display();
}

void RpsGame::print_winner(Winner winner)
{
    display.setFont(FigletType::FullWidth, "starwars");
    switch (winner)
    {
        case Winner::CPU:
            display.setTextColor(TB_RED);
            display.centerText(display.pctY(0.10f), "CPU Wins");
            break;
        case Winner::DRAW:
            display.setTextColor(TB_MAGENTA);
            display.centerText(display.pctY(0.10f), "Draw");
            break;
        case Winner::PLAYER:
            display.setTextColor(TB_GREEN);
            display.centerText(display.pctY(0.10f), "You Won");
            break;
    }

    display.resetFont();
    display.display();
}

void RpsGame::render()
{
    display.clearDisplay();

    print_player_move(m_player_move);
    if (m_player_move == Moves::None || !m_selected)
        return;

    Moves  computer_move = get_cpu_move();
    Winner winner        = calculate_winner(computer_move, m_player_move);

    display.setFont(FigletType::FullWidth, "Stop");
    print_countdown(3);
    print_countdown(2);
    print_countdown(1);

    print_moves(computer_move, m_player_move);
    print_winner(winner);
    display.display();

    sleep_for(duration<float>(settings.game_rps.delay_show_winner));

    display.clearDisplay();
    print_player_move(Moves::None);
    m_selected = false;
}

SceneResult RpsGame::handle_input(uint32_t key)
{
    switch (key)
    {
        case TB_KEY_ESC: return Scenes::GamesMenu;

        case 'r': m_player_move = Moves::Rock; break;
        case 'p': m_player_move = Moves::Paper; break;
        case 's': m_player_move = Moves::Scissors; break;

        case TB_KEY_ENTER:
        case '\n':
            if (m_player_move != Moves::None)
                m_selected = true;
            break;
    }
    return ScenesGame::RockPaperScissors;
}
