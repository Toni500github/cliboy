#ifndef _TERMINAL_DISPLAY_HPP_
#define _TERMINAL_DISPLAY_HPP_

#include <notcurses/notcurses.h>

#include <cstdint>
#include <format>
#include <string_view>

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
          m_term_bg_channel(0)
    {}
    ~TerminalDisplay();

    bool begin();
    void clearDisplay();
    void setCursor(const int x, const int y);
    void setTextColor(const std::uint32_t& hex);
    void setTextColor(const uint8_t r, const uint8_t g, const uint8_t b);
    void setTextBgColor(const std::uint32_t& hex);
    void setTerminalBgColor(const std::uint32_t& hex);
    void display();

    template <typename... Args>
    void print(const std::string_view fmt, Args&&... args)
    {
        const std::string& text = std::vformat(fmt, std::make_format_args(args...));

        ncplane_set_channels(m_content_plane, m_fg_channel | m_bg_channel);
        ncplane_putstr_yx(m_content_plane, m_cursor_y, m_cursor_x, text.c_str());

        m_cursor_x += text.length();

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
        const std::string& text = std::vformat(fmt, std::make_format_args(args...));

        int x = (m_width - text.length()) / 2;
        x     = std::max(0, x);

        // int saved_x = m_cursor_x;
        // int saved_y = m_cursor_y;

        setCursor(x, y);
        ncplane_set_channels(m_content_plane, m_fg_channel | m_bg_channel);
        ncplane_putstr_yx(m_content_plane, m_cursor_y, m_cursor_x, text.c_str());

        // restore cursor
        // setCursor(saved_x, saved_y);
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
    uint64_t   m_fg_channel, m_bg_channel, m_term_bg_channel;
};

extern TerminalDisplay display;

#endif
