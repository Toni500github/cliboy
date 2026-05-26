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
    // --------------------------------------------------------
    // Overlay API
    // --------------------------------------------------------

    struct OverlayConfig
    {
        std::string              title;
        uintattr_t               title_color = TB_RED | TB_BOLD;
        std::vector<std::string> lines;  // free-form body text
        std::vector<OverlayRow>  rows;   // label: value pairs
        std::string              hint = "R: Restart   ESC: Menu";
    };

    // Red "GAME OVER" box.
    void drawGameOverOverlay(const std::vector<OverlayRow>& rows, int center_y = -1) const
    {
        drawOverlay({ "GAME OVER", TB_RED | TB_BOLD, {}, rows, "R: Restart   ESC: Menu" }, center_y);
    }

    // Green "YOU WIN!" box.
    void drawWinOverlay(const std::vector<OverlayRow>& rows, int center_y = -1) const
    {
        drawOverlay({ "YOU WIN!", TB_GREEN | TB_BOLD, {}, rows, "R: Restart   ESC: Menu" }, center_y);
    }

    // Yellow "PAUSED" box (no rows, different hint).
    void drawPausedOverlay(int center_y = -1) const
    {
        drawOverlay({ "PAUSED", TB_YELLOW | TB_BOLD, {}, {}, "P: Resume   ESC: Menu" }, center_y);
    }

    // final: derived classes use the hook below
    Result<> onBegin() final;

    // Draw an overlay centred on the screen.
    // center_y overrides the automatic vertical midpoint when >= 0.
    static void drawOverlay(const OverlayConfig& cfg, int center_y);

protected:
    // --------------------------------------------------------
    // Hooks, override in derived class
    // --------------------------------------------------------

    // Called after init_game() once
    virtual Result<> onGameBegin() { return Ok(); }

    // (Re-) initialize all game data. Called by on_begin() and by
    // handle_common_input() when the player hits R
    virtual void initGame() = 0;

    // Called every frame from Scenes::render_all() via render()
    virtual void renderGame() = 0;

    // --------------------------------------------------------
    // GameState helpers
    // --------------------------------------------------------
    GameState gameState() const { return m_game_state; }

    bool isPlaying() const { return m_game_state == GameState::Playing; }
    bool isPaused() const { return m_game_state == GameState::Paused; }
    bool isWon() const { return m_game_state == GameState::Won; }
    bool isGameOver() const { return m_game_state == GameState::GameOver; }

    void setGameState(GameState state) { m_game_state = state; }

    void togglePause();

    // --------------------------------------------------------
    // Optional helpers
    // Concrete games are free to use their own fields
    // --------------------------------------------------------
    int  score() const { return m_score; }
    int  bestScore() const { return m_best_score; }
    void resetScore() { m_score = 0; }
    void addScore(int delta);  // auto updates best

    // --------------------------------------------------------
    // Common input handler
    //
    // Call this at the top of handle_input(). If it consumes the key it
    // returns the appropriate SceneResult; otherwise returns std::nullopt and
    // you handle the key yourself.
    //
    // Keys handled:
    //   ESC          -> Scenes::GamesMenu
    //   R / r        -> initGame() + reset state  (only when over or won)
    //   P / p        -> togglePause()
    // --------------------------------------------------------
    std::optional<SceneResult> handleCommonInput(uint32_t key, bool consume_p = true);

private:
    // Scene::render() bridge
    void render() final { renderGame(); }

    GameState m_game_state = GameState::Playing;
    int       m_score      = 0;
    int       m_best_score = 0;
};
