#pragma once

#include <cstdint>
#include <variant>

#include "terminal_display.hpp"
#include "util.hpp"

enum class Scenes
{
    MainMenu,
    GamesMenu,
    Credits,
    SettingsMenu,
    Exit,
};

enum class ScenesGame
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
    virtual int         frame_ms()
    {
        // If -1, then the tb_peek_event will be blocking
        // else it run every ms
        return -1;
    }

    Result<> begin()
    {
        if (m_has_begun)
            return Ok();

        m_has_begun = true;
        return on_begin();
    }

    void render_all()
    {
        display.clearDisplay();
        display.resetFont();

        render();  // derived class implements this

        render_footer();

        display.display();
    }

    bool has_begun() const { return m_has_begun; }

    virtual void render_footer()
    {
        if (m_footer_text.empty())
            return;

        display.resetFont();
        display.centerText(display.getHeight() - m_footer_padding, m_footer_text);
        display.display();
    }

protected:
    virtual Result<> on_begin() { return Ok(); }
    void             set_footer(std::string text, int padding = 3)
    {
        m_footer_text    = std::move(text);
        m_footer_padding = padding;
    }

private:
    bool        m_has_begun      = false;
    int         m_footer_padding = 3;
    std::string m_footer_text;
};

class CreditsScene : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("ESC: Back");
        return Ok();
    }

    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

class MainMenuScene : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("Arrow Keys: Navigate | Enter: Select | ESC: Exit");
        return Ok();
    }

    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_item = 0;
    static constexpr int MENU_ITEM_COUNT = 3;
};

class GamesMenuScene : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("Arrow Keys: Navigate | Enter: Play | ESC: Back");
        return Ok();
    }

    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_game = 0;
    static constexpr int GAME_COUNT      = static_cast<int>(ScenesGame::COUNT);
};

class SettingsScene : public Scene
{
public:
    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    size_t      m_selected_item = 0;
    bool        m_editing       = false;
    std::string m_edit_buffer;
};
