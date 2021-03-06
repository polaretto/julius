#include "message_dialog.h"

#include "city/message.h"
#include "city/view.h"
#include "core/lang.h"
#include "empire/city.h"
#include "figure/formation.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/rich_text.h"
#include "graphics/text.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "window/advisors.h"
#include "window/city.h"

#include "Data/CityInfo.h"

#define MAX_HISTORY 200

static void draw_foreground_video();

static void button_back(int param1, int param2);
static void button_close(int param1, int param2);
static void button_help(int param1, int param2);
static void button_advisor(int advisor, int param2);
static void button_go_to_problem(int param1, int param2);

static image_button image_button_back = {
    0, 0, 31, 20, IB_NORMAL, 90, 8, button_back, button_none, 0, 0, 1
};
static image_button image_button_close = {
    0, 0, 24, 24, IB_NORMAL, 134, 4, button_close, button_none, 0, 0, 1
};
static image_button image_button_go_to_problem = {
    0, 0, 27, 27, IB_NORMAL, 92, 52, button_go_to_problem, button_none, 1, 0, 1
};
static image_button image_button_help = {
    0, 0, 18, 27, IB_NORMAL, 134, 0, button_help, button_none, 1, 0, 1
};
static image_button image_button_labor = {
    0, 0, 27, 27, IB_NORMAL, 199, 0, button_advisor, button_none, ADVISOR_LABOR, 0, 1
};
static image_button image_button_trade = {
    0, 0, 27, 27, IB_NORMAL, 199, 12, button_advisor, button_none, ADVISOR_TRADE, 0, 1
};
static image_button image_button_population = {
    0, 0, 27, 27, IB_NORMAL, 199, 15, button_advisor, button_none, ADVISOR_POPULATION, 0, 1
};
static image_button image_button_imperial = {
    0, 0, 27, 27, IB_NORMAL, 199, 6, button_advisor, button_none, ADVISOR_IMPERIAL, 0, 1
};
static image_button image_button_military = {
    0, 0, 27, 27, IB_NORMAL, 199, 3, button_advisor, button_none, ADVISOR_MILITARY, 0, 1
};
static image_button image_button_health = {
    0, 0, 27, 27, IB_NORMAL, 199, 18, button_advisor, button_none, ADVISOR_HEALTH, 0, 1
};
static image_button image_button_religion = {
    0, 0, 27, 27, IB_NORMAL, 199, 27, button_advisor, button_none, ADVISOR_RELIGION, 0, 1
};

static struct {
    struct {
        int text_id;
        int scroll_position;
    } history[200];
    int num_history;

    int text_id;
    int background_is_provided;
    int show_video;

    int x;
    int y;
    int x_text;
    int y_text;
    int text_height_blocks;
    int text_width_blocks;
} data;

static struct {
    int year;
    int month;
    int param1;
    int param2;
    int message_advisor;
    int use_popup;
} playerMessage;

static void set_city_message(int year, int month,
                             int param1, int param2,
                             int message_advisor, int use_popup)
{
    playerMessage.year = year;
    playerMessage.month = month;
    playerMessage.param1 = param1;
    playerMessage.param2 = param2;
    playerMessage.message_advisor = message_advisor;
    playerMessage.use_popup = use_popup;
}

static void init(int text_id, int background_is_provided)
{
    for (int i = 0; i < MAX_HISTORY; i++) {
        data.history[i].text_id = 0;
        data.history[i].scroll_position = 0;
    }
    data.num_history = 0;
    rich_text_reset(0);
    data.text_id = text_id;
    data.background_is_provided = background_is_provided;
    const lang_message *msg = lang_get_message(text_id);
    if (playerMessage.use_popup != 1) {
        data.show_video = 0;
    } else if (msg->video.text && video_start((char*)msg->video.text)) {
        data.show_video = 1;
    } else {
        data.show_video = 0;
    }
    if (data.show_video) {
        video_init();
    }
}

static int resource_image(int resource)
{
    int image_id = image_group(GROUP_RESOURCE_ICONS) + resource;
    image_id += resource_image_offset(resource, RESOURCE_IMAGE_ICON);
    return image_id;
}

