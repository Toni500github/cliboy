#include "settings.hpp"

#include <format>
#include <functional>
#include <string>

#include "scenes/settings.hpp"
#include "terminal_display.hpp"

enum class SettingKind
{
    Float,   // ←/→ = decrease/increase by a fixed step
    Bool,    // ←/→/Enter = toggle; displayed as "On" / "Off"
    String,  // Enter = opens an inline edit mode; ESC = cancels, Enter = confirms
};

struct SettingEntry
{
    const char*                  section;  // non-null on the first item of each group
    const char*                  label;
    SettingKind                  kind;
    std::function<std::string()> get_value;
    std::function<void(int)>     adjust;  // Float: -1/+1; Bool: called with any n to toggle; nullptr for String
    std::function<void(const std::string&)> set_str_value;  // String only; nullptr for Float/Bool
    const uint32_t*                         preview_color = nullptr;
};

static std::string fmt_float(float v)
{
    return std::format("{:.2f}s", v);
}

static std::string fmt_bool(bool v)
{
    return v ? "ON" : "OFF";
}

static std::string fmt_hex(uint32_t v)
{
    char buf[8];
    snprintf(buf, sizeof(buf), "#%06x", v);
    return buf;
}

static void set_hex(uint32_t& target, const std::string& s)
{
    // accept both "rrggbb" and "#rrggbb"
    const char* src = s.c_str();
    if (*src == '#')
        src++;
    char*    end;
    uint32_t val = strtoul(src, &end, 16);
    if (end != src)  // valid hex
        target = val;
}

static void clamp_float(float& v, float step, float lo, float hi, int dir)
{
    v += step * dir;
    if (v < lo)
        v = lo;
    if (v > hi)
        v = hi;
}

// clang-format off
static const SettingEntry entries[] = {
    // General
    {
        "General settings",
        "ASCII characters gameplay",
        SettingKind::Bool,
        [] { return fmt_bool(!settings.general.utf8); },
        [](int) { settings.general.utf8 = !settings.general.utf8; },
        nullptr
    },
    {
        nullptr,
        "Path to assets",
        SettingKind::String,
        [] { return settings.general.assets_path; },
        nullptr,
        [](const std::string& s) { settings.general.assets_path = s; }
    },

    // Colors
    {
        "Colors",
        "Black",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.black); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.black, s); },
        &settings.colors.black
    },
    {
        nullptr,
        "Red",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.red); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.red, s); },
        &settings.colors.red
    },
    {
        nullptr,
        "Green",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.green); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.green, s); },
        &settings.colors.green
    },
    {
        nullptr,
        "Yellow",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.yellow); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.yellow, s); },
        &settings.colors.yellow
    },
    {
        nullptr,
        "Blue",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.blue); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.blue, s); },
        &settings.colors.blue
    },
    {
        nullptr,
        "Magenta",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.magenta); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.magenta, s); },
        &settings.colors.magenta
    },
    {
        nullptr,
        "Cyan",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.cyan); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.cyan, s); },
        &settings.colors.cyan
    },
    {
        nullptr,
        "White",
        SettingKind::String,
        [] { return fmt_hex(settings.colors.white); },
        nullptr,
        [](const std::string& s) { set_hex(settings.colors.white, s); },
        &settings.colors.white
    },

    // Tic-Tac-Toe
    {
        "Tic-Tac-Toe",
        "Show Endgame Delay",
        SettingKind::Float,
        [] { return fmt_float(settings.game_ttt.delay_show_endgame); },
        [](int d) { clamp_float(settings.game_ttt.delay_show_endgame, 0.1f, 0.1f, 10.0f, d); },
        nullptr
    },
    {
        nullptr,
        "Strike Animation Delay",
        SettingKind::Float,
        [] { return fmt_float(settings.game_ttt.delay_strike_anim); },
        [](int d) { clamp_float(settings.game_ttt.delay_strike_anim, 0.01f, 0.01f, 1.0f, d); },
        nullptr
    },

    // Snake
    {
        "Snake",
        "Minimum speed (ms/tick)",
        SettingKind::Float,
        [] { return fmt_float(settings.game_snake.snake_min_speed); },
        [](int d) { clamp_float(settings.game_snake.snake_min_speed, 5.0f, 5.0f, 500.0f, d); },
        nullptr
    },
    {
        nullptr,
        "Maximum speed (ms/tick)",
        SettingKind::Float,
        [] { return fmt_float(settings.game_snake.snake_max_speed); },
        [](int d) { clamp_float(settings.game_snake.snake_max_speed, 5.0f, 5.0f, 500.0f, d); },
        nullptr
    },

    // Wordle
    {
        "Wordle",
        "Show Final Grid Delay",
        SettingKind::Float,
        [] { return fmt_float(settings.game_wordle.delay_show_final_grid); },
        [](int d) { clamp_float(settings.game_wordle.delay_show_final_grid, 0.05f, 0.05f, 5.0f, d); },
        nullptr
    },
    {
        nullptr,
        "Show Endgame Delay",
        SettingKind::Float,
        [] { return fmt_float(settings.game_wordle.delay_show_endgame); },
        [](int d) { clamp_float(settings.game_wordle.delay_show_endgame, 0.1f, 0.1f, 10.0f, d); },
        nullptr
    },
    {
        nullptr,
        "Words File Path",
        SettingKind::String,
        [] { return settings.game_wordle.wordle_txt_path; },
        nullptr,
        [](const std::string& s) { settings.game_wordle.wordle_txt_path = s; }
    },
};
// clang-format on

