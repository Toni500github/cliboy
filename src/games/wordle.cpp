#include "games/wordle.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "terminal_display.hpp"

static std::string buf;
static std::string guess;
static size_t pos{};
static std::vector<std::string> words;
static bool selected{};

enum class TileState
{
    Empty,
    Absent,   // gray
    Present,  // yellow
    Correct   // green
};

struct Tile
{
    char      ch;
    TileState state;
};

static std::array<TileState, 5> get_states(const std::string& str)
{
    std::array<TileState, 5> states;
    for (size_t i = 0; i < str.size(); ++i)
    {
        char c = str[i];
        if (guess[i] == c)
            states[i] = TileState::Correct;
        else if (std::count(guess.begin(), guess.end(), c) > 0)
            states[i] = TileState::Present;
        else
            states[i] = TileState::Absent;
    }
    return states;
}

static uint32_t bg_for(TileState s)
{
    switch (s)
    {
        case TileState::Correct: return TB_GREEN;
        case TileState::Present: return TB_YELLOW;
        case TileState::Absent:  return TB_REVERSE;
        case TileState::Empty:   return TB_BLACK;
    }
    return TB_DEFAULT;
}

static void draw_wordle_grid(const std::array<std::array<Tile, 5>, 6>& grid)
{
    display.clearDisplay();

    const int cols = 5;
    const int rows = 6;

    const int cell_w   = 5;  // width in characters
    const int cell_h   = 3;  // height in characters
    const int gap_x    = 1;
    const int gap_y    = 1;

    const int grid_w = cols * cell_w + (cols - 1) * gap_x;
    const int grid_h = rows * cell_h + (rows - 1) * gap_y;

    const int start_x = (display.getWidth()  - grid_w) / 2;
    const int start_y = (display.getHeight() - grid_h) / 2;

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            const int x = start_x + c * (cell_w + gap_x);
            const int y = start_y + r * (cell_h + gap_y);

            const Tile& t = grid[r][c];

            // Fill
            display.setTextColor(TB_WHITE);
            display.setTextBgColor(bg_for(t.state));
            display.drawFilledRect(x, y, cell_w, cell_h, ' ');

            // Border
            display.setTextColor(TB_BLACK);
            display.setTextBgColor(bg_for(t.state));
            display.drawRect(x, y, cell_w, cell_h, '#');

            // Center letter
            if (t.ch != '\0' && t.ch != ' ')
            {
                const int cx = x + cell_w / 2;
                const int cy = y + cell_h / 2;

                display.setTextColor(TB_WHITE);
                display.setTextBgColor(bg_for(t.state));
                display.setCursor(cx, cy);
                display.print("{}", t.ch);
            }
        }
    }

    display.resetColors();
    display.display();
}

WordleGame::WordleGame()
{
    std::ifstream f("./assets/valid-wordle-words.txt");
    if (!f.good())
    {
        tb_shutdown();
        std::cerr << "failed to open valid-wordle-words.txt" << std::endl;
        exit(-1);
    }

    std::string word;
    while (std::getline(f, word))
        words.push_back(word);

    guess = words[rand() % words.size()];
    for (char& c : guess)
        c = toupper(c);
}

void WordleGame::render()
{
    static std::array<std::array<Tile, 5>, 6> grid{};
    static int row{};
    auto set_row = [&](int& r, const char* letters, const std::array<TileState, 5>& states) {
        for (int c = 0; c < 5; ++c)
        {
            grid[r][c].ch    = letters[c];
            grid[r][c].state = states[c];
        }
        r++;
    };

    if (selected)
    {
        set_row(row, buf.c_str(), get_states(buf));
        selected = false;
        buf.clear();
    }

    // Remaining empty rows
    for (int r = row; r < 6; ++r)
        for (int c = 0; c < 5; ++c)
            grid[r][c] = Tile{' ', TileState::Empty};

    draw_wordle_grid(grid);
}

SceneResult WordleGame::handle_input(uint32_t key)
{
    if (key == 27)
        return Scenes::Games;
    else if (buf.size() == 5 && (key == TB_KEY_ENTER || key == '\n'))
        selected = true;
    else if (buf.size() < 5 && isalpha(key))
        buf.push_back(toupper(key));

    return ScenesGame::Wordle;
}
