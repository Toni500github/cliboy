#include "settings.hpp"

#include <array>
#include <format>
#include <functional>
#include <string>

#include "scenes.hpp"
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
    std::function<void(const std::string&)> set_value;  // String only; nullptr for Float/Bool
};

static std::string fmt_float(float v)
{
    return std::format("{:.2f}s", v);
}

static std::string fmt_bool(bool v)
{
    return v ? "On" : "Off";
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
static const std::array<SettingEntry, 8> entries = {{
    // Rock Paper Scissors
    {
        "Rock Paper Scissors",
        "Countdown Delay",
        SettingKind::Float,
        [] { return fmt_float(settings.game_rps.delay_countdown); },
        [](int d) { clamp_float(settings.game_rps.delay_countdown, 0.05f, 0.05f, 5.0f, d); },
        nullptr
    },
    {
        nullptr,
        "Show Winner Delay",
        SettingKind::Float,
        [] { return fmt_float(settings.game_rps.delay_show_winner); },
        [](int d) { clamp_float(settings.game_rps.delay_show_winner, 0.1f, 0.1f, 10.0f, d); },
        nullptr
    },
    {
        nullptr,
        "Toggle bool - Dummy",
        SettingKind::Bool,
        [] { return fmt_bool(settings.dummy); },
        [](int) { settings.dummy = !settings.dummy; },
        nullptr
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
}};
// clang-format on

static void render_value(const SettingEntry& e, bool selected, bool editing, const std::string& edit_buffer, int col,
                         int row)
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
                // Show the live edit buffer with a block cursor appended
                display.setTextColor(TB_YELLOW | TB_BOLD);
                display.print("{}\u2588", edit_buffer);  // '█'
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
}

void SettingsScene::render()
{
    display.clearDisplay();

    const int cols = display.getWidth();
    const int rows = display.getHeight();

    // Title
    display.setFont(FigletType::FullWidth, "Small Slant");
    display.centerText(2, "Settings");
    display.resetFont();

    // Column layout: labels on left third, values on right third
    const int col_label = cols / 3;
    const int col_value = cols * 3.5 / 6;
    const int start_y   = 10;
    const int row_step  = 2;

    int         render_row   = start_y;
    const char* last_section = nullptr;

    for (size_t i = 0; i < entries.size(); ++i)
    {
        const SettingEntry& e        = entries[i];
        const bool          selected = (i == m_selected_item);
        const bool          editing  = selected && m_editing;

        // Section header
        if (e.section != nullptr && e.section != last_section)
        {
            last_section = e.section;
            display.setTextColor(TB_CYAN | TB_BOLD);
            display.setCursor(col_label, render_row);
            display.print("── {} ──", e.section);
            display.resetColors();
            render_row += row_step;
        }

        // Label
        if (selected && !editing)
            display.setTextColor(TB_WHITE | TB_BOLD);

        display.setCursor(col_label + 2, render_row);
        display.print(e.label);
        display.resetColors();

        // Value
        if (selected && !editing)
            display.setTextColor(TB_WHITE | TB_BOLD);

        render_value(e, selected, editing, m_edit_buffer, col_value, render_row);
        display.resetColors();

        render_row += row_step;
    }

    if (m_editing)
        display.centerText(rows - 2, "Type to edit | Backspace: Delete | Enter: Confirm | ESC: Cancel");
    else
        display.centerText(rows - 2, "↑↓: Navigate | ← →: Adjust | Enter: Edit string | ESC: Back");

    display.display();
}

SceneResult SettingsScene::handle_input(uint32_t key)
{
    static const size_t count = entries.size();

    if (m_editing)
    {
        switch (key)
        {
            case 27:
                m_editing = false;
                m_edit_buffer.clear();
                break;

            case TB_KEY_ENTER:
            case '\n':  // confirm
                entries[m_selected_item].set_value(m_edit_buffer);
                m_editing = false;
                m_edit_buffer.clear();
                break;

            case TB_KEY_BACKSPACE:
            case TB_KEY_BACKSPACE2:
                if (!m_edit_buffer.empty())
                    m_edit_buffer.pop_back();
                break;

            default:
                // printable ASCII only
                if (key >= 0x20 && key <= 0x7E)
                    m_edit_buffer += static_cast<char>(key);
                break;
        }

        return Scenes::SettingsMenu;
    }

    // Normal navigation mode
    switch (key)
    {
        case 27: return Scenes::MainMenu;

        case TB_KEY_ARROW_UP: m_selected_item = (m_selected_item - 1 + count) % count; break;

        case TB_KEY_ARROW_DOWN: m_selected_item = (m_selected_item + 1) % count; break;

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
