#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_LOCATION_NAME 2

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_location_layer;

static GFont s_time_font;
static GFont s_medium_font;
static GFont s_small_font;

// Store incoming information
static char temperature_buffer[8];
static char conditions_buffer[32];
static char location_buffer[128];
static char weather_layer_buffer[32];

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Read first item
    Tuple *t = dict_read_first(iterator);

    APP_LOG(APP_LOG_LEVEL_ERROR, "message received");
    
    // For all items
    while(t != NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "in while");
        // Which key was received?
        switch(t->key) {
            case KEY_TEMPERATURE:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d received: %d", (int)t->key, (int)t->value->int32);
                snprintf(temperature_buffer, sizeof(temperature_buffer), "%d °C", (int)t->value->int32);
                break;
            case KEY_CONDITIONS:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d received: %s", (int)t->key, t->value->cstring);
                snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
                break;
            case KEY_LOCATION_NAME:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d received: %s", (int)t->key, t->value->cstring);
                snprintf(location_buffer, sizeof(location_buffer), "%s", t->value->cstring);
                text_layer_set_text(s_location_layer, location_buffer);
                break;
            default:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
                break;
        }

        // Look for next item
        t = dict_read_next(iterator);
    }
    
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

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
    // Get weather update every 30 minutes
    if(tick_time->tm_min % 30 == 0) {
        // Begin dictionary
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);

        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);

        // Send the message!
        app_message_outbox_send();
    }

    update_time();
}

static void main_window_load(Window *window) {
    window_set_background_color(s_main_window, GColorBlack);
    
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ORBITRON_40));
    s_medium_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_15));
    s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_14));
    
    // Time layer
    s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
    text_layer_set_background_color(s_time_layer, GColorBlack);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    // Date layer
    s_date_layer = text_layer_create(GRect(0, 146, 142, 18));
    text_layer_set_text_color(s_date_layer,  GColorWhite);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_font(s_date_layer, s_medium_font);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    
    // Day layer
    s_day_layer = text_layer_create(GRect(0, 128, 142, 20));
    text_layer_set_text_color(s_day_layer,  GColorWhite );
    text_layer_set_background_color(s_day_layer,  GColorClear);
    text_layer_set_font(s_day_layer, s_medium_font);
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
    
    // Weather layer
    s_weather_layer = text_layer_create(GRect(0, 10, 144, 25));
    text_layer_set_background_color(s_weather_layer, GColorClear);
    text_layer_set_text_color(s_weather_layer, GColorWhite);
    text_layer_set_font(s_weather_layer, s_medium_font);
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentRight);
    text_layer_set_text(s_weather_layer, "Loading...");
    
    // Location layer
    s_location_layer = text_layer_create(GRect(0, 25, 144, 25));
    text_layer_set_background_color(s_location_layer, GColorClear);
    text_layer_set_text_color(s_location_layer, GColorWhite);
    text_layer_set_font(s_location_layer, s_small_font);
    text_layer_set_text_alignment(s_location_layer, GTextAlignmentRight);
    text_layer_set_text(s_location_layer, " ");
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_location_layer));
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_day_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_weather_layer);
    text_layer_destroy(s_location_layer);
    
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
    
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}