#include "games/wordle.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <random>
#include <string>
#include <thread>

#include "audio_player.hpp"
#include "settings.hpp"
#include "terminal_display.hpp"

static constexpr int kWordLen = 5;
static constexpr int kMaxRows = 6;

static constexpr const char* kFooterText =
    "Try to guess the word. Each letter color:\n"
    "Black: Absent | Yellow: Present | Green: Correct";

// --------- helpers ---------

uintattr_t WordleGame::bg_for(TileState s)
{
    switch (s)
    {
        case TileState::Correct: return TB_GREEN;
        case TileState::Present: return TB_YELLOW;
        case TileState::Absent:  return TB_BLACK;
        case TileState::Empty:   return TB_WHITE;
    }
    return TB_DEFAULT;
}

uintattr_t WordleGame::fg_for(TileState s)
{
    switch (s)
    {
        case TileState::Correct:
        case TileState::Present:
        case TileState::Empty:  return TB_BLACK | TB_BOLD;
        case TileState::Absent: return TB_WHITE | TB_BOLD;
    }
    return TB_DEFAULT;
}

RowStates WordleGame::get_states(const std::string& str)
{
    RowStates states;
    states.fill(TileState::Absent);

    // Track letters in the answer still available for a Present match
    // (i.e. not already consumed by a Correct match).
    int letter_count[26] = {};

    // Pass 1: mark Correct tiles and deduct from available counts.
    for (int i = 0; i < kWordLen; ++i)
    {
        if (str[i] == m_guess[i])
            states[i] = TileState::Correct;
        else
            letter_count[m_guess[i] - 'A']++;
    }

    // Pass 2: mark Present tiles, consuming available letters.
    for (int i = 0; i < kWordLen; ++i)
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

bool WordleGame::is_valid(const std::string& word) const
{
    return !m_words_list.empty() &&
           std::binary_search(m_words_list.begin(), m_words_list.end(), word);
}

bool WordleGame::is_correct(const RowStates& row)
{
    for (auto state : row)
        if (state != TileState::Correct)
            return false;
    return true;
}

std::string WordleGame::get_random_guess()
{
    static std::mt19937                rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, m_words_list.size() - 1);
    return str_toupper(m_words_list[dist(rng)]);
}

// --------- Initialisation ---------

void WordleGame::init_game()
{
    m_buf.clear();
    m_grid        = WordleStates{};
    m_row         = 0;
    m_is_correct  = false;
    m_is_selected = false;
    m_is_invalid  = false;
    m_invalid_word.clear();

    // Pre-fill every cell so the render loop never has to do it.
    for (auto& row : m_grid)
        for (auto& tile : row)
            tile = Tile{ ' ', TileState::Empty };

    if (!m_words_list.empty())
        m_guess = get_random_guess();
}

Result<> WordleGame::on_game_begin()
{
    std::ifstream f(settings.game_wordle.wordle_txt_path);
    if (!f)
        return Err("Failed to open wordle list: " + settings.game_wordle.wordle_txt_path);

    std::string word;
    while (std::getline(f, word))
        m_words_list.push_back(word);

    init_game();
    set_footer(kFooterText);
    return Ok();
}

// --------- Drawing ---------

void WordleGame::draw_wordle_grid(const WordleStates& grid)
{
    const char block = ' ';

    const int cell_w = 5;
    const int cell_h = 3;
    const int gap_x  = 1;
    const int gap_y  = 1;

    const int grid_w = kWordLen * cell_w + (kWordLen - 1) * gap_x;
    const int grid_h = kMaxRows * cell_h + (kMaxRows - 1) * gap_y;

    const int start_x = (display.getWidth()  - grid_w) / 2;
    const int start_y = (display.getHeight() - grid_h) / 2;

    for (int r = 0; r < kMaxRows; ++r)
    {
        for (int c = 0; c < kWordLen; ++c)
        {
            const int x = start_x + c * (cell_w + gap_x);
            const int y = start_y + r * (cell_h + gap_y);

            const Tile& t = grid[r][c];

            display.setTextColor(bg_for(t.state));
            display.setTextBgColor(bg_for(t.state));
            display.drawFilledRect(x, y, cell_w, cell_h, block);
            display.drawRect(x, y, cell_w, cell_h, block);

            display.setTextColor(fg_for(t.state));
            display.setTextBgColor(bg_for(t.state));
            display.setCursor(x + cell_w / 2, y + cell_h / 2);
            display.print("{}", t.ch);
        }
    }

    display.resetColors();
    display.display();
}

void WordleGame::draw_not_valid(const std::string& word)
{
    if (!m_is_invalid)
        return;

    const int y = display.pctY(0.45f) - (kMaxRows * 3 + (kMaxRows - 1)) / 2 - 2;

    display.resetColors();
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

void WordleGame::run_end_game_sequence(bool won)
{
    draw_wordle_grid(m_grid);
    sleep_for(duration<float>(settings.game_wordle.delay_show_final_grid));
    display.clearDisplay();

    draw_end_game(won);
    sleep_for(duration<float>(settings.game_wordle.delay_show_endgame));

    init_game();
    display.clearDisplay();
}

// Separated from rendering so render_game() only has to draw.
void WordleGame::update_game()
{
    if (!m_is_selected)
    {
        // Reflect the current buffer in the active row.
        for (int c = 0; c < kWordLen; ++c)
        {
            m_grid[m_row][c].ch    = c < static_cast<int>(m_buf.size()) ? m_buf[c] : ' ';
            m_grid[m_row][c].state = TileState::Empty;
        }
        return;
    }

    // The player pressed Enter — validate the word.
    if (!is_valid(str_tolower(m_buf)))
    {
        m_is_invalid  = true;
        m_is_selected = false;
        m_invalid_word = m_buf;
        return;
    }

    // Commit the guess.
    m_invalid_word.clear();
    m_is_invalid = false;

    const RowStates states = get_states(m_buf);
    for (int c = 0; c < kWordLen; ++c)
    {
        m_grid[m_row][c].ch    = m_buf[c];
        m_grid[m_row][c].state = states[c];
    }

    m_is_selected = false;
    m_buf.clear();
    ++m_row;

    m_is_correct = is_correct(states);
    if (m_is_correct)
    {
        run_end_game_sequence(true);
        return;
    }

    if (m_row == kMaxRows)
        run_end_game_sequence(false);
}

void WordleGame::render_game()
{
    if (!playback.isMusicPlaying())
        playback.playMusic(WordleSounds::BGM);

    display.clearDisplay();
    update_game();

    draw_wordle_grid(m_grid);
    draw_not_valid(m_invalid_word);

    display.resetFont();
    display.resetColors();
    display.display();
}

// Input
SceneResult WordleGame::handle_input(uint32_t key)
{
    if (key == TB_KEY_ESC)
        return Scenes::GamesMenu;

    if (m_buf.size() == kWordLen && (key == TB_KEY_ENTER || key == '\n'))
        m_is_selected = true;
    else if (m_buf.size() < kWordLen && is_alpha(key))
        m_buf.push_back(static_cast<char>(toupper(key)));
    else if (!m_buf.empty() && (key == TB_KEY_BACKSPACE || key == TB_KEY_BACKSPACE2))
    {
        m_buf.pop_back();
        m_is_invalid = false;
    }

    return scene_id();
}
