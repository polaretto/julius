#include "population.h"

#include "game/time.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

#define ADVISOR_HEIGHT 27

static void button_graph(int param1, int param2);

static generic_button graph_buttons[] = {
    {503,  61, 607, 116, GB_IMMEDIATE, button_graph, button_none, 0, 0},
    {503, 161, 607, 216, GB_IMMEDIATE, button_graph, button_none, 1, 0},
};

static int focus_button_id;

static void get_y_axis(int max_value, int *y_max, int *y_shift)
{
    if (max_value <= 100) {
        *y_max = 100;
        *y_shift = -1;
    } else if (max_value <= 200) {
        *y_max = 200;
        *y_shift = 0;
    } else if (max_value <= 400) {
        *y_max = 400;
        *y_shift = 1;
    } else if (max_value <= 800) {
        *y_max = 800;
        *y_shift = 2;
    } else if (max_value <= 1600) {
        *y_max = 1600;
        *y_shift = 3;
    } else if (max_value <= 3200) {
        *y_max = 3200;
        *y_shift = 4;
    } else if (max_value <= 6400) {
        *y_max = 6400;
        *y_shift = 5;
    } else if (max_value <= 12800) {
        *y_max = 12800;
        *y_shift = 6;
    } else if (max_value <= 25600) {
        *y_max = 25600;
        *y_shift = 7;
    } else {
        *y_max = 51200;
        *y_shift = 8;
    }
}

static int get_population_at_month(int max, int month)
{
    int start_offset = 0;
    if (Data_CityInfo.monthsSinceStart > max) {
        start_offset = Data_CityInfo.monthsSinceStart + 2400 - max;
    }
    int index = (start_offset + month) % 2400;
    return Data_CityInfo.monthlyPopulation[index];
}

static void get_min_max_month_year(int max_months, int *start_month, int *start_year, int *end_month, int *end_year)
{
    if (Data_CityInfo.monthsSinceStart > max_months) {
        *end_month = game_time_month() - 1;
        *end_year = game_time_year();
        if (*end_month < 0) {
            *end_month += 12;
            *end_year -= 1;
        }
        *start_month = 11 - (max_months % 12);
        *start_year = *end_year - max_months / 12;
    } else {
        *start_month = 0;
        *start_year = scenario_property_start_year();
        *end_month = (max_months + *start_month) % 12;
        *end_year = (max_months + *start_month) / 12 + *start_year;
    }
}

static void draw_history_graph(int full_size, int x, int y)
{
    int max_months;
    if (Data_CityInfo.monthsSinceStart <= 20) {
        max_months = 20;
    } else if (Data_CityInfo.monthsSinceStart <= 40) {
        max_months = 40;
    } else if (Data_CityInfo.monthsSinceStart <= 100) {
        max_months = 100;
    } else if (Data_CityInfo.monthsSinceStart <= 200) {
        max_months = 200;
    } else {
        max_months = 400;
    }
    if (!full_size) {
        if (max_months <= 40) {
            max_months = 20;
        } else {
            max_months = 100;
        }
    }
    // determine max value
    int max_value = 0;
    for (int m = 0; m < max_months; m++) {
        int value = get_population_at_month(max_months, m);
        if (value > max_value) {
            max_value = value;
        }
    }
    int y_max, y_shift;
    get_y_axis(max_value, &y_max, &y_shift);
    if (full_size) {
        // y axis
        text_draw_number_centered(y_max, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(y_max / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
        // x axis
        int startMonth, startYear, endMonth, endYear;
        get_min_max_month_year(max_months, &startMonth, &startYear, &endMonth, &endYear);

        int width = lang_text_draw(25, startMonth, x - 20, y + 210, FONT_SMALL_PLAIN);
        lang_text_draw_year(startYear, x + width - 20, y + 210, FONT_SMALL_PLAIN);

        width = lang_text_draw(25, endMonth, x + 380, y + 210, FONT_SMALL_PLAIN);
        lang_text_draw_year(startYear, x + width + 380, y + 210, FONT_SMALL_PLAIN);
    }

    if (full_size) {
        graphics_set_clip_rectangle(0, 0, 640, y + 200);
        for (int m = 0; m < max_months; m++) {
            int pop = get_population_at_month(max_months, m);
            int val;
            if (y_shift == -1) {
                val = 2 * pop;
            } else {
                val = pop >> y_shift;
            }
            if (val > 0) {
                switch (max_months) {
                    case 20:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR), x + 20 * m, y + 200 - val);
                        break;
                    case 40:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 1, x + 10 * m, y + 200 - val);
                        break;
                    case 100:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 2, x + 4 * m, y + 200 - val);
                        break;
                    case 200:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 3, x + 2 * m, y + 200 - val);
                        break;
                    default:
                        graphics_draw_line(x + m, y + 200 - val, x + m, y + 199, COLOR_RED);
                        break;
                }
            }
        }
        graphics_reset_clip_rectangle();
    } else {
        y_shift += 2;
        for (int m = 0; m < max_months; m++) {
            int val = get_population_at_month(max_months, m) >> y_shift;
            if (val > 0) {
                if (max_months == 20) {
                    graphics_fill_rect(x + m, y + 50 - val, 4, val + 1, COLOR_RED);
                } else {
                    graphics_draw_line(x + m, y + 50 - val, x + m, y + 50, COLOR_RED);
                }
            }
        }
    }
}

