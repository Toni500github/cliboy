#ifndef _TERMINAL_DISPLAY_HPP_
#define _TERMINAL_DISPLAY_HPP_

#include <notcurses/notcurses.h>

#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "srilakshmikanthanp/libfiglet.hpp"
#include "util.hpp"
using namespace srilakshmikanthanp::libfiglet;

enum FigletType
{
    FIGLET_FULL_WIDTH,
    FIGLET_KERNING,
    FIGLET_SMUSHED
};

// A similiar clone of Adafruit_SSD130 for terminals
class TerminalDisplay
{
public:
    TerminalDisplay()
        : m_nc(nullptr),
          m_stdplane(nullptr),
          m_content_plane(nullptr),
          m_width(0),
          m_height(0),
          m_cursor_x(0),
          m_cursor_y(0),
          m_fg_channel(0),
          m_bg_channel(0),
          m_term_bg_channel(0),
          m_flf_font(nullptr),
          m_figlet()
    {}
    ~TerminalDisplay();

    bool begin();
    void clearDisplay();
    void setCursor(const int x, const int y);
    void setTextColor(const std::uint32_t& hex);
    void setTextColor(const uint8_t r, const uint8_t g, const uint8_t b);
    void setTextBgColor(const std::uint32_t& hex);
    void setTerminalBgColor(const std::uint32_t& hex);
    void setFont(FigletType figlet_type, const std::string_view font);
    void resetFont();
    void drawLine(int x0, int y0, int x1, int y1, char ch = ' ');
    void drawCircle(int center_x, int center_y, int radius, char ch = ' ');
    void drawRect(int x, int y, int width, int height, char ch = ' ');
    void drawFilledRect(int x, int y, int width, int height, char ch = ' ');
    void drawPixel(int x, int y, char ch);
    void display();

    template <typename... Args>
    void print(const std::string_view fmt, Args&&... args)
    {
        const std::string& text = m_figlet ? (*m_figlet)(std::vformat(fmt, std::make_format_args(args...)))
                                           : std::vformat(fmt, std::make_format_args(args...));

        const std::vector<std::string>& text_lines = split(text, '\n');

        ncplane_set_fg_alpha(m_content_plane, NCALPHA_OPAQUE);
        ncplane_set_bg_alpha(m_content_plane, NCALPHA_TRANSPARENT);
        ncplane_set_fg_rgb8(m_content_plane, ncchannel_r(m_fg_channel), ncchannel_g(m_fg_channel),
                            ncchannel_b(m_fg_channel));

        int max_width = 0;
        for (const auto& line : text_lines)
        {
            ncplane_putstr_yx(m_content_plane, m_cursor_y, m_cursor_x, line.c_str());
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

        ncplane_set_fg_alpha(m_content_plane, NCALPHA_OPAQUE);
        ncplane_set_bg_alpha(m_content_plane, NCALPHA_TRANSPARENT);
        ncplane_set_fg_rgb8(m_content_plane, ncchannel_r(m_fg_channel), ncchannel_g(m_fg_channel),
                            ncchannel_b(m_fg_channel));

        int current_y = y;
        for (const auto& line : text_lines)
        {
            int x = (m_width - static_cast<int>(line.size())) / 2;
            x     = std::max(0, x);
            ncplane_putstr_yx(m_content_plane, current_y++, x, line.c_str());
        }
        setCursor(m_cursor_x, current_y);
    }

    uint       getWidth() const { return m_width; }
    uint       getHeight() const { return m_height; }
    int        getCursorX() const { return m_cursor_x; }
    int        getCursorY() const { return m_cursor_y; }
    notcurses* getNC() const { return m_nc; }

private:
    notcurses* m_nc;
    ncplane *  m_stdplane, *m_content_plane;
    uint       m_width, m_height;
    int        m_cursor_x, m_cursor_y;
    uint32_t   m_fg_channel, m_bg_channel, m_term_bg_channel;

    std::shared_ptr<flf_font> m_flf_font;
    std::optional<figlet>     m_figlet;
};

extern TerminalDisplay display;

#endif
