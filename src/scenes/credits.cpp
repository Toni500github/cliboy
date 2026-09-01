#include "scenes/credits.hpp"

#include "terminal_display.hpp"

void CreditsScene::render()
{
    display.clearDisplay();

    display.centerText(display.pctY(0.10f), "Credits");

    constexpr const char* credits[] = {
        "CliBoy - Terminal Games Collection",
        "",
        "A simple demonstration of terminal-based games",
        "https://github.com/Toni500github/cliboy/",
        "",
        "Music credits:",
        "- Menus, Wordle and 2048 Background Musics:",
        "https://www.youtube.com/playlist?list=PLwJjxqYuirCLkq42mGw4XKGQlpZSfxsYd",
        "",
        "- Tetris soundtrack:",
        "https://youtu.be/NmCCQxVBfyM",
        "",
        "- Minesweeper soundtrack:",
        "https://youtu.be/2ui7CagiIoo",
        "",
        "",
        "Thank you for playing!",
    };

    constexpr int n_credits = ARRAY_SIZE(credits);
    int           start_y   = display.pctY(0.50f) - (n_credits - 1) / 2;
    for (int i = 0; i < n_credits; i++)
        display.centerText(start_y + i, credits[i]);

    display.display();
}

SceneResult CreditsScene::handleInput(uint32_t key)
{
    if (key == TB_KEY_ESC)
        return Scenes::MainMenu;
    return sceneID();
}
