#ifndef SCENES_HPP
#define SCENES_HPP

enum Scenes
{
    SCENE_NONE = 0,
    SCENE_MAIN_MENU,
    SCENE_SINGLEP_GAMES,
    SCENE_MULTIP_GAMES,
    SCENE_CREDITS,
};

enum ScenesMainMenu
{
    SCENE_MAIN_MENU_NONE = 10,
    SCENE_MAIN_MENU_SINGLEP,
    SCENE_MAIN_MENU_MULTIP,
    SCENE_MAIN_MENU_CREDITS,
};

enum selectedSinglePGame
{
    GAME_SINGLEP_NONE = 20,
    GAME_SINGLEP_RPS,
    GAME_SINGLEP_SNAKE,
};

enum selectedMultiPGame
{
    GAME_MULTIP_NONE = 300,
    GAME_MULTIP_RPS,
    GAME_MULTIP_PING_PONG,
    GAME_MULTIP_TTT,
};

inline int currentScene = SCENE_MAIN_MENU;

void load_scene(int scene, int game);
void load_scene_main_menu(int choice);
void load_scene_game_credits();

#endif
