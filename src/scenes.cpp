#include "scenes.hpp"

#include "terminal_display.hpp"

static void load_scene_game(int game)
{
    display.clearDisplay();
    display.setTextColor(0xFFFFFF);

    display.setCursor(0, 35);
    display.print("<");

    switch (game)
    {
        case GAME_MULTIP_RPS:
        case GAME_SINGLEP_RPS: display.centerText(10, "Rock Paper Scissors"); break;
    }

    display.setCursor(display.getWidth() - 7, 35);
    display.print(">");

    display.display();
}

void load_scene_game_credits()
{
    display.clearDisplay();
    display.setTextColor(0xFFFFFF);

    display.centerText(10, "Created on");
    display.centerText(20, "Toni500github/cliboy");

    display.display();
}

void load_scene_main_menu(int choice)
{
    display.clearDisplay();
    display.setTextColor(0xFFFFFF);

    display.centerText(5, "CliBoy v0.0.1");

    display.centerText(35, choice == SCENE_MAIN_MENU_SINGLEP ? "> Single-Player" : "Single-Player");
    display.centerText(45, choice == SCENE_MAIN_MENU_MULTIP ? "> Multi-Player" : "Multi-Player");
    display.centerText(55, choice == SCENE_MAIN_MENU_CREDITS ? "> Credits" : "Credits");

    display.display();
}

void load_scene(int scene, int game)
{
    switch (scene)
    {
        case SCENE_MAIN_MENU:     load_scene_main_menu(game); break;
        case SCENE_MULTIP_GAMES:
        case SCENE_SINGLEP_GAMES: load_scene_game(game); break;
        case SCENE_CREDITS:       load_scene_game_credits(); break;
    }
}
