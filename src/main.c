#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;

static GFont s_time_font;
static GFont s_day_font;
static GFont s_date_font;

static void update_time() {
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    static char buffer[] = "00:00";

    if(clock_is_24h_style() == true) {
        strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
    } else {
        strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
    }
    
    static char date_buffer[30] = "";
    strftime(date_buffer, 30, "%b %e, %Y", tick_time);
    
    static char day_buffer[10] = "";
    strftime(day_buffer, 10, "%A", tick_time);
    
    text_layer_set_text(s_day_layer, day_buffer);
    text_layer_set_text(s_date_layer, date_buffer);
    text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void main_window_load(Window *window) {
    window_set_background_color(s_main_window, GColorBlack);
    
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ORBITRON_40));
    s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_14));
    s_day_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_14));

    // Time layer
    s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
    text_layer_set_background_color(s_time_layer, GColorBlack);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    s_date_layer = text_layer_create(GRect(0, 146, 142, 18));
    text_layer_set_text_color(s_date_layer,  GColorWhite);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_font(s_date_layer, s_date_font);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    
    s_day_layer = text_layer_create(GRect(0, 128, 142, 20));
    text_layer_set_text_color(s_day_layer,  GColorWhite );
    text_layer_set_background_color(s_day_layer,  GColorClear);
    text_layer_set_font(s_day_layer, s_day_font);
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    fonts_unload_custom_font(s_time_font);
}

static void init() {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    
    window_stack_push(s_main_window, true);
    
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}