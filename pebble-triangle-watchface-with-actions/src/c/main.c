#include <pebble.h>

static Window *s_window;
static Layer *s_divider_layer;
static TextLayer *s_hour_layer, *s_min_layer, *s_date_layer, *s_day_layer;

static GRect padded_bounds(Layer *layer, int pad) {
  GRect b = layer_get_bounds(layer);
  b.origin.x += pad;
  b.origin.y += pad;
  b.size.w -= pad*2;
  b.size.h -= pad*2;
  return b;
}

static void update_time() {
  static char s_hour_buf[3];
  static char s_min_buf[3];
  static char s_date_buf[3];
  static char s_day_buf[4];

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  // Hours
  if(clock_is_24h_style()) {
    strftime(s_hour_buf, sizeof(s_hour_buf), "%H", t);
  } else {
    strftime(s_hour_buf, sizeof(s_hour_buf), "%I", t);
    if (s_hour_buf[0] == '0') memmove(s_hour_buf, s_hour_buf+1, 2);
  }
  // Minutes
  strftime(s_min_buf, sizeof(s_min_buf), "%M", t);

  // Date & day
  strftime(s_date_buf, sizeof(s_date_buf), "%d", t);
  strftime(s_day_buf, sizeof(s_day_buf), "%a", t);

  text_layer_set_text(s_hour_layer, s_hour_buf);
  text_layer_set_text(s_min_layer, s_min_buf);
  text_layer_set_text(s_date_layer, s_date_buf);
  text_layer_set_text(s_day_layer, s_day_buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void divider_update_proc(Layer *layer, GContext *ctx) {
  GRect b = padded_bounds(layer, 0);
  int16_t midx = b.origin.x + b.size.w/2;
  // Apex point ~60% height
  int16_t apex_y = b.origin.y + (b.size.h * 3) / 5;
  GPoint top_center = GPoint(midx, b.origin.y);
  GPoint apex = GPoint(midx, apex_y);
  GPoint bl = GPoint(b.origin.x, b.origin.y + b.size.h - 1);
  GPoint br = GPoint(b.origin.x + b.size.w - 1, b.origin.y + b.size.h - 1);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);

  // Vertical top -> apex
  graphics_draw_line(ctx, top_center, apex);
  // Diagonals apex -> bottom corners
  graphics_draw_line(ctx, apex, bl);
  graphics_draw_line(ctx, apex, br);
}

static void main_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect b = layer_get_bounds(root);

  window_set_background_color(window, GColorBlack);

  // Divider layer draws the Y shape
  s_divider_layer = layer_create(b);
  layer_set_update_proc(s_divider_layer, divider_update_proc);
  layer_add_child(root, s_divider_layer);

  // Geometry for text layers
  int16_t pad = PBL_IF_ROUND_ELSE(12, 6);
  int16_t midx = b.size.w/2;
  int16_t apex_y = (b.size.h * 3) / 5;

  // Left (Hours)
  s_hour_layer = text_layer_create(GRect(pad, pad, midx - pad*1.5, apex_y - pad));
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  text_layer_set_font(s_hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(root, text_layer_get_layer(s_hour_layer));

  // Right (Minutes)
  s_min_layer = text_layer_create(GRect(midx + pad/2, pad, midx - pad*1.5, apex_y - pad));
  text_layer_set_text_alignment(s_min_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_min_layer, GColorClear);
  text_layer_set_text_color(s_min_layer, GColorWhite);
  text_layer_set_font(s_min_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(root, text_layer_get_layer(s_min_layer));

  // Bottom triangle: date and day stacked
  int16_t bottom_h = b.size.h - apex_y;
  s_date_layer = text_layer_create(GRect(pad, apex_y + pad/2, b.size.w - pad*2, bottom_h/2));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  layer_add_child(root, text_layer_get_layer(s_date_layer));

  s_day_layer = text_layer_create(GRect(pad, apex_y + bottom_h/2 - pad/2, b.size.w - pad*2, bottom_h/2));
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(root, text_layer_get_layer(s_day_layer));

  update_time();
}

static void main_window_unload(Window *window) {
  layer_destroy(s_divider_layer);
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_min_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
