/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <xstar.h>
#include <xlvgl.h>
#include <kernel/command/command.h>

#define TINYUI_WIDTH			240
#define TINYUI_HEIGHT			240
#define TINYUI_PAGE_COUNT		6
#define TINYUI_STATUS_HEIGHT	30
#define TINYUI_PAGES_HEIGHT		184

#define TINYUI_COLOR_TEXT		0xF8F9FC
#define TINYUI_COLOR_SUBTEXT	0x97A9CB
#define TINYUI_COLOR_BLUE		0x53B5D9
#define TINYUI_COLOR_RED		0xEC6A78
#define TINYUI_COLOR_PURPLE		0x8C6BD1
#define TINYUI_COLOR_ORANGE		0xE88B65
#define TINYUI_COLOR_GREEN		0x59B9A2
#define TINYUI_COLOR_YELLOW		0xE9B55F
#define TINYUI_COLOR_PAGE		0x17203B
#define TINYUI_COLOR_HEADER		0x202D4B
#define TINYUI_COLOR_ROW		0x253351

struct tinyui_app_t {
	const char * name;
	const char * symbol;
	uint32_t color;
};

struct tinyui_t {
	lv_obj_t * root;
	lv_obj_t * pages;
	lv_obj_t * dots[TINYUI_PAGE_COUNT];
	lv_obj_t * clock;
	lv_obj_t * appview;
	lv_obj_t * appview_icon;
	lv_obj_t * appview_title;
	lv_obj_t * settings;
	lv_obj_t * settings_list;
	lv_obj_t * settings_detail;
	lv_obj_t * settings_title;
	lv_timer_t * timer;
	lv_point_t icon_press;
	lv_point_t settings_press;
	int page;
	int settings_section;
	int icon_dragged;
	int settings_dragged;
};

static const struct tinyui_app_t tinyui_apps[TINYUI_PAGE_COUNT] = {
	{ "Weather", LV_SYMBOL_TINT, TINYUI_COLOR_BLUE },
	{ "Calendar", LV_SYMBOL_LIST, TINYUI_COLOR_RED },
	{ "Music", LV_SYMBOL_AUDIO, TINYUI_COLOR_PURPLE },
	{ "Camera", LV_SYMBOL_IMAGE, TINYUI_COLOR_ORANGE },
	{ "Calculator", LV_SYMBOL_PLUS, 0x8998B6 },
	{ "Settings", LV_SYMBOL_SETTINGS, TINYUI_COLOR_YELLOW },
};

static const struct tinyui_app_t tinyui_settings_sections[] = {
	{ "General", LV_SYMBOL_SETTINGS, 0x6688D9 },
	{ "Wi-Fi", LV_SYMBOL_WIFI, TINYUI_COLOR_BLUE },
	{ "Bluetooth", LV_SYMBOL_BLUETOOTH, TINYUI_COLOR_GREEN },
	{ "Display", LV_SYMBOL_EYE_OPEN, TINYUI_COLOR_YELLOW },
	{ "Sound", LV_SYMBOL_VOLUME_MAX, TINYUI_COLOR_PURPLE },
	{ "Date & time", LV_SYMBOL_LIST, TINYUI_COLOR_RED },
	{ "Location", LV_SYMBOL_GPS, TINYUI_COLOR_GREEN },
	{ "About", LV_SYMBOL_OK, 0x8998B6 },
};

static struct {
	int brightness;
	int volume;
	int notifications;
	int wifi;
	int bluetooth;
	int dark_mode;
	int silent;
	int auto_time;
	int hour_24;
	int location;
	int auto_location;
} tinyui_settings_values = {
	.brightness = 70,
	.volume = 45,
	.notifications = 1,
	.wifi = 1,
	.bluetooth = 1,
	.dark_mode = 1,
	.auto_time = 1,
	.hour_24 = 1,
};

static struct tinyui_t tinyui;

