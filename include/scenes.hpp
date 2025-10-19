#ifndef SCENES_HPP
#define SCENES_HPP

enum Scenes
{
    SCENE_NONE = 0,
    SCENE_MAIN_MENU,
    SCENE_GAMES,
    SCENE_SETTINGS,
};

enum ScenesMainMenu
{
    SCENE_MAIN_MENU_NONE = 10,
    SCENE_MAIN_MENU_SINGLEP,
    SCENE_MAIN_MENU_SETTINGS,
};

enum selectedGame
{
    GAME_NONE = 20,
    GAME_RPS,
    GAME_TTT,
};

enum ScenesSettings
{
    SCENE_SETTINGS_KEY_UP = 60,
    SCENE_SETTINGS_KEY_DOWN,
    SCENE_SETTINGS_KEY_LEFT,
    SCENE_SETTINGS_KEY_RIGHT,
    SCENE_SETTINGS_KEY_QUIT
};

inline int currentScene = SCENE_MAIN_MENU;

void load_scene(int scene, int game);
void load_scene_main_menu(int choice);
void load_scene_game_credits();

#endif
