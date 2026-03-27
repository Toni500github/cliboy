#include "scenes/games_menu.hpp"

#include "audio_player.hpp"
#include "terminal_display.hpp"

struct GameEntry
{
    const char* name;
    const char* tag;
    uintattr_t  color;
};

void GamesMenuScene::render()
{
    if (!playback.isMusicPlaying())
        playback.playMusic(MenuSounds::BGM);

    display.clearDisplay();

    // Title
    display.setTextColor(0xffff00);
    display.centerText(display.pctY(0.06f), "Select a Game");
    display.resetColors();

    // Separator
    display.setTextColor(TB_CYAN);
    display.drawLine(display.pctX(0.20f),
                     display.pctY(0.12f),
                     display.pctX(0.80f),
                     display.pctY(0.12f),
                     settings.general.utf8 ? U'─' : '-');
    display.resetColors();

    // Game list
    // clang-format off
    static constexpr GameEntry game_items[] = {
        { "Tetris",              "stack & clear lines",  TB_RED    | TB_BOLD },
        { "Tic Tac Toe",         "3 in a row",           TB_BLUE   | TB_BOLD },
        { "Snake",               "eat, grow, survive",   TB_GREEN  | TB_BOLD },
        { "Wordle",              "5-letter word guess",  TB_YELLOW | TB_BOLD },
        { "2048",                "merge to 2048",        TB_CYAN   | TB_BOLD },
    };
    // clang-format on

    const int item_step = 3;
    const int block_h   = GAME_COUNT * item_step - 1;

    // Center between separator and footer
    const int avail_top = display.pctY(0.12f) + 2;
    const int avail_bot = display.getHeight() - 4;
    const int start_y   = avail_top + (avail_bot - avail_top - block_h) / 2;

    for (int i = 0; i < GAME_COUNT; i++)
    {
        int              y = start_y + i * item_step;
        const GameEntry& g = game_items[i];

        if (i == m_selected_game)
        {
            display.setTextColor(g.color);
            display.centerText(y, "» {} «", g.name);
            display.resetColors();
            display.setTextColor(TB_WHITE);
            display.centerText(y + 1, "{}", g.tag);
        }
        else
        {
            display.setTextColor(g.color & ~TB_BOLD);  // same hue, not bold
            display.centerText(y, "{}", g.name);
        }
        display.resetColors();
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

        case '\n':
        case TB_KEY_ENTER: return static_cast<ScenesGame>(m_selected_game);
    }

    return Scenes::GamesMenu;
}

void GamesMenuScene::end(SceneResult next_scen)
{
    // Keep music running if staying within menu scenes (MainMenu shares the BGM).
    // Stop it if heading into a game.
    if (std::holds_alternative<ScenesGame>(next_scen))
        playback.stopMusic();
}
