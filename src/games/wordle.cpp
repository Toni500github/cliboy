#include "games/wordle.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>
#include "terminal_display.hpp"

static std::string buf;
static std::string guess;
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
    char ch;
    TileState     state;
};

using RowStates = std::array<TileState, 5>;
using WordleStates = std::array<std::array<Tile, 5>, 6>;

static RowStates get_states(const std::string& str)
{
    RowStates states;
    for (size_t i = 0; i < str.size(); ++i)
    {
        char c = str[i];
        if (guess[i] == c)
            states[i] = TileState::Correct;
        else if (guess.find(c) != guess.npos)
            states[i] = TileState::Present;
        else
            states[i] = TileState::Absent;
    }
    return states;
}

static uintattr_t bg_for(TileState s)
{
    switch (s)
    {
        case TileState::Correct: return TB_GREEN;
        case TileState::Present: return TB_YELLOW;
        case TileState::Absent:  return TB_BLACK;
        case TileState::Empty:   return TB_WHITE;
    }
}

static uintattr_t fg_for(TileState s)
{
    switch (s)
    {
        case TileState::Present:
        case TileState::Empty:
        case TileState::Correct:
            return TB_BLACK | TB_BOLD;

        case TileState::Absent:
            return TB_WHITE | TB_BOLD;
    }
}

static bool is_valid(const std::string& word)
{
    if (words.empty())
        return false;

    return std::binary_search(words.begin(), words.end(), word);
}

static void draw_wordle_grid(const WordleStates& grid)
{
    display.clearDisplay();

    const uint32_t block = U'█';
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
            display.setTextColor(bg_for(t.state));
            display.setTextBgColor(bg_for(t.state));
            display.drawFilledRect(x, y, cell_w, cell_h, block);

            // Border
            display.drawRect(x, y, cell_w, cell_h, block);

            // Center letter
            if (isalpha(t.ch))
            {
                const int cx = x + cell_w / 2;
                const int cy = y + cell_h / 2;

                display.setTextColor(fg_for(t.state));
                display.setTextBgColor(bg_for(t.state));
                display.setCursor(cx, cy);
                display.print("{}", t.ch);
            }
        }
    }

    display.resetColors();
    display.display();
}

Result<> WordleGame::on_begin()
{
    std::ifstream f("./assets/valid-wordle-words.txt");
    if (f.bad())
        return Err("Failed to open ./assets/valid-wordle-words.txt");

    std::string word;
    while (std::getline(f, word))
        words.push_back(word);

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(1, words.size());

    guess = str_toupper(words[dist(gen)]);

    return Ok();
}

void WordleGame::render()
{
    static WordleStates grid{};
    static int row{};

    if (!selected)
    {
        for (int c = 0; c < 5; ++c)
        {
            grid[row][c].ch    = c < static_cast<int>(buf.size()) ? buf[c] : ' ';
            grid[row][c].state = TileState::Empty;
        }
    }
    else
    {
        if (!is_valid(str_tolower(buf)))
        {
            selected = false;
            return;
        }

        const auto& states = get_states(buf);
        for (int c = 0; c < 5; ++c)
        {
            grid[row][c].ch    = buf[c];
            grid[row][c].state = states[c];
        }
        row++;
        selected = false;
        buf.clear();
    }

    // Remaining empty rows
    for (int r = row; r < 6; ++r)
        if (grid[r].empty())
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
    else if (!buf.empty() && (key == TB_KEY_BACKSPACE || key == TB_KEY_BACKSPACE2 || key == 8))
        buf.pop_back();

    return ScenesGame::Wordle;
}
