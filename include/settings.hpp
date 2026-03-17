#pragma once

#include <string>

struct Settings
{
    struct general_settings_t
    {
        std::string assets_path = "./assets";
    } general;

    struct game_rps_t
    {
        float delay_countdown   = 0.8;
        float delay_show_winner = 2.0;
    } game_rps;

    struct game_ttt_t
    {
        float delay_show_endgame = 2.0;
        float delay_strike_anim  = 0.05;
    } game_ttt;

    struct game_wordle_t
    {
        float       delay_show_final_grid = 0.8;
        float       delay_show_endgame    = 3.0;
        std::string wordle_txt_path       = "./assets/valid-wordle-words.txt";
    } game_wordle;
};

extern Settings settings;
