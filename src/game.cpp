#include "game.hpp"

#include "settings.hpp"

static std::string str_repeat(const std::string& s, size_t n)
{
    std::string out;
    out.reserve(s.size() * std::max<size_t>(0, n));
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
    if (s.size() >= width)
        return s;

    size_t total_pad = width - s.size();
    size_t pad_l     = std::max<size_t>(0, total_pad / 2);
    size_t pad_r     = std::max<size_t>(0, total_pad - pad_l);
    return std::string(pad_l, ' ') + s + std::string(pad_r, ' ');
}

void show_error(const std::string& title, const std::string& msg)
{
    display.clearDisplay();

    BaseGame::drawOverlay({ title, TB_RED | TB_BOLD, { msg }, {}, "Press ENTER to continue" });

    display.display();

    tb_event ev;
    while (tb_poll_event(&ev) > 0)
        if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_ENTER)
            break;
}

Result<> BaseGame::onBegin()
{
    initGame();
    return onGameBegin();
}

void BaseGame::togglePause()
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

void BaseGame::addScore(int delta)
{
    m_score += delta;
    if (m_score > m_best_score)
        m_best_score = m_score;
}

void BaseGame::drawOverlay(const OverlayConfig& cfg, int center_y)
{
    std::string_view H, V, TL, TR, BL, BR;
    if (settings.general.utf8)
    {
        H  = "═";
        V  = "║";
        TL = "╔";
        TR = "╗";
        BL = "╚";
        BR = "╝";
    }
    else
    {
        H  = "-";
        V  = "|";
        TL = "+";
        TR = "+";
        BL = "+";
        BR = "+";
    }

    // Minimum to fit the title with 2 spaces padding either side.
    size_t inner_w = cfg.title.size() + 4;

    // Free-form lines: contents + 2 spaces padding each side
    for (const std::string& line : cfg.lines)
        inner_w = std::max(inner_w, line.size() + 4);

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
    const int box_x = display.getWidth() / 2 - static_cast<int>(inner_w + 2) / 2;

    int y = center_y;

    // Time to draw
    display.setTextColor(cfg.title_color);

    // Top border: ╔══════════╗
    display.centerText(y++, "{}{}{}", TL, str_repeat(H.data(), inner_w), TR);

    // Title row: ║  GAME OVER  ║
    display.centerText(y++, "{}{}{}", V, centre_in(cfg.title, inner_w), V);

    // Free-form lines
    for (const std::string& line : cfg.lines)
    {
        display.setTextColor(cfg.title_color);
        display.setCursor(box_x, y);
        display.print("{}", V);

        display.setTextColor(TB_WHITE);
        display.setCursor(box_x + 1, y);
        display.print("{}", centre_in(line, inner_w));

        display.setTextColor(cfg.title_color);
        display.setCursor(box_x + 1 + static_cast<int>(inner_w), y++);
        display.print("{}", V);
    }

    // Info rows: ║  Score:   42  ║
    for (const OverlayRow& row : cfg.rows)
    {
        // Build "  Label: " prefix, then right-align the value in the remaining space.
        const std::string prefix = "  " + row.label + ": ";
        const size_t      val_w  = inner_w - prefix.size() - 2;
        const std::string line   = prefix + rpad(row.value, val_w) + "  ";
        display.centerText(y++, "{}{}{}", V, line, V);
    }

    // Bottom border: ╚══════════╝
    display.centerText(y++, "{}{}{}", BL, str_repeat(H.data(), inner_w), BR);

    // Hint line (one blank row gap below the box)
    if (!cfg.hint.empty())
    {
        display.setTextColor(TB_WHITE);
        display.centerText(y + 1, cfg.hint);
    }

    display.resetColors();
}

std::optional<SceneResult> BaseGame::handleCommonInput(uint32_t key, bool consume_p)
{
    if (key == TB_KEY_ESC)
        return Scenes::GamesMenu;

    if ((key == 'r' || key == 'R') && (isGameOver() || isWon()))
    {
        m_game_state = GameState::Playing;
        resetScore();
        initGame();
        return sceneID();
    }

    if (consume_p && (key == 'p' || key == 'P') && !isGameOver() && !isWon())
    {
        togglePause();
        return sceneID();
    }

    return std::nullopt;  // key not consumed, the caller handles it
}
