#pragma once

#include <cstdint>
#include <string>

struct Settings
{
    struct general_settings_t
    {
        std::string assets_path = "./assets";
        bool        utf8        = true;
    } general;

    struct colors_t
    {
        uint32_t black   = 0x1e2127;
        uint32_t red     = 0xe06c75;
        uint32_t green   = 0x98c379;
        uint32_t yellow  = 0xe5c07b;
        uint32_t blue    = 0x61afef;
        uint32_t magenta = 0xc678dd;
        uint32_t cyan    = 0x56b6c2;
        uint32_t white   = 0xdcdfe4;
    } colors;

    struct game_ttt_t
    {
        float delay_show_endgame = 2.0;
        float delay_strike_anim  = 0.05;
    } game_ttt;

    struct game_snake_t
    {
        float snake_min_speed = 40.0f;  // fastest tick floor (ms)
        float snake_max_speed = 80.0f;  // starting tick speed (ms)
    } game_snake;

    struct game_wordle_t
    {
        float       delay_show_final_grid = 0.8;
        float       delay_show_endgame    = 3.0;
        std::string wordle_txt_path       = "./assets/valid-wordle-words.txt";
    } game_wordle;
};

extern Settings settings;
