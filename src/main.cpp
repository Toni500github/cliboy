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

#include <cstdlib>
#include <iostream>

#include "games/rockpaperscissors.hpp"
#include "games/tictactoe.hpp"
#include "games/wordle.hpp"
#include "scenes.hpp"
#include "settings.hpp"
#include "terminal_display.hpp"

TerminalDisplay display;
Settings        settings;

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

int game_loop()
{
    MainMenuScene  main_menu;
    GamesMenuScene games_menu;
    CreditsScene   credits;
    SettingsScene  settings_menu;
    TTTGame        game_ttt;
    RpsGame        game_rps;
    WordleGame     game_wordle;

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
                    case Scenes::MainMenu:     active_scene = &main_menu; break;
                    case Scenes::GamesMenu:    active_scene = &games_menu; break;
                    case Scenes::Credits:      active_scene = &credits; break;
                    case Scenes::SettingsMenu: active_scene = &settings_menu; break;
                    default:                   running = false; break;
                }
            },
            [&](ScenesGame s) {
                switch (s)
                {
                    case ScenesGame::RockPaperScissors: active_scene = &game_rps; break;
                    case ScenesGame::TicTacToe:         active_scene = &game_ttt; break;
                    case ScenesGame::Wordle:            active_scene = &game_wordle; break;
                    default:                            running = false; break;
                }
            }
        }, current_scene);
        // clang-format on

        if (!running || !active_scene)
            break;

        const Result<>& r = active_scene->begin();
        if (!r.ok())
        {
            tb_shutdown();
            std::cerr << "Error while initing a scene/game: " << r.error().value << std::endl;
            return 1;
        }

        active_scene->render_all();

        tb_event ev;
        tb_peek_event(&ev, active_scene->frame_ms());

        uint32_t key = 0;
        if (ev.type == TB_EVENT_KEY)
            key = ev.key ? ev.key : ev.ch;

        current_scene = active_scene->handle_input(key);
    }
    return 0;
}

void exit()
{
    tb_shutdown();
}

int main()
{
    if (!display.begin())
        return 1;

    std::atexit(exit);
    return game_loop();
}