static void draw_city_message_text(const lang_message *msg)
{
    if (msg->message_type != MESSAGE_TYPE_TUTORIAL) {
        int width = lang_text_draw(25, playerMessage.month, data.x_text + 10, data.y_text + 6, FONT_NORMAL_WHITE);
        width += lang_text_draw_year(playerMessage.year, data.x_text + 12 + width, data.y_text + 6, FONT_NORMAL_WHITE);
        if (msg->message_type == MESSAGE_TYPE_DISASTER && playerMessage.param1) {
            if (data.text_id == MessageDialog_Theft) {
                // param1 = denarii
                lang_text_draw_amount(8, 0, playerMessage.param1, data.x + 240, data.y_text + 6, FONT_NORMAL_WHITE);
            } else {
                // param1 = building type
                lang_text_draw(41, playerMessage.param1, data.x + 240, data.y_text + 6, FONT_NORMAL_WHITE);
            }
        } else {
            width += lang_text_draw(63, 5, data.x_text + width + 80, data.y_text + 6, FONT_NORMAL_WHITE);
            text_draw(scenario_player_name(), data.x_text + width + 80, data.y_text + 6, FONT_NORMAL_WHITE, 0);
        }
    }
    switch (msg->message_type) {
        case MESSAGE_TYPE_DISASTER:
        case MESSAGE_TYPE_INVASION:
            lang_text_draw(12, 1, data.x + 100, data.y_text + 44, FONT_NORMAL_WHITE);
            rich_text_draw(msg->content.text, data.x_text + 8, data.y_text + 86,
                16 * data.text_width_blocks, data.text_height_blocks - 1, 0);
            break;

        case MESSAGE_TYPE_EMIGRATION:
            if (Data_CityInfo.populationEmigrationCause >= 1 && Data_CityInfo.populationEmigrationCause <= 5) {
                lang_text_draw(12, Data_CityInfo.populationEmigrationCause + 2,
                    data.x + 64, data.y_text + 44, FONT_NORMAL_WHITE);
            }
            rich_text_draw(msg->content.text,
                data.x_text + 8, data.y_text + 86, 16 * data.text_width_blocks - 16,
                data.text_height_blocks - 1, 0);
            break;

        case MESSAGE_TYPE_TUTORIAL:
            rich_text_draw(msg->content.text,
                data.x_text + 8, data.y_text + 6, 16 * data.text_width_blocks - 16,
                data.text_height_blocks - 1, 0);
            break;

        case MESSAGE_TYPE_TRADE_CHANGE:
            image_draw(resource_image(playerMessage.param2), data.x + 64, data.y_text + 40);
            lang_text_draw(21, empire_city_get(playerMessage.param1)->name_id,
                data.x + 100, data.y_text + 44, FONT_NORMAL_WHITE);
            rich_text_draw(msg->content.text,
                data.x_text + 8, data.y_text + 86, 16 * data.text_width_blocks - 16,
                data.text_height_blocks - 1, 0);
            break;

        case MESSAGE_TYPE_PRICE_CHANGE:
            image_draw(resource_image(playerMessage.param2), data.x + 64, data.y_text + 40);
            text_draw_money(playerMessage.param1, data.x + 100, data.y_text + 44, FONT_NORMAL_WHITE);
            rich_text_draw(msg->content.text,
                data.x_text + 8, data.y_text + 86, 16 * data.text_width_blocks - 16,
                data.text_height_blocks - 1, 0);
            break;

        default: {
            int lines = rich_text_draw(msg->content.text,
                data.x_text + 8, data.y_text + 56, 16 * data.text_width_blocks - 16,
                data.text_height_blocks - 1, 0);
            if (msg->message_type == MESSAGE_TYPE_IMPERIAL) {
                const scenario_request *request = scenario_request_get(playerMessage.param1);
                int yOffset = data.y_text + 86 + lines * 16;
                text_draw_number(request->amount, '@', " ", data.x_text + 8, yOffset, FONT_NORMAL_WHITE);
                image_draw(resource_image(request->resource), data.x_text + 70, yOffset - 5);
                lang_text_draw(23, request->resource,
                    data.x_text + 100, yOffset, FONT_NORMAL_WHITE);
                if (request->state == REQUEST_STATE_NORMAL || request->state == REQUEST_STATE_OVERDUE) {
                    int width = lang_text_draw_amount(8, 4, request->months_to_comply, data.x_text + 200, yOffset, FONT_NORMAL_WHITE);
                    lang_text_draw(12, 2, data.x_text + 200 + width, yOffset, FONT_NORMAL_WHITE);
                }
            }
            break;
        }
    }
}

