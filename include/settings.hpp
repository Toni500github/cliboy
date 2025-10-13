#ifndef _SETTINGS_HPP_
#define _SETTINGS_HPP_

#include <notcurses/nckeys.h>

struct settings_t
{
    int ch_up    = 'w';
    int ch_left  = 'a';
    int ch_down  = 's';
    int ch_right = 'd';
    int ch_quit  = 'q';
};

extern settings_t settings;

#endif  // !_SETTINGS_HPP_
