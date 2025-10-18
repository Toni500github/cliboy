/*
 * Copyright 2025 Toni500git
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 * disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <notcurses/notcurses.h>
#include <unistd.h>

#include <chrono>
#include <clocale>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>

#include "scenes.hpp"
#include "settings.hpp"
#include "terminal_display.hpp"
#include "util.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

static int choice     = SCENE_MAIN_MENU_SINGLEP;
static int min_choice = SCENE_MAIN_MENU_SINGLEP;
static int max_choice = SCENE_MAIN_MENU_SETTINGS;

TerminalDisplay display;
settings_t      settings;

void play_singlep_rps();

void reset_to_main_menu()
{
    currentScene = SCENE_MAIN_MENU;
    choice       = SCENE_MAIN_MENU_SINGLEP;
    min_choice   = SCENE_MAIN_MENU_SINGLEP;
    max_choice   = SCENE_MAIN_MENU_SETTINGS;
}

void setup()
{
    srand(time(NULL));
    setlocale(LC_ALL, "");

    if (!display.begin())
    {
        std::cerr << "Couldn't init display\n";
        std::exit(1);
    }
}

bool flag = true;

void loop()
{
    static auto last_frame = steady_clock::now();

    display.clearDisplay();

    update_button();

    if (button_pressed & (KEY_UP_BIT | KEY_LEFT_BIT))
    {
        if (choice > min_choice)
            --choice;
    }
    else if (button_pressed & (KEY_DOWN_BIT | KEY_RIGHT_BIT))
    {
        if (choice < max_choice)
            ++choice;
    }

    load_scene(currentScene, choice);

    if (button_pressed & KEY_QUIT)
    {
        if (currentScene == SCENE_MAIN_MENU)
            flag = false;
        reset_to_main_menu();
        return;
    }
    if (button_pressed & KEY_SELECTED)
    {
        switch (currentScene)
        {
            case SCENE_MAIN_MENU:
                switch (choice)
                {
                    case SCENE_MAIN_MENU_SINGLEP:
                        currentScene = SCENE_SINGLEP_GAMES;
                        choice       = GAME_SINGLEP_RPS;
                        min_choice   = GAME_SINGLEP_RPS;
                        max_choice   = GAME_SINGLEP_RPS;
                        break;
                    case SCENE_MAIN_MENU_SETTINGS:
                        currentScene = SCENE_SETTINGS;
                        choice       = SCENE_SETTINGS_KEY_UP;
                        min_choice   = SCENE_SETTINGS_KEY_UP;
                        max_choice   = SCENE_SETTINGS_KEY_RIGHT;
                        break;
                }
                break;
            case SCENE_SINGLEP_GAMES:
                switch (choice)
                {
                    case GAME_SINGLEP_RPS: currentScene = SCENE_NONE; play_singlep_rps();
                }
                break;
            case SCENE_SETTINGS: settings_update_key_button(choice); break;
        }
    }

    display.display();

    auto now               = steady_clock::now();
    auto frame_time        = duration_cast<milliseconds>(now - last_frame);
    auto target_frame_time = 160ms;

    if (frame_time < target_frame_time)
        std::this_thread::sleep_for(target_frame_time - frame_time);

    last_frame = now;
}

int main()
{
    setup();
    while (flag)
    {
        loop();
    }

    return 0;
}
