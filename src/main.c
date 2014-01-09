#include <pebble.h>

/********************
	 vHOUR_LOC
	+--------+ 
	| 12  59 | NUM_LABEL_OFFSET
	|,--.,--.| BAR_MAX_LOC
	||  ||  || 
	||  ||  || 
	|+--++--+| BAR_MIN_LOC 
	|HOURMINS| TEXT_LABEL_MAX_LOC
	+--------+ TEXT_LABEL_MIN_LOC
	     ^MINUTE_LOC
*********************/
	
#define BACKGROUND_COLOR GColorWhite
#define FOREGROUND_COLOR GColorBlack

#define CORNER_MASK GCornersTop
#define CORNER_SIZE 12
#define BAR_MAX_LOC 22
#define BAR_MIN_LOC 150
#define MAX_HEIGHT 128 //(BAR_MIN_LOC - BAR_MAX_LOC)
#define HOUR_LOC 4
#define MINUTE_LOC 74
#define HOUR_WIDTH 66
#define MINUTE_WIDTH 66
#define HOUR_UNIT_HEIGHT (MAX_HEIGHT/12.0f)
#define MINUTE_UNIT_HEIGHT (MAX_HEIGHT/60.0f)

float adjusted_hour_unit_height = HOUR_UNIT_HEIGHT;

#define TEXT_LABEL_MAX_LOC 142
#define TEXT_LABEL_MIN_LOC 168
#define TEXT_LABEL_HEIGHT 26 //(TEXT_LABEL_MIN_LOC - TEXT_LABEL_MAX_LOC)
#define NUM_LABEL_OFFSET 36 //(BAR_MAX_LOC + 14)
#define NUM_LABEL_HEIGHT 46

#define HOUR_LABEL_TEXT "HOUR"
#define MINUTE_LABEL_TEXT "MINS"
	
GFont *text_font;
GFont *num_font;

int16_t hour = 0;
int16_t minute = 0;

char *time_format; //format of hours depends on watch's hour mode

static char hour_text[] = "00"; 
static char minute_text[] = "00";

Window *window;
Layer *bar_layer;
Layer *root_layer;

static void bar_layer_draw(Layer *layer, GContext *ctx) 
{
	int16_t x, y, w, h;
	
	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_text_color(ctx, FOREGROUND_COLOR);
	
	x = HOUR_LOC;
	y = BAR_MIN_LOC - (int)(hour * adjusted_hour_unit_height);
	w = HOUR_WIDTH;
	h = (int)(hour * adjusted_hour_unit_height);

	graphics_fill_rect(ctx, GRect( x, y, w, h ), CORNER_SIZE, CORNER_MASK);
  	graphics_draw_text(ctx, hour_text, num_font, GRect(0, (y - NUM_LABEL_OFFSET), 72, NUM_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, HOUR_LABEL_TEXT, text_font, GRect(0, TEXT_LABEL_MAX_LOC, 72, TEXT_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

	x = MINUTE_LOC;
	y = BAR_MIN_LOC - (int)(minute * MINUTE_UNIT_HEIGHT);
	w = MINUTE_WIDTH;
	h = (int)(minute * MINUTE_UNIT_HEIGHT);

	graphics_fill_rect(ctx, GRect( x, y, w, h ), CORNER_SIZE, CORNER_MASK);
	graphics_draw_text(ctx, minute_text, num_font, GRect(72, (y - NUM_LABEL_OFFSET), 72, NUM_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, MINUTE_LABEL_TEXT, text_font, GRect(72, TEXT_LABEL_MAX_LOC, 72, TEXT_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
	minute = tick_time->tm_min;
	hour = tick_time->tm_hour;

	if(clock_is_24h_style() == false)
	{
		if(hour > 12){hour -= 12;}		
		else if(hour == 0){hour = 12;}	//for correct hieght bar in 12 hour mode
	}
	
	strftime(minute_text, sizeof(minute_text), "%M", tick_time);
	strftime(hour_text, sizeof(hour_text), time_format, tick_time);
	
	layer_mark_dirty(bar_layer);
}

void init(void) 
{
	time_t t;
	struct tm *now;	
	now = localtime(&t);
	hour = now->tm_hour;
	minute = now->tm_min;
	
	if(clock_is_24h_style())
	{
		adjusted_hour_unit_height = HOUR_UNIT_HEIGHT / 2.0f; //for correct hieght bar in 24 hour mode
		time_format = "%H";		//	%H 	= 00 - 23; %k = 0 - 23
	}
	else
	{
		time_format = "%l"; // %I = 01 - 12; %l = 1 - 12
	}

	text_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TYPEONE_24));
	num_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TYPEONE_34));
	
	window = window_create();
	window_stack_push(window, true);
	window_set_background_color(window, BACKGROUND_COLOR);
	root_layer = window_get_root_layer(window);
	
	bar_layer = layer_create(layer_get_frame(root_layer));
	layer_set_update_proc(bar_layer, bar_layer_draw);
	layer_add_child(root_layer, bar_layer);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);	
}

void deinit(void) {
	fonts_unload_custom_font(text_font);
	fonts_unload_custom_font(num_font);
	layer_destroy(bar_layer);
	window_destroy(window);
}

int main(void) 
{
	init();
	app_event_loop();
	deinit();
}