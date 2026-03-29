#pragma once

#include <optional>
#include <string>
#include <utility>

#include "scenes.hpp"
#include "terminal_display.hpp"
#include "util.hpp"

enum class GameState
{
    Playing,
    Paused,
    Won,
    GameOver
};

struct OverlayRow
{
    std::string label;  // e.g "Score"
    std::string value;  // 1234

    OverlayRow(std::string lbl, std::string vl) : label(std::move(lbl)), value(std::move(vl)) {}
};

class BaseGame : public Scene
{
public:
    // final: derived classes use the hook below
    Result<> on_begin() final;

protected:
    // --------------------------------------------------------
    // Hooks, override in derived class
    // --------------------------------------------------------

    // Called after init_game() once
    virtual Result<> on_game_begin() { return Ok(); }

    // (Re-) initialize all game data. Called by on_begin() and by
    // handle_common_input() when the player hits R
    virtual void init_game() = 0;

    // Called every frame from Scenes::render_all() via render()
    virtual void render_game() = 0;

    // --------------------------------------------------------
    // GameState helpers
    // --------------------------------------------------------
    GameState game_state() const { return m_game_state; }

    bool is_playing() const { return m_game_state == GameState::Playing; }
    bool is_paused() const { return m_game_state == GameState::Paused; }
    bool is_won() const { return m_game_state == GameState::Won; }
    bool is_game_over() const { return m_game_state == GameState::GameOver; }

    void set_game_state(GameState state) { m_game_state = state; }

    void toggle_pause();

    // --------------------------------------------------------
    // Optional helpers
    // Concrete games are free to use their own fields
    // --------------------------------------------------------
    int  score() const { return m_score; }
    int  best_score() const { return m_best_score; }
    void reset_score() { m_score = 0; }
    void add_score(int delta);  // auto updates best

    // --------------------------------------------------------
    // Overlay API
    // --------------------------------------------------------

    struct OverlayConfig
    {
        std::string             title;
        uintattr_t              title_color = TB_RED | TB_BOLD;
        std::vector<OverlayRow> rows;
        std::string             hint = "R: Restart   ESC: Menu";
    };

    // Draw an overlay centred on the screen.
    // center_y overrides the automatic vertical midpoint when >= 0.
    void draw_overlay(const OverlayConfig& cfg, int center_y = -1) const;

    // Red "GAME OVER" box.
    void draw_game_over_overlay(const std::vector<OverlayRow>& rows) const
    {
        draw_overlay({ "GAME OVER", TB_RED | TB_BOLD, rows, "R: Restart   ESC: Menu" });
    }

    // Green "YOU WIN!" box.
    void draw_win_overlay(const std::vector<OverlayRow>& rows) const
    {
        draw_overlay({ "YOU WIN!", TB_GREEN | TB_BOLD, rows, "R: Restart   ESC: Menu" });
    }

    // Yellow "PAUSED" box (no rows, different hint).
    void draw_paused_overlay() const { draw_overlay({ "PAUSED", TB_YELLOW | TB_BOLD, {}, "P: Resume   ESC: Menu" }); }

    // --------------------------------------------------------
    // Common input handler
    //
    // Call this at the top of handle_input(). If it consumes the key it
    // returns the appropriate SceneResult; otherwise returns std::nullopt and
    // you handle the key yourself.
    //
    // Keys handled:
    //   ESC          -> Scenes::GamesMenu
    //   R / r        -> init_game() + reset state  (only when over or won)
    //   P / p        -> toggle_pause()
    // --------------------------------------------------------
    std::optional<SceneResult> handle_common_input(uint32_t key, bool consume_p = true);

private:
    // Scene::render() bridge
    void render() final { render_game(); }

    GameState m_game_state = GameState::Playing;
    int       m_score      = 0;
    int       m_best_score = 0;
};
