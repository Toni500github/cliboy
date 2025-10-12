#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <string>

#include "settings.hpp"

// clang-format off
static inline std::int64_t
    KEY_DOWN_BIT  = (1l << settings.ch_down),
    KEY_LEFT_BIT  = (1l << settings.ch_left),
    KEY_RIGHT_BIT = (1l << settings.ch_right),
    KEY_UP_BIT    = (1l << settings.ch_up),
    KEY_SELECTED  = (1l << settings.ch_enter);

// clang-format on
struct MenuItem
{
    int         id;
    std::string text;
};

void update_button(int msdelay = 0);

extern uint64_t button_state;

#endif