static void render_value(const SettingEntry& e,
                         bool                selected,
                         bool                editing,
                         const std::string&  edit_buffer,
                         int                 col,
                         int                 row)
{
    const std::string val = e.get_value();
    display.setCursor(col, row);

    switch (e.kind)
    {
        case SettingKind::Float:
        case SettingKind::Bool:
            if (selected)
                display.print("< {} >", val);
            else
                display.print("{}", val);
            break;

        case SettingKind::String:
            if (editing)
            {
                display.setTextColor(TB_YELLOW | TB_BOLD);
                display.print("{}{}", edit_buffer, settings.general.utf8 ? "\u2588" : "|");
                display.resetColors();
            }
            else if (selected)
            {
                display.print("{}  [Enter to edit]", val);
            }
            else
            {
                display.print("{}", val);
            }
            break;
    }

    // Draw color preview swatch if this entry has one
    if (e.preview_color != nullptr)
    {
        // pick a live color: if currently editing, try to parse the buffer
        uint32_t swatch = *e.preview_color;
        if (editing)
        {
            // preview the typed value live before confirming
            uint32_t parsed = swatch;
            set_hex(parsed, edit_buffer);  // reuse the helper
            swatch = parsed;
        }

        const int swatch_x = col + 20;  // fixed offset past the value column
        display.setTextBgColor(swatch);
        display.setTextColor(swatch);
        // draw 3 spaces as a filled block
        for (int i = 0; i < 3; i++)
        {
            display.setCursor(swatch_x + i, row);
            display.print(" ");
        }
        display.resetColors();
    }
}

// Simulates the row layout starting from a given scroll offset and returns
// the render_row at which entry `target` would be drawn, or -1 if it would
// fall entirely above the window (shouldn't happen after clamping).
static int row_of_entry(size_t       target,
                        size_t       scroll_offset,
                        int          start_y,
                        int          row_step,
                        const char** out_last_section = nullptr)
{
    int         row          = start_y;
    const char* last_section = nullptr;

    for (size_t i = scroll_offset;; ++i)
    {
        if (i >= ARRAY_SIZE(entries))
            return -1;

        const SettingEntry& e = entries[i];

        if (e.section != nullptr && e.section != last_section)
        {
            last_section = e.section;
            if (i == target)
            {
                // target is a section-header row; report the item row below it
                if (out_last_section)
                    *out_last_section = last_section;
                return row + row_step;
            }
            row += row_step;
        }

        if (i == target)
        {
            if (out_last_section)
                *out_last_section = last_section;
            return row;
        }

        row += row_step;
    }
}

void SettingsScene::ensure_visible()
{
    // Scroll up: selected item is above the current window.
    if (m_selected_item < m_scroll_offset)
    {
        m_scroll_offset = m_selected_item;
        return;
    }

    // Scroll down: advance scroll_offset until the selected item fits.
    const int start_y  = display.pctY(0.3f);
    const int row_step = 2;
    // Reserve space for the footer area (approx 4 rows from the bottom).
    const int max_y = display.getHeight() - 4;

    while (m_scroll_offset < m_selected_item)
    {
        int r = row_of_entry(m_selected_item, m_scroll_offset, start_y, row_step);
        if (r != -1 && r < max_y)
            break;  // selected item fits in the visible area
        ++m_scroll_offset;
    }
}