static void draw_background_normal()
{
    rich_text_set_fonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED);
    const lang_message *msg = lang_get_message(data.text_id);
    data.x = msg->x;
    data.y = msg->y;
    int header_offset = (msg->type == TYPE_MANUAL) ? 48 : 32;
    data.x_text = data.x + 16;
    outer_panel_draw(data.x, data.y, msg->width_blocks, msg->height_blocks);
    // title
    if (msg->title.x) {
        text_draw(msg->title.text,
            data.x + msg->title.x, data.y + msg->title.y, FONT_LARGE_BLACK, 0);
        data.y_text = data.y + 32;
    } else {
        if (msg->message_type == MESSAGE_TYPE_TUTORIAL) {
            text_draw_centered(msg->title.text,
                data.x, data.y + msg->title.y, 16 * msg->width_blocks, FONT_LARGE_BLACK, 0);
        } else {
            text_draw_centered(msg->title.text,
                data.x, data.y + 14, 16 * msg->width_blocks, FONT_LARGE_BLACK, 0);
        }
        data.y_text = data.y + 48;
    }
    // pictures
    if (msg->image1.id) {
        int graphicId, graphicX, graphicY;
        if (data.text_id) {
            graphicId = image_group(GROUP_MESSAGE_IMAGES) + msg->image1.id - 1;
            graphicX = msg->image1.x;
            graphicY = msg->image1.y;
        } else { // message id = 0 ==> about, fixed image position
            graphicX = graphicY = 16;
            graphicId = image_group(GROUP_BIG_PEOPLE);
        }
        image_draw(graphicId, data.x + graphicX, data.y + graphicY);
        if (data.y + graphicY + image_get(graphicId)->height + 8 > data.y_text) {
            data.y_text = data.y + graphicY + image_get(graphicId)->height + 8;
        }
    }
    if (msg->image2.id) {
        int graphicId = image_group(GROUP_MESSAGE_IMAGES) + msg->image2.id - 1;
        image_draw(graphicId, data.x + msg->image2.x, data.y + msg->image2.y);
        if (data.y + msg->image2.y + image_get(graphicId)->height + 8 > data.y_text) {
            data.y_text = data.y + msg->image2.y + image_get(graphicId)->height + 8;
        }
    }
    // subtitle
    if (msg->subtitle.x) {
        int width = 16 * msg->width_blocks - 16 - msg->subtitle.x;
        int height = text_draw_multiline(msg->subtitle.text,
            data.x + msg->subtitle.x, data.y + msg->subtitle.y, width,FONT_NORMAL_BLACK);
        if (data.y + msg->subtitle.y + height > data.y_text) {
            data.y_text = data.y + msg->subtitle.y + height;
        }
    }
    data.text_height_blocks = msg->height_blocks - 1 - (header_offset + data.y_text - data.y) / 16;
    data.text_width_blocks = rich_text_init(msg->content.text,
        data.x_text, data.y_text, msg->width_blocks - 4, data.text_height_blocks, 1);

    // content!
    inner_panel_draw(data.x_text, data.y_text, data.text_width_blocks, data.text_height_blocks);
    graphics_set_clip_rectangle(data.x_text + 3, data.y_text + 3,
        16 * data.text_width_blocks - 6, 16 * data.text_height_blocks - 6);
    rich_text_clear_links();

    if (msg->type == TYPE_MESSAGE) {
        draw_city_message_text(msg);
    } else {
        rich_text_draw(msg->content.text,
            data.x_text + 8, data.y_text + 6, 16 * data.text_width_blocks - 16,
            data.text_height_blocks - 1, 0);
    }
    graphics_reset_clip_rectangle();
    rich_text_draw_scrollbar_dot();
}

