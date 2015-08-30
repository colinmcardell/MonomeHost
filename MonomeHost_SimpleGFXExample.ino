#include <Usb.h>  
#include "MonomeController.h"

USBHost usb;
MonomeController monome(usb);

typedef enum {
  eGridDisplayStateHorizontalVerticalLines,
  eGridDisplayStateLinesFromCorners,
  eGridDisplayStateRectFromPoint,
  eGridDisplayStateCircleFromPoint
} grid_display_state_t;

grid_display_state_t display_state;
bool displaying_menu;

void grid_key_callback(byte x, byte y, byte z) { 
  Serial.print("\r\nMonome Grid Key: {");
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print("} : ");
  Serial.print(z);
  Serial.print(";");
  
  bool is_key_down_callback = (bool)z;
  bool is_menu_key = (x == monome.size_x() - 1 && y == 0);
  
  if (is_menu_key) {
    
    // menu button value did change
    monome.grid_led_clear();
    if (is_key_down_callback) {
      displaying_menu = true;
      draw_menu();
    } else {
      displaying_menu = false;
    }
    
  } else if (displaying_menu) {
    
    // menu is displaying, key changes will alter `display_state` if value is
    // within the available `grid_display_state_t`'s (as x values).
    if (is_key_down_callback) {
      if (x <= eGridDisplayStateCircleFromPoint && x >= eGridDisplayStateHorizontalVerticalLines) {
        display_state = (grid_display_state_t)x;
      }
    }
    
    monome.grid_led_clear();
    draw_menu();
    
  } else {
    
    // key value for current `display_state` did change and grid needs to be
    // updated.
    uint8_t val;
    if (is_key_down_callback) {
        val = 7;
    } else {
        val = 0;
    }
    
    switch (display_state) {
      case eGridDisplayStateHorizontalVerticalLines:
        draw_horizontal_and_vertical_lines(x, y, val);
        break;
      case eGridDisplayStateLinesFromCorners:
        draw_lines_from_corners(x, y, val);
        break;
      case eGridDisplayStateRectFromPoint:
        draw_rect_from_point(x, y, val);
        break;
      case eGridDisplayStateCircleFromPoint:
        draw_circle_from_point(x, y, val);
        break;
    }
  }
  
  // refresh grid
  monome.refresh();
}

void connect_callback(const char * name, byte cols, byte rows) {
  Serial.print("\r\nMonome Device Connected: Type - ");
  Serial.print(name);
  Serial.print(";\r\nMonome Device Connected: Columns - ");
  Serial.print(cols);
  Serial.print(";\r\nMonome Device Connected: Rows - ");
  Serial.print(rows);
  Serial.print(";\r\n");
}

void setup() { 
  // setup grid callbacks
  monome.SetConnectCallback(&connect_callback);
  monome.SetGridKeyCallback(&grid_key_callback);
  
  // default values
  display_state = eGridDisplayStateHorizontalVerticalLines;
  displaying_menu = false;

  Serial.begin(115200);
  Serial.print("\r\nInitialized.\r\n");
  delay(200);
}

void loop() { 
  usb.Task();
}

/*
 *  Various Drawing Modes:
 */
 
void draw_menu() {
  for (uint8_t i = 0; i <= eGridDisplayStateCircleFromPoint; ++i) {
    uint8_t val = 2;
    if (i == display_state) {
      val = 15;
    }
    monome.led_set(i, 0, val);
  }
  monome.led_set(monome.size_x() - 1, 0, 15);
}

void draw_horizontal_and_vertical_lines(byte x, byte y, uint8_t val) {
  uint8_t size_x = monome.size_x();
  uint8_t size_y = monome.size_y();
  
  // Horizontal Line
  uint8_t h_start;
  uint8_t h_length;
  if (x >= (size_x / 2)) {
    h_start = x;
    h_length = size_x - h_start;
  } else {
    h_start = 0;
    h_length = x + 1;
  }
  monome.grid_draw_h_line(h_start, y, h_length, val);
  
  // Vertical Line
  uint8_t v_start;
  uint8_t v_length;
  if (y >= (size_y / 2)) {
    v_start = y;
    v_length = size_y - v_start;
  } else {
    v_start = 0;
    v_length = y + 1;
  }
  monome.grid_draw_v_line(x, v_start, v_length, val);
}

void draw_lines_from_corners(byte x, byte y, uint8_t val) {
  uint8_t size_x = monome.size_x();
  uint8_t size_y = monome.size_y();
  
  uint8_t start_x, start_y;
  
  if (x < (size_x / 2)) {
    start_x = 0;
  } else {
    start_x = size_x - 1;
  }
  
  if (y < (size_y / 2)) {
    start_y = 0;
  } else {
    start_y = size_y - 1;
  }
  
  monome.grid_draw_line(start_x, start_y, x, y, val);
}

void draw_rect_from_point(byte x, byte y, uint8_t val) {
  int8_t x0 = x - 1;
  int8_t y0 = y - 1;
  monome.grid_draw_rect(x0, y0, 3, 3, val);
}

void draw_circle_from_point(byte x, byte y, uint8_t val) {
  monome.grid_fill_circle(x, y, 2, val);
}