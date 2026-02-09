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

#include <thread>

#include "games/rockpaperscissors.hpp"
#include "games/tictactoe.hpp"
#include "scenes.hpp"
#include "terminal_display.hpp"

TerminalDisplay display;

using namespace std::chrono_literals;

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

bool is_scene_none(const SceneResult& r)
{
    return std::visit([](auto v) { return v == std::decay_t<decltype(v)>::kNone; }, r);
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

    char32_t ch = 0;

    while (running && ch != (char32_t)-1)
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

        ncinput input{};
        ch = notcurses_get_blocking(display.getNC(), &input);

        if (ch == 0)
            continue;

        // Prefer synthesized/special key id when present; otherwise use the Unicode character.
        uint32_t           key    = input.id ? input.id : (uint32_t)ch;
        const SceneResult& result = active_scene->handle_input(key);
        if (!is_scene_none(result))
            current_scene = result;
    }
}

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX 1
#  endif
#  include <fcntl.h>
#  include <io.h>
#  include <windows.h>

static void win_enable_vt_and_raw_input()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Enable VT processing on output
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE && hOut != nullptr)
    {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode))
        {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            mode |= DISABLE_NEWLINE_AUTO_RETURN;
            SetConsoleMode(hOut, mode);
        }
    }

    // Disable echo/line input so terminal replies don't get printed as text
    // and so key input becomes immediate (no line buffering).
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE && hIn != nullptr)
    {
        DWORD mode = 0;
        if (GetConsoleMode(hIn, &mode))
        {
            // Keep extended flags, disable cooked input behaviors
            mode |= ENABLE_EXTENDED_FLAGS;

            mode &= ~ENABLE_ECHO_INPUT;
            mode &= ~ENABLE_LINE_INPUT;
            mode &= ~ENABLE_PROCESSED_INPUT;

            mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

            SetConsoleMode(hIn, mode);
        }
    }
}
#endif

int main()
{
#ifdef _WIN32
    win_enable_vt_and_raw_input();
#endif

    if (!display.begin())
        return 1;

    game_loop();

    return 0;
}