static void draw_background_video()
{
    rich_text_set_fonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED);
    const lang_message *msg = lang_get_message(data.text_id);
    data.x = 32;
    data.y = 28;
    outer_panel_draw(data.x, data.y, 26, 28);
    graphics_draw_rect(data.x + 7, data.y + 7, 402, 294, COLOR_BLACK);
    rich_text_clear_links();
    
    inner_panel_draw(data.x + 8, data.y + 308, 25, 6);
    text_draw_centered(msg->title.text,
        data.x + 8, data.y + 414, 400, FONT_NORMAL_BLACK, 0);
    
    int width = lang_text_draw(25, playerMessage.month, data.x + 16, data.y + 312, FONT_NORMAL_WHITE);
    width += lang_text_draw_year(playerMessage.year, data.x + 18 + width, data.y + 312, FONT_NORMAL_WHITE);
    
    if (msg->type == TYPE_MESSAGE && msg->message_type == MESSAGE_TYPE_DISASTER &&
        data.text_id == 251) {
        lang_text_draw_amount(8, 0, playerMessage.param1, data.x + 90 + width, data.y + 312, FONT_NORMAL_WHITE);
    } else {
        width += lang_text_draw(63, 5, data.x + 90 + width, data.y + 312, FONT_NORMAL_WHITE);
        text_draw(scenario_player_name(), data.x + 90 + width, data.y + 312, FONT_NORMAL_WHITE, 0);
    }
    data.text_height_blocks = msg->height_blocks - 1 - (32 + data.y_text - data.y) / 16;
    data.text_width_blocks = msg->width_blocks - 4;
    rich_text_draw(msg->content.text, data.x + 16, data.y + 332, 384, data.text_height_blocks - 1, 0);

    if (msg->type == TYPE_MESSAGE && msg->message_type == MESSAGE_TYPE_IMPERIAL) {
        const scenario_request *request = scenario_request_get(playerMessage.param1);
        text_draw_number(request->amount, '@', " ", data.x + 8, data.y + 384, FONT_NORMAL_WHITE);
        image_draw(
            image_group(GROUP_RESOURCE_ICONS) + request->resource + resource_image_offset(request->resource, RESOURCE_IMAGE_ICON),
            data.x + 70, data.y + 379);
        lang_text_draw(23, request->resource, data.x + 100, data.y + 384, FONT_NORMAL_WHITE);
        if (request->state == REQUEST_STATE_NORMAL || request->state == REQUEST_STATE_OVERDUE) {
            width = lang_text_draw_amount(8, 4, request->months_to_comply, data.x + 200, data.y + 384, FONT_NORMAL_WHITE);
            lang_text_draw(12, 2, data.x + 200 + width, data.y + 384, FONT_NORMAL_WHITE);
        }
    }

    draw_foreground_video();
}

static void draw_background()
{
    if (!data.background_is_provided) {
        window_city_draw_all();
    }
    graphics_in_dialog();
    if (data.show_video) {
        draw_background_video();
    } else {
        draw_background_normal();
    }
    graphics_reset_dialog();
}

static image_button *get_advisor_button()
{
    switch (playerMessage.message_advisor) {
        case MESSAGE_ADVISOR_LABOR:
            return &image_button_labor;
        case MESSAGE_ADVISOR_TRADE:
            return &image_button_trade;
        case MESSAGE_ADVISOR_POPULATION:
            return &image_button_population;
        case MESSAGE_ADVISOR_IMPERIAL:
            return &image_button_imperial;
        case MESSAGE_ADVISOR_MILITARY:
            return &image_button_military;
        case MESSAGE_ADVISOR_HEALTH:
            return &image_button_health;
        case MESSAGE_ADVISOR_RELIGION:
            return &image_button_religion;
        default:
            return &image_button_help;
    }
}

static void draw_foreground_normal()
{
    const lang_message *msg = lang_get_message(data.text_id);
    
    if (msg->type == TYPE_MANUAL && data.num_history > 0) {
        image_buttons_draw(
            data.x + 16, data.y + 16 * msg->height_blocks - 36,
            &image_button_back, 1);
        lang_text_draw(12, 0,
            data.x + 52, data.y + 16 * msg->height_blocks - 31, FONT_NORMAL_BLACK);
    }

    if (msg->type == TYPE_MESSAGE) {
        image_buttons_draw(data.x + 16, data.y + 16 * msg->height_blocks - 40, get_advisor_button(), 1);
        if (msg->message_type == MESSAGE_TYPE_DISASTER || msg->message_type == MESSAGE_TYPE_INVASION) {
            image_buttons_draw(data.x + 64, data.y_text + 36, &image_button_go_to_problem, 1);
        }
    }
    image_buttons_draw(data.x + 16 * msg->width_blocks - 38, data.y + 16 * msg->height_blocks - 36, &image_button_close, 1);
    rich_text_draw_scrollbar();
}

