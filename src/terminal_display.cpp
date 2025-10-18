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

    return true;
}

void TerminalDisplay::clearDisplay()
{
    // set both foreground and background channels
    ncplane_set_channels(m_content_plane, m_fg_channel | m_bg_channel);
    ncplane_erase(m_content_plane);
    m_cursor_x = m_cursor_y = 0;
}

// clang-format off
void TerminalDisplay::display()
{ notcurses_render(m_nc); }

// clang-format on
void TerminalDisplay::setTextColor(const std::uint32_t& hex)
{
    uint8_t r = (hex >> 16) & 0xff;
    uint8_t g = (hex >> 8) & 0xff;
    uint8_t b = (hex) & 0xff;
    setTextColor(r, g, b);
}

void TerminalDisplay::setTextColor(const uint8_t r, const uint8_t g, const uint8_t b)
{
    m_fg_channel = 0;
    ncchannel_set_rgb8(&m_fg_channel, r, g, b);
    ncchannel_set_alpha(&m_fg_channel, NCALPHA_OPAQUE);

    ncplane_set_fg_rgb8(m_content_plane, r, g, b);
}

void TerminalDisplay::setTextBgColor(const std::uint32_t& hex)
{
    uint8_t r = (hex >> 16) & 0xff;
    uint8_t g = (hex >> 8) & 0xff;
    uint8_t b = (hex) & 0xff;

    m_bg_channel = 0;
    ncchannel_set_rgb8(&m_bg_channel, r, g, b);
    ncchannel_set_alpha(&m_bg_channel, NCALPHA_TRANSPARENT);

    ncplane_set_bg_rgb8(m_content_plane, r, g, b);
}

void TerminalDisplay::setTerminalBgColor(const std::uint32_t& hex)
{
    uint8_t r         = (hex >> 16) & 0xff;
    uint8_t g         = (hex >> 8) & 0xff;
    uint8_t b         = (hex) & 0xff;
    m_term_bg_channel = NCCHANNELS_INITIALIZER(0, 0, 0, r, g, b);
    ncplane_set_bg_rgb8(m_stdplane, r, g, b);
}

void TerminalDisplay::setCursor(const int x, const int y)
{
    m_cursor_x = std::clamp<int>(x, 0, m_width - 1);
    m_cursor_y = std::clamp<int>(y, 0, m_height - 1);
}

void TerminalDisplay::setFont(bool full_width, const std::string_view font)
{
    m_flf_font = flf_font::make_shared(std::format("./assets/fonts/{}.flf", font));
    if (!m_flf_font)
    {
        std::cerr << "Failed to open font '" << font << "' at path ./assets/fonts";
        std::exit(-1);
    }

    if (full_width)
        m_figlet.emplace(figlet(m_flf_font, full_width::make_shared()));
    else
        m_figlet.emplace(figlet(m_flf_font, kerning::make_shared()));
}

void TerminalDisplay::resetFont()
{
    m_flf_font = nullptr;
    m_figlet.reset();
}
