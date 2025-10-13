#ifndef UTIL_HPP
#define UTIL_HPP

#include <chrono>
#include <cstdint>
#include <string>

#include "settings.hpp"

using namespace std::chrono;

// clang-format off
static inline std::int64_t
    KEY_DOWN_BIT  = (1l << settings.ch_down),
    KEY_LEFT_BIT  = (1l << settings.ch_left),
    KEY_RIGHT_BIT = (1l << settings.ch_right),
    KEY_UP_BIT    = (1l << settings.ch_up),
    KEY_SELECTED  = (1l << '\n'),
    KEY_QUIT      = (1l << settings.ch_quit);

// clang-format on
struct MenuItem
{
    int         id;
    std::string text;
};

void update_button();
void reset_to_main_menu();
void settings_update_key_button(int choice);

extern uint64_t button_state;
extern uint64_t button_pressed;

#endif
