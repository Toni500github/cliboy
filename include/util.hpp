#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>

enum
{
    KEY_DOWN_BIT  = (1 << 1),
    KEY_LEFT_BIT  = (1 << 2),
    KEY_RIGHT_BIT = (1 << 3),
    KEY_UP_BIT    = (1 << 4),
};

void update_button();

extern uint16_t button_state;

#endif
