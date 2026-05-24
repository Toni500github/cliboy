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

// --------- helpers ---------

uintattr_t WordleGame::bgFor(TileState s)
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

uintattr_t WordleGame::fgFor(TileState s)
{
    switch (s)
    {
        case TileState::Correct:
        case TileState::Present:
        case TileState::Empty:   return TB_BLACK | TB_BOLD;
        case TileState::Absent:  return TB_WHITE | TB_BOLD;
    }
    return TB_DEFAULT;
}

// Call this before draw_keyboard to build the current letter state map.
LetterStates buildLetterStates(const WordleStates& grid)
{
    LetterStates states;
    states.fill(TileState::Empty);

    for (const auto& row : grid)
    {
        for (const Tile& t : row)
        {
            if (t.ch == ' ' || t.state == TileState::Empty)
                continue;

            int idx = t.ch - 'A';

            // Only upgrade, never downgrade:
            // Correct > Present > Absent > Empty
            if (static_cast<int>(t.state) > static_cast<int>(states[idx]))
                states[idx] = t.state;
        }
    }

    return states;
}

RowStates WordleGame::getStates(const std::string& str)
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

bool WordleGame::isValid(const std::string& word) const
{
    return !m_words_list.empty() && std::binary_search(m_words_list.begin(), m_words_list.end(), word);
}

bool WordleGame::isCorrect(const RowStates& row)
{
    for (auto state : row)
        if (state != TileState::Correct)
            return false;
    return true;
}

std::string WordleGame::getRandomGuess()
{
    static std::mt19937                rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, m_words_list.size() - 1);
    return str_toupper(m_words_list[dist(rng)]);
}

// --------- Initialisation ---------

void WordleGame::initGame()
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
        m_guess = getRandomGuess();
}

Result<> WordleGame::onGameBegin()
{
    std::ifstream f(settings.game_wordle.wordle_txt_path);
    if (!f)
        return Err("Failed to open wordle list: " + settings.game_wordle.wordle_txt_path);

    std::string word;
    while (std::getline(f, word))
        m_words_list.push_back(word);

    initGame();
    setFooter(kFooterText);
    return Ok();
}

// --------- Drawing ---------

void WordleGame::drawWordleGrid(const WordleStates& grid)
{
    const char block = ' ';

    const int grid_w = kWordLen * cell_w + (kWordLen - 1) * gap_x;

    const int start_x = (display.getWidth() - grid_w) / 2;
    const int start_y = (display.getHeight() - total_h) / 2;  // top of combined block

    for (int r = 0; r < kMaxRows; ++r)
    {
        for (int c = 0; c < kWordLen; ++c)
        {
            const int x = start_x + c * (cell_w + gap_x);
            const int y = start_y + r * (cell_h + gap_y);

            const Tile& t = grid[r][c];

            display.setTextColor(bgFor(t.state));
            display.setTextBgColor(bgFor(t.state));
            display.drawFilledRect(x, y, cell_w, cell_h, block);
            display.drawRect(x, y, cell_w, cell_h, block);

            display.setTextColor(fgFor(t.state));
            display.setTextBgColor(bgFor(t.state));
            display.setCursor(x + cell_w / 2, y + cell_h / 2);
            display.print("{}", t.ch);
        }
    }

    display.resetColors();
    display.display();
}

