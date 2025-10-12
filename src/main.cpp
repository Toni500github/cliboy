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

uint64_t button_state = 0;

static int choice     = SCENE_MAIN_MENU_SINGLEP;
static int min_choice = SCENE_MAIN_MENU_SINGLEP;
static int max_choice = SCENE_MAIN_MENU_SETTINGS;

TerminalDisplay display;
settings_t      settings;

static void update_key_button(int choice)
{
    struct ncinput ni;

    display.clearDisplay();
    display.centerText(5, "Press any key to assign...");
    display.display();

    // Wait for a single keypress
    uint32_t id = notcurses_get(display.getNC(), nullptr, &ni);

    if (id != (uint32_t)-1 && id != NCKEY_ENTER && id != NCKEY_ESC)
    {
        switch (choice)
        {
            case SCENE_SETTINGS_KEY_UP:    settings.ch_up = id; break;
            case SCENE_SETTINGS_KEY_DOWN:  settings.ch_down = id; break;
            case SCENE_SETTINGS_KEY_LEFT:  settings.ch_left = id; break;
            case SCENE_SETTINGS_KEY_RIGHT: settings.ch_right = id; break;
        }

        display.clearDisplay();
        display.centerText(5, "Assigned: {0:c} (0x{0:x})", id);
        display.display();
        std::this_thread::sleep_for(1s);
    }
}

void update_button(int msdelay)
{
    button_state = 0;

    struct ncinput ni;
    uint32_t       id;

    // Non-blocking poll (returns 0 if no input available)
    while ((id = notcurses_get_nblock(display.getNC(), &ni)) != 0)
    {
        if (id == (uint32_t)-1)
            break;  // error

        const int& idi = static_cast<int>(id);

        if (idi == settings.ch_down || id == NCKEY_DOWN)
            button_state |= KEY_DOWN_BIT;

        else if (idi == settings.ch_left || id == NCKEY_LEFT)
            button_state |= KEY_LEFT_BIT;

        else if (idi == settings.ch_right || id == NCKEY_RIGHT)
            button_state |= KEY_RIGHT_BIT;

        else if (idi == settings.ch_up || id == NCKEY_UP)
            button_state |= KEY_UP_BIT;

        else if (idi == settings.ch_enter || id == NCKEY_ENTER)
            button_state |= KEY_SELECTED;
    }

    if (msdelay > 0)
        std::this_thread::sleep_for(microseconds(msdelay));
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

static auto last_update = steady_clock::now();

void loop()
{
    display.clearDisplay();

    update_button();

    if (button_state & KEY_UP_BIT || button_state & KEY_RIGHT_BIT)
    {
        if (choice > min_choice)
            --choice;
    }
    else if (button_state & KEY_DOWN_BIT || button_state & KEY_LEFT_BIT)
    {
        if (choice < max_choice)
            ++choice;
    }

    load_scene(currentScene, choice);

    if (button_state & KEY_SELECTED)
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
                        max_choice   = GAME_SINGLEP_SNAKE;
                        break;
                    case SCENE_MAIN_MENU_MULTIP:
                        currentScene = SCENE_MULTIP_GAMES;
                        choice       = GAME_MULTIP_RPS;
                        min_choice   = GAME_MULTIP_RPS;
                        max_choice   = GAME_MULTIP_TTT;
                        break;
                    case SCENE_MAIN_MENU_SETTINGS:
                        currentScene = SCENE_SETTINGS;
                        choice       = SCENE_SETTINGS_KEY_UP;
                        min_choice   = SCENE_SETTINGS_KEY_UP;
                        max_choice   = SCENE_SETTINGS_KEY_RIGHT;
                        break;
                }
                break;
            case SCENE_SETTINGS: update_key_button(choice); break;
        }
    }

    display.display();

    auto now = steady_clock::now();
    if (duration_cast<milliseconds>(now - last_update).count() >= 33)
        last_update = now;

    std::this_thread::sleep_for(33ms);
}

int main()
{
    setup();
    while (1)
    {
        loop();
    }

    return 0;
}
