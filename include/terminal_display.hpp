#ifndef _TERMINAL_DISPLAY_HPP_
#define _TERMINAL_DISPLAY_HPP_

#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "srilakshmikanthanp/libfiglet.hpp"

#ifdef _WIN32
#  include "termbox2_win.h"
#else
#  include "termbox2.h"
#endif

#include "util.hpp"

using namespace srilakshmikanthanp::libfiglet;

enum class FigletType
{
    FullWidth,
    Kerning,
    Smushed
};

// A similiar clone of Adafruit_SSD130 for terminals
class TerminalDisplay
{
public:
    TerminalDisplay()
        : m_width(0),
          m_height(0),
          m_cursor_x(0),
          m_cursor_y(0),
          m_fg_col(0),
          m_bg_col(0),
          m_flf_font(nullptr),
          m_figlet()
    {}
    ~TerminalDisplay();

    bool begin();
    void clearDisplay();
    void setCursor(const int x, const int y);
    void setTextColor(const uint32_t hex);
    void setTextBgColor(const uint32_t hex);
    void resetColors();
    void setFont(FigletType figlet_type, const std::string_view font);
    void resetFont();
    void updateDims();
    void drawLine(int x0, int y0, int x1, int y1, unsigned char ch);
    void drawCircle(int center_x, int center_y, int radius, unsigned char ch);
    void drawRect(int x, int y, int width, int height, unsigned char ch);
    void drawFilledRect(int x, int y, int width, int height, unsigned char ch);
    void drawPixel(int x, int y, unsigned char ch);
    void display();

    template <typename... Args>
    void print(const std::string_view fmt, Args&&... args)
    {
        const std::string& text = m_figlet ? (*m_figlet)(std::vformat(fmt, std::make_format_args(args...)))
                                           : std::vformat(fmt, std::make_format_args(args...));

        const std::vector<std::string>& text_lines = split(text, '\n');

        int max_width = 0;
        for (const auto& line : text_lines)
        {
            tb_print(m_cursor_x, m_cursor_y, m_fg_col, m_bg_col, line.c_str());
            m_cursor_y++;
            max_width = std::max<int>(max_width, line.size());
        }

        m_cursor_x += max_width;

        if (m_cursor_x >= static_cast<int>(m_width))
        {
            m_cursor_x = 0;
            m_cursor_y++;
            if (m_cursor_y >= static_cast<int>(m_height))
                m_cursor_y = m_height - 1;
        }
    }

    template <typename... Args>
    void centerText(int y, const std::string_view fmt, Args&&... args)
    {
        const std::string& text = m_figlet ? (*m_figlet)(std::vformat(fmt, std::make_format_args(args...)))
                                           : std::vformat(fmt, std::make_format_args(args...));

        const std::vector<std::string>& text_lines = split(text, '\n');
        size_t                          max_width  = 0;
        for (const auto& line : text_lines)
            max_width = std::max(max_width, line.size());

        int current_y = y;
        for (const auto& line : text_lines)
        {
            int x = (m_width - static_cast<int>(line.size())) / 2;
            x     = std::max(0, x);

            tb_print(x, current_y++, m_fg_col, m_bg_col, line.c_str());
            setCursor(x, current_y);
        }
    }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getCursorX() const { return m_cursor_x; }
    int getCursorY() const { return m_cursor_y; }

private:
    int        m_width, m_height;
    int        m_cursor_x, m_cursor_y;
    uintattr_t m_fg_col, m_bg_col;

    std::shared_ptr<flf_font> m_flf_font;
    std::optional<figlet>     m_figlet;
};

extern TerminalDisplay display;

#endif
