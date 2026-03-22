#include "scenes/main_menu.hpp"
#include "terminal_display.hpp"

void MainMenuScene::render()
{
    display.clearDisplay();

    display.setFont(FigletType::FullWidth, "Big Money-nw");
    display.centerText(display.pctY(0.10f), "CliBoy");
    display.resetFont();

    const char* menu_items[] = { "Games", "Settings", "Credits" };
    int         start_y      = display.pctY(0.50f) - (MENU_ITEM_COUNT - 1) * 5 / 2;

    display.setFont(FigletType::FullWidth, "Small Slant");
    for (int i = 0; i < MENU_ITEM_COUNT; i++)
    {
        int y = start_y + i * 5;

        if (i == m_selected_item)
        {
            display.setTextColor(TB_WHITE | TB_BOLD);
            // display.setTextBgColor(TB_BLACK);
            display.centerText(y, "> {} <", menu_items[i]);
            display.resetColors();
        }
        else
        {
            display.centerText(y, menu_items[i]);
        }
    }
    display.resetFont();

    display.display();
}

SceneResult MainMenuScene::handle_input(uint32_t key)
{
    switch (key)
    {
        case TB_KEY_ESC: return Scenes::Exit;

        case TB_KEY_ARROW_UP:   m_selected_item = (m_selected_item - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT; break;
        case TB_KEY_ARROW_DOWN: m_selected_item = (m_selected_item + 1) % MENU_ITEM_COUNT; break;

        case TB_KEY_ENTER:
        case '\n':
            return static_cast<Scenes>(m_selected_item);
    }

    return Scenes::MainMenu;
}
