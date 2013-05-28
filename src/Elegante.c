#include "pebble_os.h"
#include "pebble_app.h"

#define MY_UUID { 0xBB, 0xA9, 0xDE, 0xB9, 0x57, 0xAC, 0x48, 0x26, 0xAD, 0x9B, 0xA2, 0x35, 0xF5, 0x60, 0xAF, 0x29 }
PBL_APP_INFO(MY_UUID,
             "l'Elegante", "Dale Perkel",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

#define HOUR_VIBRATION true
#define SECONDS false

Window window;

Layer circle_dot_layer;
Layer center_circle_layer;

Layer minute_hand;
Layer hour_hand;
TextLayer date_display_layer;
GFont date_day_font;
int firstrun = 3;
GPoint center;
GPoint hour_dots[12];

#if SECONDS
Layer second_hand;
GPath second_hand_gpath;
const GPathInfo second_hand_points= {
        2,
        (GPoint []) {
                { 0, -64},
                { 0, 10},
        }
};
#endif

GPath minute_hand_gpath;
GPathInfo minute_hand_points= {
	5,
	(GPoint []) {
		{0,-60},
        {3,-58},
        {4,13},
        {-4,13},
        {-3,-58},
	}                
};

GPath hour_hand_gpath;
const GPathInfo hour_hand_points= {
	5,
        (GPoint []) {
        {0,-50},
		{3,-48},
		{4,13},
		{-4,13},
		{-3,-48},
	}
};

 
char *upcase(char *str)
{
	for (int i = 0;str[i] !=0;i++){
		if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 0x20;
	}

	return str;
}

//Creates the 12 dots representing the hour and outer circle
void circle_dot_layer_update(Layer *me, GContext* ctx) {
	(void)me;
	
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);

	graphics_fill_circle(ctx, center, 71);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, center, 68);
	graphics_context_set_fill_color(ctx, GColorWhite);
	
	for(int i = 0; i < 12; i++) {
        	graphics_fill_circle(ctx, hour_dots[i], 2);
	}
}

//Creates the inner circle
void center_circle_layer_update(Layer *me, GContext* ctx) {
	(void)me;
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, center, 5);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_circle(ctx, center, 2);
}

//Updates the seconds when called to the number of seconds at the moment
#if SECONDS
void second_hand_update(Layer *me, GContext* ctx) {
	(void)me;
	 
	PblTm t;
  	get_time(&t);

	graphics_context_set_stroke_color(ctx, GColorWhite);	

	int second_angle = t.tm_sec * (0xffff / 60 );
    gpath_rotate_to(&second_hand_gpath, second_angle);
    gpath_draw_outline(ctx, &second_hand_gpath);
}
#endif

//Updates the minutes when called to the number of minutes at the moment
void minute_hand_update(Layer *me, GContext* ctx) {
    (void)me;

    PblTm t;
    get_time(&t);

    graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorWhite);

	int minute_angle = t.tm_min * (0xffff / 60 );
	gpath_rotate_to(&minute_hand_gpath, minute_angle);
	gpath_draw_filled(ctx, &minute_hand_gpath);
}

//Updates the hours when called to the number of hours at the moment
void hour_hand_update(Layer *me, GContext* ctx) {
    (void)me;

    PblTm t;
    get_time(&t);

	int hour;

    graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorWhite);

    if (t.tm_hour == 12)  hour = 0;
    else if (t.tm_hour > 12) hour = t.tm_hour - 12;
    else hour = t.tm_hour;
        
	int hour_angle = (( (hour*60) + t.tm_min) * (0xffff / 720 ));
	gpath_rotate_to(&hour_hand_gpath, hour_angle);
    gpath_draw_filled(ctx, &hour_hand_gpath);
}

