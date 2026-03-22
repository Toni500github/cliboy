#include "scenes/credits.hpp"

#include "terminal_display.hpp"

void CreditsScene::render()
{
    display.clearDisplay();

    display.centerText(display.pctY(0.10f), "Credits");

    const char* credits[] = {
        "CliBoy - Terminal Games Collection",
        "",
        "A simple demonstration of terminal-based games",
        "",
        "Thank you for playing!",
    };

    const int n_credits = ARRAY_SIZE(credits);
    int       start_y   = display.pctY(0.50f) - (n_credits - 1) / 2;
    for (int i = 0; i < n_credits; i++)
        display.centerText(start_y + i, credits[i]);

    display.display();
}

SceneResult CreditsScene::handle_input(uint32_t key)
{
    if (key == TB_KEY_ESC)
        return Scenes::MainMenu;
    return Scenes::Credits;
}