static lv_obj_t * tinyui_box(lv_obj_t * parent, int x, int y, int width, int height, uint32_t color, int radius)
{
	lv_obj_t * obj = lv_obj_create(parent);

	lv_obj_set_pos(obj, x, y);
	lv_obj_set_size(obj, width, height);
	lv_obj_set_style_radius(obj, radius, 0);
	lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
	lv_obj_set_style_border_width(obj, 0, 0);
	lv_obj_set_style_pad_all(obj, 0, 0);
	lv_obj_set_style_pad_row(obj, 0, 0);
	lv_obj_set_style_pad_column(obj, 0, 0);
	lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_scrollable(obj, 0);
	return obj;
}

static lv_obj_t * tinyui_label(lv_obj_t * parent, const char * text, int x, int y,
	const lv_font_t * font, uint32_t color)
{
	lv_obj_t * label = lv_label_create(parent);

	lv_label_set_text(label, text);
	lv_obj_set_pos(label, x, y);
	lv_obj_set_style_text_font(label, font, 0);
	lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
	return label;
}

static void tinyui_update_time(lv_timer_t * timer)
{
	struct wallclock_time_t tm;
	char buffer[8];

	wallclock_gettime(&tm, NULL);
	xos_sprintf(buffer, "%02d:%02d", tm.hour, tm.minute);
	lv_label_set_text(tinyui.clock, buffer);
}

static void tinyui_update_dots(int page)
{
	int i;

	if(page < 0)
		page = 0;
	if(page >= TINYUI_PAGE_COUNT)
		page = TINYUI_PAGE_COUNT - 1;
	tinyui.page = page;
	for(i = 0; i < TINYUI_PAGE_COUNT; i++)
		lv_obj_set_style_bg_opa(tinyui.dots[i], i == page ? LV_OPA_COVER : LV_OPA_40, 0);
}

static void tinyui_pages_event(lv_event_t * event)
{
	lv_point_t end;
	int page;

	if(lv_event_get_code(event) != LV_EVENT_SCROLL)
		return;
	lv_obj_get_scroll_end(tinyui.pages, &end);
	page = (end.x + TINYUI_WIDTH / 2) / TINYUI_WIDTH;
	if(page != tinyui.page)
		tinyui_update_dots(page);
}

static void tinyui_dot_event(lv_event_t * event)
{
	int page;

	if(lv_event_get_code(event) != LV_EVENT_CLICKED)
		return;
	page = (int)(uintptr_t)lv_event_get_user_data(event);
	if(page != tinyui.page)
		lv_obj_scroll_to_x(tinyui.pages, page * TINYUI_WIDTH, LV_ANIM_ON);
}

static void tinyui_appview_back_event(lv_event_t * event)
{
	if(lv_event_get_code(event) == LV_EVENT_CLICKED)
		lv_obj_set_hidden(tinyui.appview, 1);
}

static void tinyui_appview_gesture_event(lv_event_t * event)
{
	lv_indev_t * indev = lv_indev_active();

	if(indev && lv_indev_get_gesture_dir(indev) == LV_DIR_RIGHT)
		lv_obj_set_hidden(tinyui.appview, 1);
}

static void tinyui_settings_go_back(void)
{
	if(tinyui.settings_section >= 0)
	{
		tinyui.settings_section = -1;
		lv_label_set_text(tinyui.settings_title, "Settings");
		lv_obj_set_hidden(tinyui.settings_detail, 1);
		lv_obj_set_hidden(tinyui.settings_list, 0);
	}
	else
	{
		lv_obj_set_hidden(tinyui.settings, 1);
	}
}

static void tinyui_settings_back_event(lv_event_t * event)
{
	if(lv_event_get_code(event) != LV_EVENT_CLICKED)
		return;
	tinyui_settings_go_back();
}

static void tinyui_settings_gesture_event(lv_event_t * event)
{
	lv_indev_t * indev = lv_indev_active();

	if(indev && lv_indev_get_gesture_dir(indev) == LV_DIR_RIGHT)
		tinyui_settings_go_back();
}

