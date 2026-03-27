#include "games/wordle.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <random>
#include <string>
#include <thread>

#include "audio_player.hpp"
#include "settings.hpp"
#include "terminal_display.hpp"

RowStates WordleGame::get_states(const std::string& str)
{
    RowStates states;
    states.fill(TileState::Absent);

    // Track how many of each letter in the answer are still "available"
    // to be matched as Present (i.e. not already consumed by a Correct match)
    int letter_count[26] = {};

    // Pass 1: mark Correct tiles and deduct from available counts
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == m_guess[i])
            states[i] = TileState::Correct;
        else
            letter_count[m_guess[i] - 'A']++;  // still available for Present
    }

    // Pass 2: mark Present tiles, consuming available letters
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (states[i] == TileState::Correct)
            continue;

        int& available = letter_count[str[i] - 'A'];
        if (available > 0)
        {
            states[i] = TileState::Present;
            available--;
        }
    }

    return states;
}

uintattr_t WordleGame::bg_for(TileState s)
{
    switch (s)
    {
        case TileState::Correct: return TB_GREEN;
        case TileState::Present: return TB_YELLOW;
        case TileState::Absent:  return TB_BLACK;
        case TileState::Empty:   return TB_WHITE;
    }
    return TB_DEFAULT;  // silence -Wreturn-type
}

uintattr_t WordleGame::fg_for(TileState s)
{
    switch (s)
    {
        case TileState::Present:
        case TileState::Empty:
        case TileState::Correct: return TB_BLACK | TB_BOLD;

        case TileState::Absent: return TB_WHITE | TB_BOLD;
    }
    return TB_DEFAULT;  // silence -Wreturn-type
}

bool WordleGame::is_valid(const std::string& word)
{
    if (m_words_list.empty())
        return false;

    return std::binary_search(m_words_list.begin(), m_words_list.end(), word);
}

bool WordleGame::is_correct(const RowStates& row)
{
    for (auto c : row)
        if (c != TileState::Correct)
            return false;
    return true;
}

void WordleGame::draw_wordle_grid(const WordleStates& grid)
{
    const char block = ' ';  // will be filled in bg
    const int  cols  = 5;
    const int  rows  = 6;

    const int cell_w = 5;  // width in characters
    const int cell_h = 3;  // height in characters
    const int gap_x  = 1;
    const int gap_y  = 1;

    const int grid_w = cols * cell_w + (cols - 1) * gap_x;
    const int grid_h = rows * cell_h + (rows - 1) * gap_y;

    const int start_x = (display.getWidth() - grid_w) / 2;
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

void WordleGame::draw_not_valid(const std::string& word)
{
    if (!m_is_invalid)
        return;

    display.resetColors();
    // Place warning just above the vertically-centered grid
    const int y = display.pctY(0.45f) - (6 * 3 + 5 * 1) / 2 - 2;

    display.setTextColor(TB_RED | TB_BOLD);
    display.centerText(y, "Invalid word: {}", word);
    display.display();
}

void WordleGame::draw_end_game(bool won)
{
    display.setTextColor(won ? TB_GREEN : TB_RED);
    display.setFont(FigletType::FullWidth, "Big");
    display.centerText(display.pctY(0.40f), won ? "You Win!" : "You Lost");

    display.setTextColor(TB_WHITE);
    display.resetFont();
    display.centerText(display.pctY(0.60f), "Guess: {}", m_guess);
    display.display();
}

std::string WordleGame::get_random_guess()
{
    static std::mt19937                rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, m_words_list.size() - 1);
    return str_toupper(m_words_list[dist(rng)]);
}

void WordleGame::reset_game()
{
    m_buf.clear();
    m_grid        = WordleStates{};
    m_row         = 0;
    m_is_correct  = false;
    m_is_selected = false;
    m_is_invalid  = false;
    m_guess       = get_random_guess();
}

Result<> WordleGame::on_begin()
{
    std::ifstream f(settings.game_wordle.wordle_txt_path);
    if (!f)
        return Err("Failed to open wordle list: " + settings.game_wordle.wordle_txt_path);

    std::string word;
    while (std::getline(f, word))
        m_words_list.push_back(word);

    m_guess = get_random_guess();

    set_footer("Try to guess the word. Each letter color:\nBlack: Absent | Yellow: Present | Green: Correct");
    return Ok();
}

void WordleGame::render()
{
    if (!playback.isMusicPlaying())
        playback.playMusic(WordleSounds::BGM);

    display.clearDisplay();

    if (!m_is_selected)
    {
        for (int c = 0; c < 5; ++c)
        {
            m_grid[m_row][c].ch    = c < static_cast<int>(m_buf.size()) ? m_buf[c] : ' ';
            m_grid[m_row][c].state = TileState::Empty;
        }
    }
    else
    {
        m_is_invalid = !is_valid(str_tolower(m_buf));
        if (m_is_invalid)
        {
            m_is_selected  = false;
            m_invalid_word = m_buf;
        }
        else
        {
            m_invalid_word.clear();

            const RowStates& states = get_states(m_buf);
            for (int c = 0; c < 5; ++c)
            {
                m_grid[m_row][c].ch    = m_buf[c];
                m_grid[m_row][c].state = states[c];
            }
            m_row++;
            m_is_selected = false;
            m_buf.clear();

            m_is_correct = is_correct(states);
            if (m_is_correct)
            {
                draw_wordle_grid(m_grid);
                sleep_for(duration<float>(settings.game_wordle.delay_show_final_grid));
                display.clearDisplay();
                draw_end_game(true);
                sleep_for(duration<float>(settings.game_wordle.delay_show_endgame));
                reset_game();
                display.clearDisplay();
            }
        }
    }

    if (m_row == 6 && !m_is_correct)
    {
        draw_wordle_grid(m_grid);
        sleep_for(duration<float>(settings.game_wordle.delay_show_final_grid));
        display.clearDisplay();
        draw_end_game(false);
        sleep_for(duration<float>(settings.game_wordle.delay_show_endgame));
        reset_game();
        display.clearDisplay();
    }

    // Remaining empty rows
    for (int r = m_row; r < 6; ++r)
        if (m_grid[r].empty())
            for (int c = 0; c < 5; ++c)
                m_grid[r][c] = Tile{ ' ', TileState::Empty };

    draw_wordle_grid(m_grid);
    draw_not_valid(m_invalid_word);

    display.resetFont();
    display.resetColors();

    set_footer("Try to guess the word. Each letter color:\nBlack: Absent | Yellow: Present | Green: Correct");
    display.display();
}

SceneResult WordleGame::handle_input(uint32_t key)
{
    if (key == TB_KEY_ESC)
        return Scenes::GamesMenu;

    if (m_buf.size() == 5 && (key == TB_KEY_ENTER || key == '\n'))
        m_is_selected = true;
    else if (m_buf.size() < 5 && is_alpha(key))
        m_buf.push_back(toupper(key));
    else if (!m_buf.empty() && (key == TB_KEY_BACKSPACE || key == TB_KEY_BACKSPACE2))
    {
        m_buf.pop_back();
        m_is_invalid = false;
    }

    return ScenesGame::Wordle;
}
