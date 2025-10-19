#include "scenes.hpp"

#include <vector>

#include "settings.hpp"
#include "terminal_display.hpp"
#include "util.hpp"

void drawMenu(int y, int distance, int current_choice, const std::vector<MenuItem>& items)
{
    for (const auto& item : items)
    {
        if (current_choice == item.id)
            display.centerText(y, "> {}", item.text);
        else
            display.centerText(y, "{}", item.text);
        y += distance;
    }
}

static void load_scene_game(int game)
{
    display.clearDisplay();
    display.setTextColor(0xFFFFFF);

    display.setCursor(0, display.getHeight() / 2);
    display.print("<");
    display.setCursor(display.getWidth() - 7, display.getCursorY());
    display.print(">");

    display.setFont(FIGLET_KERNING, "Small");
    switch (game)
    {
        case GAME_RPS:
            display.centerText(display.getHeight() / 3, "Rock");
            display.centerText(display.getCursorY() + 2, "Paper");
            display.centerText(display.getCursorY() + 2, "Scissors");
            break;
        case GAME_TTT:
            display.centerText(display.getHeight() / 3, "Tic");
            display.centerText(display.getCursorY() + 2, "Tax");
            display.centerText(display.getCursorY() + 2, "Toe");
            break;
    }
    display.resetFont();

    display.display();
}

void load_scene_game_settings(int choice)
{
    display.clearDisplay();
    display.setTextColor(0xFFFFFF);

    display.setFont(FIGLET_FULL_WIDTH, "Standard");
    display.centerText(10, "Settings");
    display.resetFont();

    const std::vector<MenuItem> menu_items{
        { SCENE_SETTINGS_KEY_UP, std::format("Up key: {:c}", settings.ch_up) },
        { SCENE_SETTINGS_KEY_DOWN, std::format("Down key: {:c}", settings.ch_down) },
        { SCENE_SETTINGS_KEY_LEFT, std::format("Left key: {:c}", settings.ch_left) },
        { SCENE_SETTINGS_KEY_RIGHT, std::format("Right key: {:c}", settings.ch_right) },
        { SCENE_SETTINGS_KEY_QUIT, std::format("Quit key: ESC") }
    };
    drawMenu(display.getCursorY() + 4, 2, choice, menu_items);

    display.display();
}

void load_scene_main_menu(int choice)
{
    display.clearDisplay();
    display.setTextColor(0xFFFFFF);

    display.setFont(FIGLET_FULL_WIDTH, "Big Money-nw");
    display.centerText(5, "CliBoy");
    display.resetFont();

    static const std::vector<MenuItem> menu_items{ { SCENE_MAIN_MENU_SINGLEP, "Games" },
                                                   { SCENE_MAIN_MENU_SETTINGS, "Settings" } };
    display.setFont(FIGLET_FULL_WIDTH, "Small Slant");
    drawMenu(display.getCursorY() + 5, 5, choice, menu_items);
    display.resetFont();

    display.display();
}

void load_scene(int scene, int game)
{
    switch (scene)
    {
        case SCENE_MAIN_MENU: load_scene_main_menu(game); break;
        case SCENE_GAMES:     load_scene_game(game); break;
        case SCENE_SETTINGS:  load_scene_game_settings(game); break;
    }
}