static void tinyui_app_event(lv_event_t * event)
{
	const struct tinyui_app_t * app;
	lv_event_code_t code = lv_event_get_code(event);
	lv_indev_t * indev = lv_indev_active();
	lv_point_t point;
	int index;
	int dx;
	int dy;

	if(code == LV_EVENT_PRESSED)
	{
		tinyui.icon_dragged = 0;
		if(indev)
			lv_indev_get_point(indev, &tinyui.icon_press);
		return;
	}
	if(code == LV_EVENT_PRESSING || code == LV_EVENT_RELEASED)
	{
		if(indev)
		{
			lv_indev_get_point(indev, &point);
			dx = point.x - tinyui.icon_press.x;
			dy = point.y - tinyui.icon_press.y;
			if(dx > 10 || dx < -10 || dy > 10 || dy < -10)
				tinyui.icon_dragged = 1;
		}
		return;
	}
	if(code != LV_EVENT_CLICKED || tinyui.icon_dragged)
		return;
	index = (int)(uintptr_t)lv_event_get_user_data(event);
	if(index == TINYUI_PAGE_COUNT - 1)
	{
		tinyui.settings_section = -1;
		lv_label_set_text(tinyui.settings_title, "Settings");
		lv_obj_set_hidden(tinyui.settings_detail, 1);
		lv_obj_set_hidden(tinyui.settings_list, 0);
		lv_obj_set_hidden(tinyui.settings, 0);
		return;
	}
	app = &tinyui_apps[index];
	lv_label_set_text(tinyui.appview_title, app->name);
	lv_label_set_text(tinyui.appview_icon, app->symbol);
	lv_obj_set_style_bg_color(lv_obj_get_parent(tinyui.appview_icon), lv_color_hex(app->color), 0);
	lv_obj_set_hidden(tinyui.appview, 0);
}