void WordleGame::drawKeyboard(const WordleStates& grid)
{
    const LetterStates& letter_states = buildLetterStates(grid);

    static constexpr const char* rows[3] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };

    const char block = ' ';

    const int max_keys = 10;  // longest row
    const int total_w  = max_keys * kb_cell_w + (max_keys - 1) * kb_gap_x;
    const int start_y  = (display.getHeight() - total_h) / 2 + grid_h + grid_kb_gap;

    for (int r = 0; r < 3; ++r)
    {
        const char* row    = rows[r];
        const int   n_keys = static_cast<int>(strlen(row));

        const int row_w   = n_keys * kb_cell_w + (n_keys - 1) * kb_gap_x;
        const int start_x = (display.getWidth() - total_w) / 2 + (total_w - row_w) / 2;
        const int y       = start_y + r * (kb_cell_h + kb_gap_y);

        for (int c = 0; c < n_keys; ++c)
        {
            const char      letter = row[c];
            const TileState state  = letter_states[letter - 'A'];
            const int       x      = start_x + c * (kb_cell_w + kb_gap_x);

            display.setTextColor(bgFor(state));
            display.setTextBgColor(bgFor(state));
            display.drawFilledRect(x, y, kb_cell_w, kb_cell_h, block);

            display.setTextColor(fgFor(state));
            display.setTextBgColor(bgFor(state));
            display.setCursor(x + kb_cell_w / 2, y + kb_cell_h / 2);
            display.print("{}", letter);
        }
    }

    display.resetColors();
    display.display();
}

void WordleGame::drawNotValid(const std::string& word)
{
    if (!m_is_invalid)
        return;

    const int block_start_y = (display.getHeight() - total_h) / 2;
    const int y             = block_start_y - 2;  // one blank line above the grid

    display.resetColors();
    display.setTextColor(TB_RED | TB_BOLD);
    display.centerText(y, "Invalid word: {}", word);
    display.display();
}

void WordleGame::drawEndGame(bool won)
{
    display.setTextColor(won ? TB_GREEN : TB_RED);
    display.setFont(FigletType::FullWidth, "Big");
    display.centerText(display.pctY(0.40f), won ? "You Win!" : "You Lost");

    display.setTextColor(TB_WHITE);
    display.resetFont();
    display.centerText(display.pctY(0.60f), "Guess: {}", m_guess);
    display.display();
}

void WordleGame::runEndGameSequence(bool won)
{
    drawWordleGrid(m_grid);
    sleep_for(duration<float>(settings.game_wordle.delay_show_final_grid));
    display.clearDisplay();

    drawEndGame(won);
    sleep_for(duration<float>(settings.game_wordle.delay_show_endgame));

    initGame();
    display.clearDisplay();
}

// Separated from rendering so render_game() only has to draw.
void WordleGame::updateGame()
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
    if (!isValid(str_tolower(m_buf)))
    {
        m_is_invalid   = true;
        m_is_selected  = false;
        m_invalid_word = m_buf;
        return;
    }

    // Commit the guess.
    m_invalid_word.clear();
    m_is_invalid = false;

    const RowStates states = getStates(m_buf);
    for (int c = 0; c < kWordLen; ++c)
    {
        m_grid[m_row][c].ch    = m_buf[c];
        m_grid[m_row][c].state = states[c];
    }

    m_is_selected = false;
    m_buf.clear();
    ++m_row;

    m_is_correct = isCorrect(states);
    if (m_is_correct)
    {
        runEndGameSequence(true);
        return;
    }

    if (m_row == kMaxRows)
        runEndGameSequence(false);
}

void WordleGame::renderGame()
{
    if (!playback.isMusicPlaying())
        playback.playMusic(WordleSounds::BGM);

    display.clearDisplay();
    updateGame();

    drawWordleGrid(m_grid);
    drawKeyboard(m_grid);

    drawNotValid(m_invalid_word);

    display.resetFont();
    display.resetColors();
    display.display();
}

// Input
SceneResult WordleGame::handleInput(uint32_t key)
{
    if (key == TB_KEY_ESC)
        return Scenes::GamesMenu;

    if (m_buf.size() == kWordLen && (key == TB_KEY_ENTER || key == '\n'))
        m_is_selected = true;
    else if (m_buf.size() < kWordLen && is_alpha(key))
        m_buf.push_back(static_cast<char>(std::toupper(key)));
    else if (!m_buf.empty() && (key == TB_KEY_BACKSPACE || key == TB_KEY_BACKSPACE2))
    {
        m_buf.pop_back();
        m_is_invalid = false;
    }

    return sceneID();
}
