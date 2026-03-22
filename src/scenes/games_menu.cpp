#include "scenes/games_menu.hpp"

#include "terminal_display.hpp"

void GamesMenuScene::render()
{
    display.clearDisplay();

    display.centerText(display.pctY(0.10f), "Select a Game");

    const char* game_items[] = { "Rock Paper Scissors", "Tic Tac Toe", "Snake", "Wordle" };
    int         start_y      = display.pctY(0.50f) - (GAME_COUNT - 1);

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

    display.display();
}

SceneResult GamesMenuScene::handle_input(uint32_t key)
{
    switch (key)
    {
        case TB_KEY_ESC: return Scenes::MainMenu;

        case TB_KEY_ARROW_UP:   m_selected_game = (m_selected_game - 1 + GAME_COUNT) % GAME_COUNT; break;
        case TB_KEY_ARROW_DOWN: m_selected_game = (m_selected_game + 1) % GAME_COUNT; break;

        case TB_KEY_ENTER:
        case '\n':
            return static_cast<ScenesGame>(m_selected_game);
    }

    return Scenes::GamesMenu;
}