static void tinyui_create_app_page(int index)
{
	const struct tinyui_app_t * app = &tinyui_apps[index];
	lv_obj_t * page = tinyui_box(tinyui.pages, 0, 0, TINYUI_WIDTH, TINYUI_PAGES_HEIGHT, 0x000000, 0);
	lv_obj_t * icon;
	lv_obj_t * symbol;
	lv_obj_t * name;

	lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
	lv_obj_set_flex_grow(page, 0);
	lv_obj_set_event_bubble(page, 1);
	icon = tinyui_box(page, 72, 24, 96, 96, app->color, 24);
	lv_obj_set_style_bg_color(icon, lv_color_darken(lv_color_hex(app->color), 70), LV_STATE_PRESSED);
	lv_obj_set_clickable(icon, 1);
	lv_obj_add_event_cb(icon, tinyui_app_event, LV_EVENT_PRESSED, (void *)(uintptr_t)index);
	lv_obj_add_event_cb(icon, tinyui_app_event, LV_EVENT_PRESSING, (void *)(uintptr_t)index);
	lv_obj_add_event_cb(icon, tinyui_app_event, LV_EVENT_RELEASED, (void *)(uintptr_t)index);
	lv_obj_add_event_cb(icon, tinyui_app_event, LV_EVENT_CLICKED, (void *)(uintptr_t)index);
	symbol = tinyui_label(icon, app->symbol, 0, 0, &lv_font_montserrat_24, TINYUI_COLOR_TEXT);
	lv_obj_center(symbol);

	name = tinyui_label(page, app->name, 0, 135, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	lv_obj_set_width(name, TINYUI_WIDTH);
	lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
}

static void tinyui_setting_switch_event(lv_event_t * event)
{
	int * value = (int *)lv_event_get_user_data(event);

	*value = lv_obj_has_state(lv_event_get_target_obj(event), LV_STATE_CHECKED);
}

static void tinyui_setting_slider_event(lv_event_t * event)
{
	int * value = (int *)lv_event_get_user_data(event);

	*value = lv_slider_get_value(lv_event_get_target_obj(event));
}

static void tinyui_setting_switch(lv_obj_t * parent, int y, const char * title, const char * detail, int * value)
{
	lv_obj_t * row = tinyui_box(parent, 8, y, 224, 55, TINYUI_COLOR_ROW, 12);
	lv_obj_t * sw;

	tinyui_label(row, title, 12, 7, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	tinyui_label(row, detail, 12, 29, &lv_font_montserrat_16, TINYUI_COLOR_SUBTEXT);
	sw = lv_switch_create(row);
	lv_obj_set_pos(sw, 169, 15);
	lv_obj_set_size(sw, 43, 25);
	if(*value)
		lv_obj_add_state(sw, LV_STATE_CHECKED);
	lv_obj_add_event_cb(sw, tinyui_setting_switch_event, LV_EVENT_VALUE_CHANGED, value);
}

static void tinyui_setting_slider(lv_obj_t * parent, int y, const char * title, int * value, uint32_t color)
{
	lv_obj_t * row = tinyui_box(parent, 8, y, 224, 76, TINYUI_COLOR_ROW, 12);
	lv_obj_t * slider;

	tinyui_label(row, title, 12, 9, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	slider = lv_slider_create(row);
	lv_obj_set_pos(slider, 14, 49);
	lv_obj_set_size(slider, 196, 9);
	lv_slider_set_value(slider, *value, LV_ANIM_OFF);
	lv_obj_set_style_bg_color(slider, lv_color_hex(color), LV_PART_INDICATOR);
	lv_obj_add_event_cb(slider, tinyui_setting_slider_event, LV_EVENT_VALUE_CHANGED, value);
}

static void tinyui_setting_info(lv_obj_t * parent, int y, const char * title, const char * value)
{
	lv_obj_t * row = tinyui_box(parent, 8, y, 224, 51, TINYUI_COLOR_ROW, 12);

	tinyui_label(row, title, 12, 6, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	tinyui_label(row, value, 12, 27, &lv_font_montserrat_16, TINYUI_COLOR_SUBTEXT);
}

static void tinyui_setting_dropdown(lv_obj_t * parent, int y, const char * title, const char * options)
{
	lv_obj_t * row = tinyui_box(parent, 8, y, 224, 55, TINYUI_COLOR_ROW, 12);
	lv_obj_t * dropdown;

	tinyui_label(row, title, 12, 19, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	dropdown = lv_dropdown_create(row);
	lv_obj_set_pos(dropdown, 114, 9);
	lv_obj_set_size(dropdown, 98, 36);
	lv_dropdown_set_options(dropdown, options);
}

static void tinyui_setting_scroll_space(lv_obj_t * parent)
{
	lv_obj_t * space = tinyui_box(parent, 0, 192, 1, 1, TINYUI_COLOR_PAGE, 0);

	lv_obj_set_style_bg_opa(space, LV_OPA_TRANSP, 0);
}

static void tinyui_settings_show_section(int section)
{
	struct wallclock_time_t tm;
	char buffer[32];

	lv_obj_clean(tinyui.settings_detail);
	lv_obj_scroll_to_y(tinyui.settings_detail, 0, LV_ANIM_OFF);
	lv_label_set_text(tinyui.settings_title, tinyui_settings_sections[section].name);
	lv_obj_set_hidden(tinyui.settings_list, 1);
	lv_obj_set_hidden(tinyui.settings_detail, 0);
	tinyui.settings_section = section;
	switch(section)
	{
	case 0:
		tinyui_setting_dropdown(tinyui.settings_detail, 8, "Language", "English\nChinese");
		tinyui_setting_switch(tinyui.settings_detail, 71, "Notifications", "Show alerts", &tinyui_settings_values.notifications);
		break;
	case 1:
		tinyui_setting_switch(tinyui.settings_detail, 8, "Wi-Fi", "Wireless network", &tinyui_settings_values.wifi);
		tinyui_setting_info(tinyui.settings_detail, 71, "Available networks", "No networks scanned");
		break;
	case 2:
		tinyui_setting_switch(tinyui.settings_detail, 8, "Bluetooth", "Nearby devices", &tinyui_settings_values.bluetooth);
		tinyui_setting_info(tinyui.settings_detail, 71, "Paired devices", "No devices paired");
		break;
	case 3:
		tinyui_setting_slider(tinyui.settings_detail, 8, "Brightness", &tinyui_settings_values.brightness, 0x7198ED);
		tinyui_setting_switch(tinyui.settings_detail, 92, "Dark mode", "Use dark colors", &tinyui_settings_values.dark_mode);
		tinyui_setting_dropdown(tinyui.settings_detail, 155, "Screen timeout", "30 seconds\n1 minute\n5 minutes");
		break;
	case 4:
		tinyui_setting_slider(tinyui.settings_detail, 8, "Media volume", &tinyui_settings_values.volume, 0x70C7BF);
		tinyui_setting_switch(tinyui.settings_detail, 92, "Silent mode", "Mute alerts", &tinyui_settings_values.silent);
		tinyui_setting_info(tinyui.settings_detail, 155, "Notification sound", "Default");
		break;
	case 5:
		wallclock_gettime(&tm, NULL);
		xos_sprintf(buffer, "%04d-%02d-%02d  %02d:%02d", tm.year, tm.month, tm.day, tm.hour, tm.minute);
		tinyui_setting_info(tinyui.settings_detail, 8, "Current date & time", buffer);
		tinyui_setting_switch(tinyui.settings_detail, 67, "Automatic time", "Sync with network", &tinyui_settings_values.auto_time);
		tinyui_setting_switch(tinyui.settings_detail, 130, "24-hour format", "Use 00:00 to 23:59", &tinyui_settings_values.hour_24);
		break;
	case 6:
		tinyui_setting_switch(tinyui.settings_detail, 8, "Location services", "Allow location access", &tinyui_settings_values.location);
		tinyui_setting_switch(tinyui.settings_detail, 71, "Auto detect", "Use network location", &tinyui_settings_values.auto_location);
		tinyui_setting_info(tinyui.settings_detail, 134, "Current location", "Not set");
		break;
	default:
		tinyui_setting_info(tinyui.settings_detail, 8, "Device", "TinyUI device");
		tinyui_setting_info(tinyui.settings_detail, 67, "System", "XSTAR");
		tinyui_setting_info(tinyui.settings_detail, 126, "LVGL", "9.6.0");
		tinyui_setting_info(tinyui.settings_detail, 185, "Display", "240 x 240");
		break;
	}
	tinyui_setting_scroll_space(tinyui.settings_detail);
}

static void tinyui_settings_section_event(lv_event_t * event)
{
	lv_event_code_t code = lv_event_get_code(event);
	lv_indev_t * indev = lv_indev_active();
	lv_point_t point;
	int section;
	int dx;
	int dy;

	if(code == LV_EVENT_PRESSED)
	{
		tinyui.settings_dragged = 0;
		if(indev)
			lv_indev_get_point(indev, &tinyui.settings_press);
		return;
	}
	if(code == LV_EVENT_PRESSING || code == LV_EVENT_RELEASED)
	{
		if(indev)
		{
			lv_indev_get_point(indev, &point);
			dx = point.x - tinyui.settings_press.x;
			dy = point.y - tinyui.settings_press.y;
			if(dx > 10 || dx < -10 || dy > 10 || dy < -10)
				tinyui.settings_dragged = 1;
		}
		return;
	}
	if(code != LV_EVENT_CLICKED || tinyui.settings_dragged)
		return;
	section = (int)(uintptr_t)lv_event_get_user_data(event);
	tinyui_settings_show_section(section);
}

static void tinyui_create_settings(void)
{
	static const char * subtitles[] = {
		"Language and notifications",
		"Wireless networks",
		"Nearby devices",
		"Brightness and timeout",
		"Volume and alerts",
		"Clock and time format",
		"Location services",
		"Device information",
	};
	lv_obj_t * header;
	lv_obj_t * back;
	lv_obj_t * arrow;
	int i;

	tinyui.settings = tinyui_box(tinyui.root, 0, 0, TINYUI_WIDTH, TINYUI_HEIGHT, TINYUI_COLOR_PAGE, 0);
	lv_obj_set_gesture_bubble(tinyui.settings, 0);
	lv_obj_add_event_cb(tinyui.settings, tinyui_settings_gesture_event, LV_EVENT_GESTURE, NULL);
	header = tinyui_box(tinyui.settings, 0, 0, TINYUI_WIDTH, 49, TINYUI_COLOR_HEADER, 0);
	back = tinyui_box(header, 10, 9, 36, 32, 0x34415E, 10);
	lv_obj_set_clickable(back, 1);
	lv_obj_add_event_cb(back, tinyui_settings_back_event, LV_EVENT_CLICKED, NULL);
	arrow = tinyui_label(back, LV_SYMBOL_LEFT, 0, 0, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	lv_obj_center(arrow);
	tinyui.settings_title = tinyui_label(header, "Settings", 57, 14, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);

	tinyui.settings_list = tinyui_box(tinyui.settings, 0, 49, TINYUI_WIDTH, 191, TINYUI_COLOR_PAGE, 0);
	lv_obj_set_scrollable(tinyui.settings_list, 1);
	lv_obj_set_scroll_dir(tinyui.settings_list, LV_DIR_VER);
	lv_obj_set_scroll_elastic(tinyui.settings_list, 1);
	for(i = 0; i < 8; i++)
	{
		const struct tinyui_app_t * section = &tinyui_settings_sections[i];
		lv_obj_t * row = tinyui_box(tinyui.settings_list, 8, 7 + i * 56, 224, 50, TINYUI_COLOR_ROW, 12);
		lv_obj_t * icon;
		lv_obj_t * symbol;

		lv_obj_set_clickable(row, 1);
		lv_obj_add_event_cb(row, tinyui_settings_section_event, LV_EVENT_PRESSED, (void *)(uintptr_t)i);
		lv_obj_add_event_cb(row, tinyui_settings_section_event, LV_EVENT_PRESSING, (void *)(uintptr_t)i);
		lv_obj_add_event_cb(row, tinyui_settings_section_event, LV_EVENT_RELEASED, (void *)(uintptr_t)i);
		lv_obj_add_event_cb(row, tinyui_settings_section_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
		icon = tinyui_box(row, 9, 8, 34, 34, section->color, 10);
		symbol = tinyui_label(icon, section->symbol, 0, 0, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
		lv_obj_center(symbol);
		tinyui_label(row, section->name, 53, 5, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
		tinyui_label(row, subtitles[i], 53, 25, &lv_font_montserrat_16, TINYUI_COLOR_SUBTEXT);
		tinyui_label(row, LV_SYMBOL_RIGHT, 202, 17, &lv_font_montserrat_16, TINYUI_COLOR_SUBTEXT);
	}

	tinyui.settings_detail = tinyui_box(tinyui.settings, 0, 49, TINYUI_WIDTH, 191, TINYUI_COLOR_PAGE, 0);
	lv_obj_set_scrollable(tinyui.settings_detail, 1);
	lv_obj_set_scroll_dir(tinyui.settings_detail, LV_DIR_VER);
	lv_obj_set_scroll_elastic(tinyui.settings_detail, 1);
	lv_obj_set_hidden(tinyui.settings_detail, 1);
	lv_obj_set_hidden(tinyui.settings, 1);
}

static void tinyui_create_appview(void)
{
	lv_obj_t * back;
	lv_obj_t * arrow;
	lv_obj_t * icon;
	lv_obj_t * message;

	tinyui.appview = tinyui_box(tinyui.root, 0, 0, TINYUI_WIDTH, TINYUI_HEIGHT, TINYUI_COLOR_PAGE, 0);
	lv_obj_set_gesture_bubble(tinyui.appview, 0);
	lv_obj_add_event_cb(tinyui.appview, tinyui_appview_gesture_event, LV_EVENT_GESTURE, NULL);
	back = tinyui_box(tinyui.appview, 10, 9, 36, 32, 0x34415E, 10);
	lv_obj_set_clickable(back, 1);
	lv_obj_add_event_cb(back, tinyui_appview_back_event, LV_EVENT_CLICKED, NULL);
	arrow = tinyui_label(back, LV_SYMBOL_LEFT, 0, 0, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	lv_obj_center(arrow);
	tinyui.appview_title = tinyui_label(tinyui.appview, "Weather", 57, 14, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	icon = tinyui_box(tinyui.appview, 82, 67, 76, 76, TINYUI_COLOR_BLUE, 21);
	tinyui.appview_icon = tinyui_label(icon, LV_SYMBOL_TINT, 0, 0, &lv_font_montserrat_24, TINYUI_COLOR_TEXT);
	lv_obj_center(tinyui.appview_icon);
	message = tinyui_label(tinyui.appview, "App preview", 0, 162, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	lv_obj_set_width(message, TINYUI_WIDTH);
	lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, 0);
	message = tinyui_label(tinyui.appview, "Ready for integration", 0, 191, &lv_font_montserrat_16, TINYUI_COLOR_SUBTEXT);
	lv_obj_set_width(message, TINYUI_WIDTH);
	lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_hidden(tinyui.appview, 1);
}

static void tinyui_create(void)
{
	lv_obj_t * screen = lv_screen_active();
	lv_obj_t * status;
	int i;

	xos_memset(&tinyui, 0, sizeof(tinyui));
	tinyui.settings_section = -1;
	lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
	lv_obj_set_style_border_width(screen, 0, 0);
	lv_obj_set_style_pad_all(screen, 0, 0);
	lv_obj_set_scrollable(screen, 0);
	tinyui.root = tinyui_box(screen, 0, 0, TINYUI_WIDTH, TINYUI_HEIGHT, 0x0A0E18, 0);

	status = tinyui_box(tinyui.root, 0, 0, TINYUI_WIDTH, TINYUI_STATUS_HEIGHT, 0x111827, 0);
	lv_obj_set_style_bg_opa(status, LV_OPA_70, 0);
	tinyui.clock = tinyui_label(status, "00:00", 12, 7, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	tinyui_label(status, LV_SYMBOL_WIFI, 170, 7, &lv_font_montserrat_16, TINYUI_COLOR_TEXT);
	tinyui_label(status, LV_SYMBOL_BATTERY_FULL, 204, 7, &lv_font_montserrat_16, TINYUI_COLOR_GREEN);

	tinyui.pages = tinyui_box(tinyui.root, 0, TINYUI_STATUS_HEIGHT, TINYUI_WIDTH, TINYUI_PAGES_HEIGHT, 0x000000, 0);
	lv_obj_set_style_bg_opa(tinyui.pages, LV_OPA_TRANSP, 0);
	lv_obj_set_scrollable(tinyui.pages, 1);
	lv_obj_set_flex_flow(tinyui.pages, LV_FLEX_FLOW_ROW);
	lv_obj_set_scroll_dir(tinyui.pages, LV_DIR_HOR);
	lv_obj_set_scroll_snap_x(tinyui.pages, LV_SCROLL_SNAP_CENTER);
	lv_obj_set_scroll_one(tinyui.pages, 1);
	lv_obj_add_event_cb(tinyui.pages, tinyui_pages_event, LV_EVENT_SCROLL, NULL);
	for(i = 0; i < TINYUI_PAGE_COUNT; i++)
		tinyui_create_app_page(i);

	for(i = 0; i < TINYUI_PAGE_COUNT; i++)
	{
		tinyui.dots[i] = tinyui_box(tinyui.root, 74 + i * 18, 222, 7, 7, 0xFFFFFF, LV_RADIUS_CIRCLE);
		lv_obj_set_clickable(tinyui.dots[i], 1);
		lv_obj_add_event_cb(tinyui.dots[i], tinyui_dot_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
	}
	tinyui_update_dots(0);
	tinyui_create_appview();
	tinyui_create_settings();
	tinyui_update_time(NULL);
	tinyui.timer = lv_timer_create(tinyui_update_time, 1000, NULL);
}

static void tinyui_usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    tinyui [fb] [input]\r\n");
}

static int do_tinyui(int argc, char ** argv)
{
	const char * fb = argc > 1 ? argv[1] : NULL;
	const char * input = argc > 2 ? argv[2] : NULL;
	struct xlvgl_context_t * ctx = xlvgl_context_alloc(fb, input, -1);

	if(ctx)
	{
		tinyui_create();
		while(1)
		{
			xlvgl_context_step(ctx);
			if(shell_ctrlc())
				break;
		}
		lv_timer_delete(tinyui.timer);
		xlvgl_context_free(ctx);
	}
	return 0;
}

static struct command_t cmd_tinyui = {
	.name	= "tinyui",
	.desc	= "Tiny lvgl application launcher for 240x240 displays",
	.usage	= tinyui_usage,
	.exec	= do_tinyui,
};

static void tinyui_cmd_init(void)
{
	register_command(&cmd_tinyui);
}

static void tinyui_cmd_exit(void)
{
	unregister_command(&cmd_tinyui);
}

command_initcall(tinyui_cmd_init);
command_exitcall(tinyui_cmd_exit);
