#include "scenes.hpp"
#include "terminal_display.hpp"

void MainMenuScene::render()
{
    display.clearDisplay();

    int rows = display.getHeight();

    display.setFont(FigletType::FullWidth, "Big Money-nw");
    display.centerText(5, "CliBoy");
    display.resetFont();

    const char* menu_items[] = { "Games", "Credits" };
    int         start_y      = rows / 2 - 2;

    display.setFont(FigletType::FullWidth, "Small Slant");
    for (int i = 0; i < MENU_ITEM_COUNT; i++)
    {
        int y = start_y + i * 5;

        if (i == m_selected_item)
        {
            display.setTextColor(TB_WHITE | TB_BOLD);
            //display.setTextBgColor(TB_BLACK);
            display.centerText(y, "> {} <", menu_items[i]);
            display.resetColors();
        }
        else
        {
            display.centerText(y, menu_items[i]);
        }
    }
    display.resetFont();

    display.centerText(rows - 2, "Arrow Keys: Navigate | Enter: Select | ESC: Exit");

    display.display();
}

SceneResult MainMenuScene::handle_input(uint32_t key)
{
    switch (key)
    {
        case 27: return Scenes::Exit;

        case TB_KEY_ARROW_UP:   m_selected_item = (m_selected_item - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT; break;
        case TB_KEY_ARROW_DOWN: m_selected_item = (m_selected_item + 1) % MENU_ITEM_COUNT; break;

        case TB_KEY_ENTER:
        case '\n':
            if (m_selected_item == 0)
                return Scenes::Games;
            else if (m_selected_item == 1)
                return Scenes::Credits;
            break;
    }

    return Scenes::MainMenu;
}
