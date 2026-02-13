#include "scenes.hpp"
#include "terminal_display.hpp"

void GamesMenuScene::render()
{
    display.clearDisplay();

    int rows = display.getHeight();

    display.centerText(5, "Select a Game");

    const char* game_items[] = { "Rock Paper Scissors", "Tic Tac Toe", "Wordle" };
    int         start_y      = rows / 2 - 2;

    for (int i = 0; i < GAME_COUNT; i++)
    {
        int y = start_y + i * 2;

        if (i == m_selected_game)
        {
            display.setTextColor(TB_WHITE | TB_BOLD);
            // display.setTextBgColor(TB_BLACK);
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
        case 27: return Scenes::MainMenu;

        case TB_KEY_ARROW_UP:   m_selected_game = (m_selected_game - 1 + GAME_COUNT) % GAME_COUNT; break;
        case TB_KEY_ARROW_DOWN: m_selected_game = (m_selected_game + 1) % GAME_COUNT; break;

        case TB_KEY_ENTER:
        case '\n':
            switch (m_selected_game)
            {
                case 0: return ScenesGame::RockPaperScissors;
                case 1: return ScenesGame::TicTacToe;
                case 2: return ScenesGame::Wordle;
            }
    }

    return Scenes::Games;
}