static void draw_foreground_video()
{
    video_draw(data.x + 8, data.y + 8);
    image_buttons_draw(data.x + 16, data.y + 408, get_advisor_button(), 1);
    image_buttons_draw(data.x + 372, data.y + 410, &image_button_close, 1);
}

static void draw_foreground()
{
    graphics_in_dialog();
    if (data.show_video) {
        draw_foreground_video();
    } else {
        draw_foreground_normal();
    }
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (m_dialog->scrolled == SCROLL_DOWN) {
        rich_text_scroll(1, 3);
    } else if (m_dialog->scrolled == SCROLL_UP) {
        rich_text_scroll(0, 3);
    }
    if (data.show_video) {
        if (!image_buttons_handle_mouse(m_dialog, data.x + 16, data.y + 408, get_advisor_button(), 1, 0)) {
            image_buttons_handle_mouse(m_dialog, data.x + 372, data.y + 410, &image_button_close, 1, 0);
        }
        return;
    }
    // no video
    const lang_message *msg = lang_get_message(data.text_id);

    if (msg->type == TYPE_MANUAL && image_buttons_handle_mouse(
                m_dialog, data.x + 16, data.y + 16 * msg->height_blocks - 36, &image_button_back, 1, 0)) {
        return;
    }
    if (msg->type == TYPE_MESSAGE) {
        if (image_buttons_handle_mouse(m_dialog, data.x + 16, data.y + 16 * msg->height_blocks - 40,
                                       get_advisor_button(), 1, 0)) {
            return;
        }
        if (msg->message_type == MESSAGE_TYPE_DISASTER || msg->message_type == MESSAGE_TYPE_INVASION) {
            if (image_buttons_handle_mouse(m_dialog, data.x + 64, data.y_text + 36, &image_button_go_to_problem, 1, 0)) {
                return;
            }
        }
    }

    if (image_buttons_handle_mouse(m_dialog,
        data.x + 16 * msg->width_blocks - 38,
        data.y + 16 * msg->height_blocks - 36,
        &image_button_close, 1, 0)) {
        return;
    }
    rich_text_handle_mouse(m_dialog);
    int text_id = rich_text_get_clicked_link(m_dialog);
    if (text_id >= 0) {
        if (data.num_history < MAX_HISTORY - 1) {
            data.history[data.num_history].text_id = data.text_id;
            data.history[data.num_history].scroll_position = rich_text_scroll_position();
            data.num_history++;
        }
        data.text_id = text_id;
        rich_text_reset(0);
        window_invalidate();
    }
}

static void button_back(int param1, int param2)
{
    if (data.num_history > 0) {
        data.num_history--;
        data.text_id = data.history[data.num_history].text_id;
        rich_text_reset(data.history[data.num_history].scroll_position);
        window_invalidate();
    }
}

static void cleanup()
{
    if (data.show_video) {
        video_stop();
        data.show_video = 0;
    }
    playerMessage.message_advisor = 0;
}

static void button_close(int param1, int param2)
{
    cleanup();
    UI_Window_goBack();
    window_invalidate();
}

static void button_help(int param1, int param2)
{
    button_close(0, 0);
    window_message_dialog_show(MessageDialog_Help, 0);
}

static void button_advisor(int advisor, int param2)
{
    cleanup();
    if (!window_advisors_show_advisor(advisor)) {
        window_city_show();
    }
}

static void button_go_to_problem(int param1, int param2)
{
    cleanup();
    const lang_message *msg = lang_get_message(data.text_id);
    int gridOffset = playerMessage.param2;
    if (msg->message_type == MESSAGE_TYPE_INVASION) {
        int invasionGridOffset = formation_grid_offset_for_invasion(playerMessage.param1);
        if (invasionGridOffset > 0) {
            gridOffset = invasionGridOffset;
        }
    }
    if (gridOffset > 0 && gridOffset < 26244) {
        city_view_go_to_grid_offset(gridOffset);
    }
    window_city_show();
}

void window_message_dialog_show(int text_id, int background_is_provided)
{
    window_type window = {
        Window_MessageDialog,
        draw_background,
        draw_foreground,
        handle_mouse,
        0
    };
    init(text_id, background_is_provided);
    window_show(&window);
}

void window_message_dialog_show_city_message(int text_id, int year, int month,
                                             int param1, int param2, int message_advisor, int use_popup)
{
    set_city_message(year, month, param1, param2, message_advisor, use_popup);
    window_message_dialog_show(text_id, 0);
}
