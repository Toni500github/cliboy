#define TB_IMPL       1
#define TB_OPT_ATTR_W 32
#include "terminal_display.hpp"

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
    tb_shutdown();
}

bool TerminalDisplay::begin()
{
    if (tb_init() < 0)
        return false;

    updateDims();

    tb_hide_cursor();

    return true;
}

void TerminalDisplay::updateDims()
{
    m_width  = tb_width();
    m_height = tb_height();

    m_cursor_x = std::clamp(m_cursor_x, 0, std::max(0, m_width - 1));
    m_cursor_y = std::clamp(m_cursor_y, 0, std::max(0, m_height - 1));
}

void TerminalDisplay::clearDisplay()
{
    updateDims();
    resetColors();
    tb_clear();
    m_cursor_x = 0;
    m_cursor_y = 0;
}

void TerminalDisplay::display()
{
    updateDims();
    tb_present();
}

void TerminalDisplay::resetColors()
{
    m_fg_col = TB_DEFAULT;
    m_bg_col = TB_DEFAULT;
}

void TerminalDisplay::setTextColor(const uint32_t hex)
{
    m_fg_col = (uintattr_t)hex;
}

void TerminalDisplay::setTextBgColor(const uint32_t hex)
{
    m_bg_col = (uintattr_t)hex;
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
        case FigletType::FullWidth: m_figlet.emplace(figlet(m_flf_font, full_width::make_shared())); break;
        case FigletType::Kerning:   m_figlet.emplace(figlet(m_flf_font, kerning::make_shared())); break;
        case FigletType::Smushed:   m_figlet.emplace(figlet(m_flf_font, smushed::make_shared())); break;
    }
}

void TerminalDisplay::resetFont()
{
    m_flf_font = nullptr;
    m_figlet.reset();
}

void TerminalDisplay::drawPixel(int x, int y, unsigned char ch)
{
    updateDims();
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;

    tb_set_cell(x, y, static_cast<uint32_t>(ch), m_fg_col, m_bg_col);
}

void TerminalDisplay::drawLine(int x0, int y0, int x1, int y1, unsigned char ch)
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

void TerminalDisplay::drawCircle(int center_x, int center_y, int radius, unsigned char ch)
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

void TerminalDisplay::drawRect(int x, int y, int width, int height, unsigned char ch)
{
    // Top and bottom horizontal lines
    drawLine(x, y, x + width - 1, y, ch);
    drawLine(x, y + height - 1, x + width - 1, y + height - 1, ch);

    // Left and right vertical lines
    drawLine(x, y, x, y + height - 1, ch);
    drawLine(x + width - 1, y, x + width - 1, y + height - 1, ch);
}

void TerminalDisplay::drawFilledRect(int x, int y, int width, int height, unsigned char ch)
{
    for (int row = y; row < y + height; ++row)
        for (int col = x; col < x + width; ++col)
            drawPixel(col, row, ch);
}
