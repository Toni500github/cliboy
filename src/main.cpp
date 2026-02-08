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

#include "games/tictactoe.hpp"
#include "games/rockpaperscissors.hpp"
#include "scenes.hpp"
#include "terminal_display.hpp"
#include "util.hpp"

TerminalDisplay display;

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

static timespec add_ms(timespec t, long ms)
{
    t.tv_nsec += ms * 1000000L;
    t.tv_sec += t.tv_nsec / 1000000000L;
    t.tv_nsec %= 1000000000L;
    return t;
}

bool is_scene_none(const SceneResult& r)
{
    return std::visit([](auto v) { return v == std::decay_t<decltype(v)>::kNone; }, r);
}

Result<notcurses*> init_notcurses()
{
    notcurses_options opts = {};
    opts.flags             = NCOPTION_SUPPRESS_BANNERS | NCOPTION_NO_ALTERNATE_SCREEN;

    notcurses* nc = notcurses_init(&opts, nullptr);
    if (!nc)
        return Err("Failed to initialize notcurses");

    return Ok(nc);
}

void game_loop()
{
    MainMenuScene  main_menu;
    GamesMenuScene games_menu;
    CreditsScene   credits;
    TTTScene       game_ttt_scene;
    RpsScene       game_rps_scene;

    SceneResult current_scene = Scenes::MainMenu;
    bool        running       = true;

    while (running)
    {
        Scene* active_scene = nullptr;

        // clang-format off
        std::visit(overloaded{ 
            [&](Scenes s) {                      
                switch (s)                      
                {                      
                    case Scenes::MainMenu: active_scene = &main_menu; break;
                    case Scenes::Games:    active_scene = &games_menu; break;
                    case Scenes::Credits:  active_scene = &credits; break;

                    case Scenes::Exit:
                    default:           running = false; break;
                }
            },
            [&](ScenesGame s) {
                switch (s)
                {
                    case ScenesGame::RockPaperScissors: active_scene = &game_rps_scene; break;
                    case ScenesGame::TicTacToe: active_scene = &game_ttt_scene; break;
                    default:               running = false; break;
                }
            }
        }, current_scene);
        // clang-format on

        if (!running || !active_scene)
            break;

        active_scene->render();

        ncinput  input{};
        timespec now{};
        clock_gettime(CLOCK_MONOTONIC, &now);
        timespec deadline = add_ms(now, 16);  // ~60Hz tick

        uint32_t key = notcurses_get(display.getNC(), &deadline, &input);

        // on timeout, key == 0 => loop continues, render will run again
        if (key == 0)
            continue;

        const SceneResult& result = active_scene->handle_input(key);
        if (!is_scene_none(result))
            current_scene = result;
    }
}

int main()
{
    if (!display.begin())
        return 1;

    game_loop();

    return 0;
}
