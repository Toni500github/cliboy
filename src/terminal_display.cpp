#include "terminal_display.hpp"

#include <notcurses/notcurses.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>

#include "srilakshmikanthanp/libfiglet.hpp"
using namespace srilakshmikanthanp::libfiglet;

TerminalDisplay::~TerminalDisplay()
{
    clearDisplay();

    if (m_content_plane)
    {
        ncplane_destroy(m_content_plane);
        m_content_plane = nullptr;
    }

    if (m_nc)
    {
        notcurses_stop(m_nc);
        m_nc = nullptr;
    }
}

bool TerminalDisplay::begin()
{
    m_nc = notcurses_init(nullptr, nullptr);
    if (!m_nc)
        return false;

    m_stdplane = notcurses_stdplane(m_nc);
    m_width    = ncplane_dim_x(m_stdplane);
    m_height   = ncplane_dim_y(m_stdplane);

    struct ncplane_options ncopts = {
        .y        = 0,
        .x        = 0,
        .rows     = m_height,
        .cols     = m_width,
        .userptr  = nullptr,
        .name     = "Content plane (debug ig)",
        .resizecb = nullptr,
        .flags    = 0,
    };

    m_content_plane = ncplane_create(m_stdplane, &ncopts);
    if (!m_content_plane)
    {
        notcurses_stop(m_nc);
        return false;
    }
    // m_content_plane = m_stdplane;

    return true;
}

void TerminalDisplay::clearDisplay()
{
    ncplane_set_fg_default(m_content_plane);
    ncplane_set_bg_default(m_content_plane);
    ncplane_erase(m_content_plane);
    m_cursor_x = m_cursor_y = 0;
}

void TerminalDisplay::display()
{
    notcurses_render(m_nc);
}

void TerminalDisplay::resetColors()
{
    ncchannels_set_fg_default(&m_channels);
    ncchannels_set_bg_default(&m_channels);
    ncplane_set_channels(m_content_plane, m_channels);
}

void TerminalDisplay::setTextColor(const uint32_t hex)
{
    uint8_t r = (hex >> 16) & 0xff;
    uint8_t g = (hex >> 8) & 0xff;
    uint8_t b = (hex) & 0xff;

    ncchannels_set_fg_rgb8(&m_channels, r, g, b);
    ncplane_set_channels(m_content_plane, m_channels);
}

void TerminalDisplay::setTextBgColor(const uint32_t hex)
{
    uint8_t r = (hex >> 16) & 0xff;
    uint8_t g = (hex >> 8) & 0xff;
    uint8_t b = (hex) & 0xff;

    ncchannels_set_bg_rgb8(&m_channels, r, g, b);
    ncplane_set_channels(m_content_plane, m_channels);
}

void TerminalDisplay::setCursor(const int x, const int y)
{
    m_cursor_x = std::clamp<int>(x, 0, m_width - 1);
    m_cursor_y = std::clamp<int>(y, 0, m_height - 1);
}

void TerminalDisplay::setFont(FigletType figlet_type, const std::string_view font)
{
    m_flf_font = flf_font::make_shared(std::format("./assets/fonts/{}.flf", font));
    if (!m_flf_font)
    {
        std::cerr << "Failed to open font '" << font << "' at path ./assets/fonts";
        std::exit(-1);
    }

    switch (figlet_type)
    {
        case FIGLET_FULL_WIDTH: m_figlet.emplace(figlet(m_flf_font, full_width::make_shared())); break;
        case FIGLET_KERNING:    m_figlet.emplace(figlet(m_flf_font, kerning::make_shared())); break;
        case FIGLET_SMUSHED:    m_figlet.emplace(figlet(m_flf_font, smushed::make_shared())); break;
    }
}

void TerminalDisplay::resetFont()
{
    m_flf_font = nullptr;
    m_figlet.reset();
}

void TerminalDisplay::drawPixel(int x, int y, char ch)
{
    if (x >= 0 && x < static_cast<int>(m_width) && y >= 0 && y < static_cast<int>(m_height))
    {
        int saved_x = m_cursor_x;
        int saved_y = m_cursor_y;

        setCursor(x, y);
        ncplane_putchar_yx(m_content_plane, y, x, ch);

        setCursor(saved_x, saved_y);
    }
}

void TerminalDisplay::drawLine(int x0, int y0, int x1, int y1, char ch)
{
    // Bresenham's line algorithm
    int dx  = abs(x1 - x0);
    int dy  = abs(y1 - y0);
    int sx  = (x0 < x1) ? 1 : -1;
    int sy  = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        drawPixel(x0, y0, ch);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void TerminalDisplay::drawCircle(int center_x, int center_y, int radius, char ch)
{
    // Midpoint circle algorithm
    int x   = radius;
    int y   = 0;
    int err = 0;

    while (x >= y)
    {
        drawPixel(center_x + x, center_y + y, ch);
        drawPixel(center_x + y, center_y + x, ch);
        drawPixel(center_x - y, center_y + x, ch);
        drawPixel(center_x - x, center_y + y, ch);
        drawPixel(center_x - x, center_y - y, ch);
        drawPixel(center_x - y, center_y - x, ch);
        drawPixel(center_x + y, center_y - x, ch);
        drawPixel(center_x + x, center_y - y, ch);

        if (err <= 0)
        {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void TerminalDisplay::drawRect(int x, int y, int width, int height, char ch)
{
    // Top and bottom horizontal lines
    drawLine(x, y, x + width - 1, y, ch);
    drawLine(x, y + height - 1, x + width - 1, y + height - 1, ch);

    // Left and right vertical lines
    drawLine(x, y, x, y + height - 1, ch);
    drawLine(x + width - 1, y, x + width - 1, y + height - 1, ch);
}

void TerminalDisplay::drawFilledRect(int x, int y, int width, int height, char ch)
{
    for (int row = y; row < y + height; ++row)
        for (int col = x; col < x + width; ++col)
            drawPixel(col, row, ch);
}