#if HOUR_VIBRATION
const VibePattern hour_pattern = {
        .durations = (uint32_t []) {100, 100, 100, 100, 100},
        .num_segments = 5
};
#endif

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "l'Elegante");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);
  
  center = GPoint (72,72);
	
  int angle;
  int array_counter;
  int hour_dots_position_from_center=65;

  for(array_counter=0; array_counter < 12; array_counter++) {
        angle = array_counter * (0xffff/12);
	hour_dots[array_counter] = GPoint (center.x + hour_dots_position_from_center * sin_lookup(angle)/0xffff,
						center.y + (-hour_dots_position_from_center) * cos_lookup(angle)/0xffff);
  }
  
  //Init Minute Hand
  gpath_init(&minute_hand_gpath, &minute_hand_points);
  gpath_move_to(&minute_hand_gpath, center);

  //Init Hour Hand
  gpath_init(&hour_hand_gpath, &hour_hand_points);
  gpath_move_to(&hour_hand_gpath, center);

  //Circle and 12 O'Clock Dot
  layer_init(&circle_dot_layer, window.layer.frame);
  circle_dot_layer.update_proc = &circle_dot_layer_update;
  layer_add_child(&window.layer, &circle_dot_layer);
    
   #if SECONDS
  //Init Second Hand
  gpath_init(&second_hand_gpath, &second_hand_points);
  gpath_move_to(&second_hand_gpath, center);
  
  //Second hand Layer
  layer_init(&second_hand, window.layer.frame);
  second_hand.update_proc = &second_hand_update;
  layer_add_child(&window.layer, &second_hand);
  #endif
	
  //Minute hand layer
  layer_init(&minute_hand, window.layer.frame);
  minute_hand.update_proc = &minute_hand_update;
  layer_add_child(&window.layer, &minute_hand);

  //Minute hand layer
  layer_init(&hour_hand, window.layer.frame);
  hour_hand.update_proc = &hour_hand_update;
  layer_add_child(&window.layer, &hour_hand);

  //Center Circle
  layer_init(&center_circle_layer, window.layer.frame);
  center_circle_layer.update_proc = &center_circle_layer_update;
  layer_add_child(&window.layer, &center_circle_layer);
    
  //Define Fonts
  date_day_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_REGULAR_16));

  //Date_Display Layer
  text_layer_init(&date_display_layer, GRect(0, 148, 144, 20));
  text_layer_set_text_color(&date_display_layer, GColorWhite);
  text_layer_set_background_color(&date_display_layer, GColorClear);
  text_layer_set_font(&date_display_layer, date_day_font);
  text_layer_set_text_alignment(&date_display_layer, GTextAlignmentCenter);
  text_layer_set_text(&date_display_layer, "l'elegante by:");
  layer_add_child(&window.layer, &date_display_layer.layer);

}

void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  (void)t;

  if (firstrun == 3) firstrun=2;
  else if (firstrun == 2) {
	firstrun = 1;
	text_layer_set_text(&date_display_layer, "Dale Perkel");
  }

  #if SECONDS
  else if (t->units_changed & SECOND_UNIT || firstrun == 1) {
  #else
  else if (t->units_changed & MINUTE_UNIT || firstrun == 1) {
  #endif
  	PblTm pebble_time;
  	get_time(&pebble_time);	
  
  	//Update display 
  	layer_mark_dirty(&minute_hand);

  	// Update at midnight or on first run
  	if ((pebble_time.tm_hour == 0 && pebble_time.tm_min == 0 && pebble_time.tm_sec == 0) || firstrun == 1) {

		// Set day name text
		static char day_text[] = "Xxxxxxxxx";
		string_format_time(day_text, sizeof(day_text), "%a", t->tick_time);

		// Set date text
		static char date_text[] = "00 Xxxxxxxxx";
		string_format_time(date_text, sizeof(date_text), "%d", t->tick_time);
	
		upcase(day_text);
		upcase(date_text);
	
		static char *date_display_text = "XXX 00";
		memset(date_display_text,'\0',sizeof(date_display_text));
		strcat(date_display_text,day_text);
		strcat(date_display_text," ");
		strcat(date_display_text,date_text);
	
		text_layer_set_text(&date_display_layer, date_display_text);
  	}

  	// Vibrate Every Hour
  	#if HOUR_VIBRATION
  	if (pebble_time.tm_min == 0 && pebble_time.tm_sec == 0 ){
		vibes_enqueue_custom_pattern(hour_pattern);
  	}
  	#endif

  	//Set firstrun to 0
  	if (firstrun == 1) {
        firstrun = 0;
  	}
  }
}
  
void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  fonts_unload_custom_font(date_day_font);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
	 .tick_handler = &handle_tick,
	 .tick_units = SECOND_UNIT
	}  	
  };
  app_event_loop(params, &handlers);
}
