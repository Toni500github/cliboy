#include "game.hpp"

static std::string str_repeat(const std::string& s, size_t n)
{
    std::string out;
    out.reserve(s.size() * std::max(0UL, n));
    for (size_t i = 0; i < n; ++i)
        out += s;
    return out;
}

// Pad a string to exactly `width` chars, right-aligned
static std::string rpad(const std::string& s, size_t width)
{
    if (s.size() >= width)
        return s;
    return std::string(width - s.size(), ' ') + s;
}

// Centre a string inside a field of `width` chars.
static std::string centre_in(const std::string& s, size_t width)
{
    size_t total_pad = width - s.size();
    size_t pad_l     = std::max(0UL, total_pad / 2);
    size_t pad_r     = std::max(0UL, total_pad - pad_l);
    return std::string(pad_l, ' ') + s + std::string(pad_r, ' ');
}

Result<> BaseGame::on_begin()
{
    init_game();
    return on_game_begin();
}

void BaseGame::toggle_pause()
{
    if (m_game_state == GameState::Playing)
    {
        m_game_state = GameState::Paused;
        playback.pauseMusic();
    }
    else if (m_game_state == GameState::Paused)
    {
        m_game_state = GameState::Playing;
        playback.resumeMusic();
    }
}

void BaseGame::add_score(int delta)
{
    m_score += delta;
    if (m_score > m_best_score)
        m_best_score = m_score;
}

void BaseGame::draw_overlay(const OverlayConfig& cfg, int center_y) const
{
    const bool utf8 = settings.general.utf8;

    const std::string H  = utf8 ? "═" : "-";
    const std::string V  = utf8 ? "║" : "|";
    const std::string TL = utf8 ? "╔" : "+";
    const std::string TR = utf8 ? "╗" : "+";
    const std::string BL = utf8 ? "╚" : "+";
    const std::string BR = utf8 ? "╝" : "+";

    // Calculate inner width (content area, excluding the border chars)
    // Minimum to fit the title with 2 spaces padding either side.
    size_t inner_w = cfg.title.size() + 4;

    // Each info row: "  Label: " + value + "  "
    for (const OverlayRow& row : cfg.rows)
    {
        size_t row_content = 2                        // leading spaces
                             + row.label.size() + 2   // ": "
                             + row.value.size() + 2;  // trailing spaces
        inner_w            = std::max(inner_w, row_content);
    }

    // Hint line sits outside the box but we still want the box at least as wide.
    if (!cfg.hint.empty())
        inner_w = std::max(inner_w, cfg.hint.size());

    // Round up to even so the centred title always has symmetric padding.
    if (inner_w % 2 != 0)
        inner_w++;

    // Vertical position
    // Number of rendered rows: top border + title + N info rows + bottom border.
    const int box_rows = 2 + cfg.rows.size();
    if (center_y < 0)
        center_y = display.getHeight() / 2 - box_rows / 2;

    int y = center_y;

    // --------------------------------------------------------
    // Time to draw
    // --------------------------------------------------------
    display.setTextColor(cfg.title_color);

    // Top border: ╔══════════╗
    display.centerText(y++, "{}{}{}", TL, str_repeat(H, inner_w), TR);

    // Title row: ║  GAME OVER  ║
    display.centerText(y++, "{}{}{}", V, centre_in(cfg.title, inner_w), V);

    // Info rows: ║  Score:   42  ║
    for (const auto& row : cfg.rows)
    {
        // Build "  Label: " prefix, then right-align the value in the remaining space.
        const std::string prefix = "  " + row.label + ": ";
        const size_t      val_w  = inner_w - prefix.size() - 2;
        const std::string line   = prefix + rpad(row.value, val_w) + "  ";
        display.centerText(y++, "{}{}{}", V, line, V);
    }

    // Bottom border: ╚══════════╝
    display.centerText(y++, "{}{}{}", BL, str_repeat(H, inner_w), BR);

    // Hint line (one blank row gap below the box)
    if (!cfg.hint.empty())
    {
        display.setTextColor(TB_WHITE);
        display.centerText(y + 1, cfg.hint);
    }

    display.resetColors();
}

std::optional<SceneResult> BaseGame::handle_common_input(uint32_t key, bool consume_p)
{
    // Always let ESC go back to the game menu.
    if (key == TB_KEY_ESC)
        return Scenes::GamesMenu;

    // R restarts only when the game has ended (won or game over).
    if ((key == 'r' || key == 'R') && (is_game_over() || is_won()))
    {
        m_game_state = GameState::Playing;
        reset_score();
        init_game();
        return scene_id();
    }

    // P toggles pause (only meaningful while playing or already paused).
    if (consume_p && (key == 'p' || key == 'P') && !is_game_over() && !is_won())
    {
        toggle_pause();
        return scene_id();
    }

    return std::nullopt;  // key not consumed — caller handles it
}
