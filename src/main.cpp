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

#include <clocale>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "scenes.hpp"
#include "terminal_display.hpp"
#include "util.hpp"

uint16_t button_state = 0;

static int choice     = SCENE_MAIN_MENU_SINGLEP;
static int min_choice = SCENE_MAIN_MENU_SINGLEP;
static int max_choice = SCENE_MAIN_MENU_CREDITS;

TerminalDisplay display;

void update_button()
{
    button_state = 0;

    struct ncinput ni;
    uint32_t       id;

    // Non-blocking poll (returns 0 if no input available)
    while ((id = notcurses_get_nblock(display.getNC(), &ni)) != 0)
    {
        if (id == (uint32_t)-1)
            break;  // error

        switch (id)
        {
            case 's':
            case 'S':
            case NCKEY_DOWN: button_state |= KEY_DOWN_BIT; break;

            case 'd':
            case 'D':
            case NCKEY_LEFT: button_state |= KEY_LEFT_BIT; break;

            case 'j':
            case 'J':
            case NCKEY_RIGHT: button_state |= KEY_RIGHT_BIT; break;

            case 'k':
            case 'K':
            case NCKEY_UP: button_state |= KEY_UP_BIT; break;
        }
    }
}

void term_signal_fn(int)
{
    display.clearDisplay();
    printf("Cya later");
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

void loop()
{
    display.clearDisplay();
    update_button();

    if (button_state & KEY_LEFT_BIT)
    {
        if (choice > min_choice)
            --choice;
    }
    else if (button_state & KEY_RIGHT_BIT)
    {
        if (choice < max_choice)
            ++choice;
    }

    load_scene(currentScene, choice);
    usleep(100);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, &term_signal_fn);

    setup();
    while (1)
    {
        loop();
    }

    return 0;
}