static void draw_census_graph(int full_size, int x, int y)
{
    int max_value = 0;
    for (int i = 0; i < 100; i++) {
        if (Data_CityInfo.populationPerAge[i] > max_value) {
            max_value = Data_CityInfo.populationPerAge[i];
        }
    }
    int y_max, y_shift;
    get_y_axis(max_value, &y_max, &y_shift);
    if (full_size) {
        // y axis
        text_draw_number_centered(y_max, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(y_max / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
        // x axis
        for (int i = 0; i <= 10; i++) {
            text_draw_number_centered(i * 10, x + 40 * i - 22, y + 210, 40, FONT_SMALL_PLAIN);
        }
    }

    if (full_size) {
        graphics_set_clip_rectangle(0, 0, 640, y + 200);
        for (int i = 0; i < 100; i++) {
            int pop = Data_CityInfo.populationPerAge[i];
            int val;
            if (y_shift == -1) {
                val = 2 * pop;
            } else {
                val = pop >> y_shift;
            }
            if (val > 0) {
                image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 2, x + 4 * i, y + 200 - val);
            }
        }
        graphics_reset_clip_rectangle();
    } else {
        y_shift += 2;
        for (int i = 0; i < 100; i++) {
            int val = Data_CityInfo.populationPerAge[i] >> y_shift;
            if (val > 0) {
                graphics_draw_line(x + i, y + 50 - val, x + i, y + 50, COLOR_RED);
            }
        }
    }
}

static void draw_society_graph(int full_size, int x, int y)
{
    int max_value = 0;
    for (int i = 0; i < 20; i++) {
        if (Data_CityInfo.populationPerLevel[i] > max_value) {
            max_value = Data_CityInfo.populationPerLevel[i];
        }
    }
    int y_max, y_shift;
    get_y_axis(max_value, &y_max, &y_shift);
    if (full_size) {
        // y axis
        text_draw_number_centered(y_max, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(y_max / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
        // x axis
        lang_text_draw_centered(55, 9, x - 80, y + 210, 200, FONT_SMALL_PLAIN);
        lang_text_draw_centered(55, 10, x + 280, y + 210, 200, FONT_SMALL_PLAIN);
    }

    if (full_size) {
        graphics_set_clip_rectangle(0, 0, 640, y + 200);
        for (int i = 0; i < 20; i++) {
            int pop = Data_CityInfo.populationPerLevel[i];
            int val;
            if (y_shift == -1) {
                val = 2 * pop;
            } else {
                val = pop >> y_shift;
            }
            if (val > 0) {
                image_draw(image_group(GROUP_POPULATION_GRAPH_BAR), x + 20 * i, y + 200 - val);
            }
        }
        graphics_reset_clip_rectangle();
    } else {
        y_shift += 2;
        for (int i = 0; i < 20; i++) {
            int val = Data_CityInfo.populationPerLevel[i] >> y_shift;
            if (val > 0) {
                graphics_fill_rect(x + 5 * i, y + 50 - val, 4, val + 1, COLOR_RED);
            }
        }
    }
}

static int draw_background()
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 5, 10, 10);

    // Title: depends on big graph shown
    if (Data_CityInfo_Extra.populationGraphOrder < 2) {
        lang_text_draw(55, 0, 60, 12, FONT_LARGE_BLACK);
    } else if (Data_CityInfo_Extra.populationGraphOrder < 4) {
        lang_text_draw(55, 1, 60, 12, FONT_LARGE_BLACK);
    } else {
        lang_text_draw(55, 2, 60, 12, FONT_LARGE_BLACK);
    }

    image_draw(image_group(GROUP_PANEL_WINDOWS) + 14, 56, 60);
    
    int big_text, top_text, bot_text;
    void (*big_graph)(int, int, int);
    void (*top_graph)(int, int, int);
    void (*bot_graph)(int, int, int);
    switch (Data_CityInfo_Extra.populationGraphOrder) {
        default:
        case 0:
            big_text = 6;
            top_text = 4;
            bot_text = 5;
            big_graph = draw_history_graph;
            top_graph = draw_census_graph;
            bot_graph = draw_society_graph;
            break;
        case 1:
            big_text = 6;
            top_text = 5;
            bot_text = 4;
            big_graph = draw_history_graph;
            top_graph = draw_society_graph;
            bot_graph = draw_census_graph;
            break;
        case 2:
            big_text = 7;
            top_text = 3;
            bot_text = 5;
            big_graph = draw_census_graph;
            top_graph = draw_history_graph;
            bot_graph = draw_society_graph;
            break;
        case 3:
            big_text = 7;
            top_text = 5;
            bot_text = 3;
            big_graph = draw_census_graph;
            top_graph = draw_society_graph;
            bot_graph = draw_history_graph;
            break;
        case 4:
            big_text = 8;
            top_text = 3;
            bot_text = 4;
            big_graph = draw_society_graph;
            top_graph = draw_history_graph;
            bot_graph = draw_census_graph;
            break;
        case 5:
            big_text = 8;
            top_text = 4;
            bot_text = 3;
            big_graph = draw_society_graph;
            top_graph = draw_census_graph;
            bot_graph = draw_history_graph;
            break;
    }
    lang_text_draw_centered(55, big_text, 60, 295, 400, FONT_NORMAL_BLACK);
    lang_text_draw_centered(55, top_text, 504, 120, 100, FONT_NORMAL_BLACK);
    lang_text_draw_centered(55, bot_text, 504, 220, 100, FONT_NORMAL_BLACK);
    big_graph(1, 64, 64);
    top_graph(0, 505, 63);
    bot_graph(0, 505, 163);

    // food/migration info panel
    inner_panel_draw(48, 336, 34, 5);
    int image_id = image_group(GROUP_BULLET);
    int width;
    image_draw(image_id, 56, 344);
    image_draw(image_id, 56, 362);
    image_draw(image_id, 56, 380);
    image_draw(image_id, 56, 398);

    // food stores
    if (scenario_property_rome_supplies_wheat()) {
        lang_text_draw(55, 11, 75, 342, FONT_NORMAL_WHITE);
    } else {
        width = lang_text_draw_amount(8, 6, Data_CityInfo.foodInfoGranariesOperating, 75, 342, FONT_NORMAL_WHITE);
        if (Data_CityInfo.foodInfoFoodSupplyMonths > 0) {
            width += lang_text_draw(55, 12, 75 + width, 342, FONT_NORMAL_WHITE);
            lang_text_draw_amount(8, 4, Data_CityInfo.foodInfoFoodSupplyMonths, 75 + width, 342, FONT_NORMAL_WHITE);
        } else if (Data_CityInfo.foodInfoFoodStoredInGranaries > Data_CityInfo.foodInfoFoodNeededPerMonth / 2) {
            lang_text_draw(55, 13, 75 + width, 342, FONT_NORMAL_WHITE);
        } else if (Data_CityInfo.foodInfoFoodStoredInGranaries > 0) {
            lang_text_draw(55, 15, 75 + width, 342, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(55, 14, 75 + width, 342, FONT_NORMAL_WHITE);
        }
    }

    // food types eaten
    width = lang_text_draw(55, 16, 75, 360, FONT_NORMAL_WHITE);
    text_draw_number(Data_CityInfo.foodInfoFoodTypesAvailable, '@', " ", 75 + width, 360, FONT_NORMAL_WHITE);

    // immigration
    if (Data_CityInfo.populationNewcomersThisMonth >= 5) {
        lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
        width = text_draw_number(Data_CityInfo.populationNewcomersThisMonth, '@', " ", 75, 396, FONT_NORMAL_WHITE);
        lang_text_draw(55, 17, 75 + width, 396, FONT_NORMAL_WHITE);
    } else if (Data_CityInfo.populationRefusedImmigrantsNoRoom || Data_CityInfo.populationRoomInHouses <= 0) {
        lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
        lang_text_draw(55, 19, 75, 396, FONT_NORMAL_WHITE);
    } else if (Data_CityInfo.populationMigrationPercentage < 80) {
        lang_text_draw(55, 25, 75, 378, FONT_NORMAL_WHITE);
        int textId;
        switch (Data_CityInfo.populationEmigrationCauseTextId) {
            case 0: textId = 20; break;
            case 1: textId = 21; break;
            case 2: textId = 22; break;
            case 3: textId = 23; break;
            case 4: textId = 31; break;
            case 5: textId = 32; break;
            default: textId = 0; break;
        }
        if (textId) {
            lang_text_draw(55, textId, 75, 396, FONT_NORMAL_WHITE);
        }
    } else {
        lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
        width = text_draw_number(Data_CityInfo.populationNewcomersThisMonth, '@', " ", 75, 396, FONT_NORMAL_WHITE);
        if (Data_CityInfo.populationNewcomersThisMonth == 1) {
            lang_text_draw(55, 18, 75 + width, 396, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(55, 17, 75 + width, 396, FONT_NORMAL_WHITE);
        }
    }

    return ADVISOR_HEIGHT;
}

static void draw_foreground()
{
    if (focus_button_id == 0) {
        button_border_draw(501, 60, 106, 57, 0);
        button_border_draw(501, 160, 106, 57, 0);
    } else if (focus_button_id == 1) {
        button_border_draw(501, 60, 106, 57, 1);
        button_border_draw(501, 160, 106, 57, 0);
    } else if (focus_button_id == 2) {
        button_border_draw(501, 60, 106, 57, 0);
        button_border_draw(501, 160, 106, 57, 1);
    }
}

static void handle_mouse(const mouse *m)
{
    generic_buttons_handle_mouse(m, 0, 0, graph_buttons, 2, &focus_button_id);
}

static void button_graph(int param1, int param2)
{
    int new_order;
    switch (Data_CityInfo_Extra.populationGraphOrder) {
        default:
        case 0:
            new_order = param1 ? 5 : 2;
            break;
        case 1:
            new_order = param1 ? 3 : 4;
            break;
        case 2:
            new_order = param1 ? 4 : 0;
            break;
        case 3:
            new_order = param1 ? 1 : 5;
            break;
        case 4:
            new_order = param1 ? 2 : 1;
            break;
        case 5:
            new_order = param1 ? 0 : 3;
            break;
    }
    Data_CityInfo_Extra.populationGraphOrder = new_order;
    window_invalidate();
}

static int get_tooltip_text()
{
    if (focus_button_id) {
        return 111;
    } else {
        return 0;
    }
}

const advisor_window_type *window_advisor_population()
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
