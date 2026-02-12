#pragma once

#include <cstdint>
#include <variant>

#include "util.hpp"

enum class Scenes
{
    kNone,
    MainMenu,
    Games,
    Credits,
    Exit,
};

enum class ScenesGame : uint8_t
{
    RockPaperScissors,
    TicTacToe,
    Wordle,
    COUNT,
};

using SceneResult = std::variant<Scenes, ScenesGame>;

class Scene
{
public:
    virtual ~Scene()                               = default;
    virtual void        render()                   = 0;
    virtual SceneResult handle_input(uint32_t key) = 0;
    Result<>            begin()
    {
        if (m_has_begun)
            return Ok();

        m_has_begun = true;
        return on_begin();
    }

    bool has_begun() const { return m_has_begun; }

protected:
    virtual Result<> on_begin() { return Ok(); }

private:
    bool m_has_begun = false;
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
    static constexpr int GAME_COUNT      = static_cast<int>(ScenesGame::COUNT);
};
