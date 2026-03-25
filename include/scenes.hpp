#pragma once

#include <cstdint>
#include <variant>

#include "terminal_display.hpp"
#include "settings.hpp"
#include "util.hpp"

enum class Scenes
{
    GamesMenu,
    SettingsMenu,
    Credits,
    COUNT,  // If you not using it in MainMenuScene, then use Exit

    MainMenu,
    Exit,
};

enum class ScenesGame
{
    Tetris,
    TicTacToe,
    Snake,
    Wordle,
    Game2048,
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

    virtual void render_footer()
    {
        if (m_footer_text.empty())
            return;

        display.resetFont();
        display.centerText(display.getHeight() - m_footer_padding, m_footer_text);
        display.display();
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
