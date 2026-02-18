#include "scenes.hpp"
#include "terminal_display.hpp"

void CreditsScene::render()
{
    display.clearDisplay();

    int rows = display.getHeight();

    display.centerText(2, "Credits");

    const char* credits[] = {
        "CliBoy - Terminal Games Collection",
        "",
        "A simple demonstration of terminal-based games",
        "",
        "Thank you for playing!",
    };

    int start_y = rows / 2 - 3;
    for (size_t i = 0; i < sizeof(credits) / sizeof(credits[0]); i++)
        display.centerText(start_y + i, credits[i]);

    display.display();
}

SceneResult CreditsScene::handle_input(uint32_t key)
{
    if (key == 27)
        return Scenes::MainMenu;
    return Scenes::Credits;
}
