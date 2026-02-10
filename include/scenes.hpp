#pragma once

#include <cstdint>
#include <variant>

enum class Scenes
{
    kNone,
    MainMenu,
    Games,
    Credits,
    Exit,
};

enum class ScenesGame
{
    kNone,
    RockPaperScissors,
    TicTacToe,
};

using SceneResult = std::variant<Scenes, ScenesGame>;

class Scene
{
public:
    virtual ~Scene()                               = default;
    virtual void        render()                   = 0;
    virtual SceneResult handle_input(uint32_t key) = 0;
};

class CreditsScene : public Scene
{
public:
    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

class MainMenuScene : public Scene
{
public:
    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_item = 0;
    static constexpr int MENU_ITEM_COUNT = 2;
};

class GamesMenuScene : public Scene
{
public:
    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_game = 0;
    static constexpr int GAME_COUNT      = 2;
};