void SettingsScene::render()
{
    display.clearDisplay();

    // Title
    display.setTextColor(TB_WHITE | TB_BOLD);
    display.setFont(FigletType::FullWidth, "Small Slant");
    display.centerText(display.pctY(0.05f), "Settings");
    display.resetFont();

    // Column layout: labels on left third, values on right ~58%
    const int col_label = display.pctX(0.33f);
    const int col_value = display.pctX(0.58f);

    // Start entries below the figlet title
    const int start_y  = display.pctY(0.25f);
    const int row_step = 2;
    // Stop rendering before the footer area.
    const int max_y = display.getHeight() - 4;

    int         render_row   = start_y;
    const char* last_section = nullptr;

    for (size_t i = m_scroll_offset; i < ARRAY_SIZE(entries); ++i)
    {
        const SettingEntry& e        = entries[i];
        const bool          selected = (i == m_selected_item);
        const bool          editing  = selected && m_editing;

        // Section header. Check that both the header and the first item
        // below it still fit; otherwise stop and show the indicator.
        if (e.section != nullptr && e.section != last_section)
        {
            if (render_row + row_step * 2 > max_y)
                break;

            last_section = e.section;
            display.setTextColor(TB_CYAN | TB_BOLD);
            display.setCursor(col_label, render_row);
            display.print("── {} ──", e.section);
            display.resetColors();
            render_row += row_step;
        }

        // Stop if the item row itself is out of the visible area.
        if (render_row >= max_y)
            break;

        // Label
        if (selected && !editing)
            display.setTextColor(TB_YELLOW | TB_BOLD);

        display.setCursor(col_label + 2, render_row);
        display.print(e.label);
        display.resetColors();

        // Value
        if (selected && !editing)
            display.setTextColor(TB_YELLOW | TB_BOLD);

        render_value(e, selected, editing, m_edit_buffer, col_value, render_row);
        display.resetColors();

        render_row += row_step;
    }

    if (m_editing)
        set_footer("Type to edit | Backspace: Delete | Enter: Confirm | ESC: Cancel");
    else
        set_footer("↑↓: Navigate | ← →: Adjust | Enter: Edit string | ESC: Back");

    display.display();
}

SceneResult SettingsScene::handle_input(uint32_t key)
{
    static const size_t count = ARRAY_SIZE(entries);

    if (m_editing)
    {
        switch (key)
        {
            case TB_KEY_ESC:
                m_editing = false;
                m_edit_buffer.clear();
                break;

            case TB_KEY_ENTER:
            case '\n':
                entries[m_selected_item].set_str_value(m_edit_buffer);
                m_editing = false;
                m_edit_buffer.clear();
                break;

            case TB_KEY_BACKSPACE:
            case TB_KEY_BACKSPACE2:
                if (!m_edit_buffer.empty())
                    m_edit_buffer.pop_back();
                break;

            default:
                if (is_alnum(key))
                    m_edit_buffer += static_cast<char>(key);
                break;
        }

        return Scenes::SettingsMenu;
    }

    // Normal navigation mode
    switch (key)
    {
        case TB_KEY_ESC: return Scenes::MainMenu;

        case TB_KEY_ARROW_UP:
            m_selected_item = (m_selected_item - 1 + count) % count;
            ensure_visible();
            break;
        case TB_KEY_ARROW_DOWN:
            m_selected_item = (m_selected_item + 1) % count;
            ensure_visible();
            break;

        case TB_KEY_ARROW_LEFT:
        case TB_KEY_ARROW_RIGHT:
        {
            const SettingEntry& e = entries[m_selected_item];
            if (e.kind == SettingKind::Float)
                e.adjust(key == TB_KEY_ARROW_LEFT ? -1 : +1);
            else if (e.kind == SettingKind::Bool)
                e.adjust(0);  // lambda just toggles, so direction irrelevant
            break;
        }

        case TB_KEY_ENTER:
        case '\n':
        {
            const SettingEntry& e = entries[m_selected_item];
            if (e.kind == SettingKind::Bool)
            {
                e.adjust(0);  // Enter also toggles bools
            }
            else if (e.kind == SettingKind::String)
            {
                // Pre-populate buffer with the current value so the user edits in place
                m_edit_buffer = e.get_value();
                m_editing     = true;
            }
            break;
        }
    }

    return Scenes::SettingsMenu;
}
