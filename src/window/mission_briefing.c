#include "mission_briefing.h"

#include "core/lang.h"
#include "game/file.h"
#include "game/mission.h"
#include "game/tutorial.h"
#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/rich_text.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/criteria.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "window/city.h"
#include "window/intermezzo.h"
#include "window/mission_selection.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

static void button_back(int param1, int param2);
static void button_start_mission(int param1, int param2);

static const int GOAL_OFFSETS_X[] = {32, 288, 32, 288, 288, 288};
static const int GOAL_OFFSETS_Y[] = {95, 95, 117, 117, 73, 135};

static image_button imageButtonBackToSelection = {
    0, 0, 31, 20, IB_NORMAL, 90, 8, button_back, button_none, 0, 0, 1
};
static image_button imageButtonStartMission = {
    0, 0, 27, 27, IB_NORMAL, 92, 56, button_start_mission, button_none, 1, 0, 1
};

static struct {
    int is_review;
    int focus_button;
} data;

static void init()
{
    data.focus_button = 0;
    rich_text_reset(0);
}

static void draw_background()
{
    if (!Data_State.missionBriefingShown) {
        Data_State.missionBriefingShown = 1;
        if (!game_file_start_scenario(scenario_name())) {
            window_city_show();
            return;
        }
    }
    
    graphics_in_dialog();
    int textId = 200 + scenario_campaign_mission();
    
    outer_panel_draw(16, 32, 38, 27);
    text_draw(lang_get_message(textId)->title.text, 32, 48, FONT_LARGE_BLACK, 0);
    text_draw(lang_get_message(textId)->subtitle.text, 32, 78, FONT_NORMAL_BLACK, 0);

    lang_text_draw(62, 7, 376, 433, FONT_NORMAL_BLACK);
    if (!data.is_review && game_mission_has_choice()) {
        lang_text_draw(13, 4, 66, 435, FONT_NORMAL_BLACK);
    }
    
    inner_panel_draw(32, 96, 33, 5);
    lang_text_draw(62, 10, 48, 104, FONT_NORMAL_WHITE);
    int goalIndex = 0;
    if (scenario_criteria_population_enabled()) {
        int x = GOAL_OFFSETS_X[goalIndex];
        int y = GOAL_OFFSETS_Y[goalIndex];
        goalIndex++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 11, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_population(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_culture_enabled()) {
        int x = GOAL_OFFSETS_X[goalIndex];
        int y = GOAL_OFFSETS_Y[goalIndex];
        goalIndex++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 12, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_culture(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_prosperity_enabled()) {
        int x = GOAL_OFFSETS_X[goalIndex];
        int y = GOAL_OFFSETS_Y[goalIndex];
        goalIndex++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 13, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_prosperity(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_peace_enabled()) {
        int x = GOAL_OFFSETS_X[goalIndex];
        int y = GOAL_OFFSETS_Y[goalIndex];
        goalIndex++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 14, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_peace(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
    }
    if (scenario_criteria_favor_enabled()) {
        int x = GOAL_OFFSETS_X[goalIndex];
        int y = GOAL_OFFSETS_Y[goalIndex];
        goalIndex++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 15, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_favor(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
    }
    int immediateGoalText = tutorial_get_immediate_goal_text();
    if (immediateGoalText) {
        int x = GOAL_OFFSETS_X[2];
        int y = GOAL_OFFSETS_Y[2];
        goalIndex++;
        label_draw(16 + x, 32 + y, 31, 1);
        lang_text_draw(62, immediateGoalText, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
    }
    
    inner_panel_draw(32, 184, 33, 15);
    
    rich_text_set_fonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED);
    rich_text_init(lang_get_message(textId)->content.text, 64, 184, 31, 15, 0);

    graphics_set_clip_rectangle(35, 187, 522, 234);
    rich_text_draw(lang_get_message(textId)->content.text, 48, 196, 496, 14, 0);
    graphics_reset_clip_rectangle();

    graphics_reset_dialog();
}

static void draw_foreground()
{
    graphics_in_dialog();

    rich_text_draw_scrollbar();
    image_buttons_draw(516, 426, &imageButtonStartMission, 1);
    if (!data.is_review && game_mission_has_choice()) {
        image_buttons_draw(26, 428, &imageButtonBackToSelection, 1);
    }

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);

    if (image_buttons_handle_mouse(m_dialog, 516, 426, &imageButtonStartMission, 1, 0)) {
        return;
    }
    if (!data.is_review && game_mission_has_choice()) {
        if (image_buttons_handle_mouse(m_dialog, 26, 428, &imageButtonBackToSelection, 1, 0)) {
            return;
        }
    }
    rich_text_handle_mouse(m_dialog);
}

static void button_back(int param1, int param2)
{
    if (!data.is_review) {
        sound_speech_stop();
        window_mission_selection_show();
    }
}

static void button_start_mission(int param1, int param2)
{
    sound_speech_stop();
    sound_music_reset();
    window_city_show();
    Data_CityInfo.missionSavedGameWritten = 0;
}

static void show()
{
    window_type window = {
        Window_MissionBriefingInitial,
        draw_background,
        draw_foreground,
        handle_mouse,
        0
    };
    init();
    window_show(&window);
}

void window_mission_briefing_show()
{
    data.is_review = 0;
    Data_State.missionBriefingShown = 0;
    window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, show);
}

void window_mission_briefing_show_review()
{
    data.is_review = 1;
    window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, show);
}
