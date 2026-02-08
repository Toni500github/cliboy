#include <notcurses/notcurses.h>

#include "scenes.hpp"
#include "terminal_display.hpp"

void GamesMenuScene::render()
{
    display.clearDisplay();

    int rows = display.getHeight();

    display.centerText(5, "Select a Game");

    const char* game_items[] = { "Rock Paper Scissors", "Tic Tac Toe" };
    int         start_y      = rows / 2 - 2;

    for (int i = 0; i < GAME_COUNT; i++)
    {
        int y = start_y + i * 2;

        if (i == m_selected_game)
        {
            display.setTextColor(0xffffff);
            // display.setTextBgColor(0x00FFFF);
            display.centerText(y, "> {} <", game_items[i]);
            display.resetColors();
        }
        else
        {
            display.centerText(y, game_items[i]);
        }
    }

    display.centerText(rows - 2, "Arrow Keys: Navigate | Enter: Play | ESC: Back");

    display.display();
}

SceneResult GamesMenuScene::handle_input(uint32_t key)
{
    switch (key)
    {
        case NCKEY_ESC:   return Scenes::MainMenu;

        case NCKEY_UP:   m_selected_game = (m_selected_game - 1 + GAME_COUNT) % GAME_COUNT; break;
        case NCKEY_DOWN: m_selected_game = (m_selected_game + 1) % GAME_COUNT; break;

        case NCKEY_ENTER:
            switch (m_selected_game)
            {
                case 0: return ScenesGame::RockPaperScissors;
                case 1: return ScenesGame::TicTacToe;
            }
    }

    return Scenes::Games;
}
