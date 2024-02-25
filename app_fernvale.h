//
// This class implements the display logic for an environmetal sensor
//
#pragma once
#include <set>

#if SCREEN_T14
#include "ESP32Tone.h"

//RTC_DATA_ATTR int saved_something = 0;

unsigned long app_count=0;

void ARDUINO_ISR_ATTR counterISR(void) {
  app_count++;
}

enum fernvale_button {
  NONE,
  BUT_1,
  BUT_2,
  BUT_3,
  DOWN,
  CENTER,
  RIGHT,
  LEFT,
  UP,
  BUT_C,
  BUT_B,
  BUT_A
};
#define BUT_COUNT ((int)BUT_A)

const char *fernvale_button_names[] = {
  "NONE",
  "BUT_1",
  "BUT_2",
  "BUT_3",
  "DOWN",
  "CENTER",
  "RIGHT",
  "LEFT",
  "UP",
  "BUT_C",
  "BUT_B",
  "BUT_A" };

enum fernvale_button fernvale_last_button;
bool fernvale_last_button_is_press=false;

#define PRESS(e) (e?"PRESS":"RELEASE")

void fernvaleKeypadRead(lv_indev_drv_t *indev, lv_indev_data_t *data);
void fernvaleEncoderRead(lv_indev_drv_t *indev, lv_indev_data_t *data);

#endif // T14

#define CLASS_APP FernvaleAppLeaf
#include "abstract_app_lvgl.h"

class FernvaleAppLeaf : public LVGLAppLeaf
{
protected: // config
  bool mute=false;

  int battery_millivolts = -1;
  float millivolts = nan("novolts");
  float milliamps = nan("noamps");

  int refresh_msec = 200;

  int counter_pin = 27;
  int pwm_pin = 13;
  int pwm_freq = 2500;
  
  int battery_threshold = 10;
  int millivolts_threshold = 1;
  int milliamps_threshold = 1;


protected: // ephemeral state
  unsigned long last_report = 0;
  unsigned long last_report_count = 0;
  unsigned long update_limit_msec = 500;
  bool mode_hz = true; // hz or ppm
  bool counter_run = false;
  bool pwm_run = false;
  unsigned long last_interval_time = 0;
  unsigned long last_interval_count = 0;

  bool buttons[BUT_COUNT];
  std::set<byte> devices;
  uint8_t selected_device = 0;
  
protected: // LVGL screen resources

  lv_style_t style_info;
  lv_style_t style_clock;
  lv_style_t style_count;
  lv_style_t style_freq;
  lv_style_t style_btn;

  lv_obj_t *screen_top = NULL;
  lv_obj_t *top_clock = NULL;
  lv_obj_t *top_host = NULL;
  lv_obj_t *top_vers = NULL;
  lv_obj_t *top_ip = NULL;
  lv_obj_t *top_comms = NULL;
  lv_obj_t *top_batt = NULL;

  lv_obj_t *top_btn1 = NULL;
  lv_obj_t *top_btn2 = NULL;
  lv_obj_t *top_btn3 = NULL;
  lv_obj_t *top_btn4 = NULL;

  lv_obj_t *screen_home = NULL;
  lv_group_t *group_home = NULL;
  lv_obj_t *home_btn1 = NULL;
  lv_obj_t *home_btn2 = NULL;
  lv_obj_t *home_btn3 = NULL;
  
  lv_obj_t *screen_freq = NULL;
  lv_group_t *group_freq = NULL;
  lv_obj_t *freq_count = NULL;
  lv_obj_t *freq_countfreq = NULL;
  lv_obj_t *freq_pwmfreq = NULL;
  lv_obj_t *freq_millivolts = NULL;
  lv_obj_t *freq_milliamps = NULL;
  lv_obj_t *freq_home = NULL;

  lv_obj_t *screen_i2c = NULL;
  lv_group_t *group_i2c = NULL;
  lv_obj_t *i2c_list = NULL;
  lv_obj_t *i2c_refresh = NULL;
  lv_obj_t *i2c_home = NULL;
  lv_obj_t *selected_device_obj = NULL;


  lv_obj_t *screen_settings = NULL;
  lv_obj_t *settings_home = NULL;
  lv_group_t *group_settings = NULL;

protected: // ephemeral state
  unsigned long last_refresh = 0;

public:
  FernvaleAppLeaf(String name, String target=NO_TAPS)
    : LVGLAppLeaf(name,target,&fernvaleKeypadRead,&fernvaleEncoderRead)
    , Debuggable(name)
  {
  }

  void refresh() {
    LEAF_ENTER(L_DEBUG);

    char buf[64];
    time_t now;
    struct tm timeinfo;
  
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(buf, sizeof(buf), "%X", &timeinfo);
    lv_label_set_text(top_clock, buf);

    if (battery_millivolts >= 0) {
      snprintf(buf, sizeof(buf), "%.2fV", battery_millivolts/1000.0);
      lv_label_set_text(top_batt, buf);
    }

    lv_timer_handler();
    LEAF_LEAVE;
  }

  virtual void loop() {
    AbstractAppLeaf::loop();
    unsigned long now=millis();
    
    if (now > (last_refresh+refresh_msec)) {
      refresh();
      last_refresh=now;
    }
  }
      
  virtual void setup(void) {
    LVGLAppLeaf::setup();
    LEAF_ENTER(L_NOTICE);

    registerLeafIntValue("counter_pin", &pwm_pin);
    registerLeafBoolValue("counter_run", &counter_run);
    registerLeafBoolValue("pwm_run", &pwm_run);
    registerLeafIntValue("pwm_pin", &pwm_pin);
    registerLeafIntValue("pwm_freq", &pwm_freq);

    pinMode(counter_pin, INPUT_PULLUP);
    pinMode(pwm_pin, OUTPUT);

    registerCommand(HERE, "freq", "Set pwm frequency on pin 13");

    LEAF_LEAVE;
  }
  
  void i2c_scan()
  {
    LEAF_ENTER(L_NOTICE);
    byte error, address;
    int nDevices;
    char buf[40];

    devices.clear();
    LEAF_NOTICE("Scanning I2C...");
    if (i2c_list) {
      lv_obj_clean(i2c_list);
      lv_list_add_text(i2c_list, "Scanning...");
    }

    nDevices = 0;
    for(address = 1; address < 127; address++ )
    {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.
      Wire.beginTransmission(address);
      error = Wire.endTransmission();

      if (error == 0)
      {
	LEAF_NOTICE("    I2C device detected at address 0x%02x (%d)", (int)address, (int)address);
	devices.insert(address);
	nDevices++;
      }
      else if (error==4)
      {
	LEAF_NOTICE("Unknown error at I2C address 0x%02x", (int)address);
      }
    }

    if (i2c_list) {
      lv_obj_clean(i2c_list);
      selected_device_obj = NULL;
      snprintf(buf, sizeof(buf), "%d device%s found", nDevices, (nDevices==1)?"":"s");
      lv_list_add_text(i2c_list, buf);
    }
    if (nDevices == 0) {
      LEAF_NOTICE("No I2C devices found");
    }
    else {
      LEAF_NOTICE("I2C scan complete, nDevices=%d", nDevices);
      for (auto it = devices.begin(); it != devices.end(); ++it) {
	int addr = (int)*it;
	const char *icon = NULL;
	snprintf(buf, sizeof(buf), "%02x", addr);
	if (i2c_list) {
	  lv_obj_t *btn = lv_list_add_btn(i2c_list, icon, buf);
	}
      }
    }
    LEAF_LEAVE;
  }

  virtual void lvglAppHandleEvent(lv_event_t *e, lv_obj_t *obj, lv_event_code_t code)  
  {
    lv_obj_t *active_screen = lv_scr_act();
    lv_dir_t gesture_dir = LV_DIR_NONE;
    
    if (isInputEvent(code)) {
      // input events,
      LEAF_ENTER_INT(L_INFO, (int)code);
      LEAF_NOTICE("Input event %p <= %s", obj, lvglEventName(code));
      if (code == LV_EVENT_GESTURE) {
	gesture_dir = lv_indev_get_gesture_dir(lv_indev_get_act());
	LEAF_NOTICE("Gesture event %s", lvglGestureName(gesture_dir));
      }

#define SCREEN_EVENT(s,o,c) ((active_screen==(screen_##s)) && (obj == (s##_##o)) && (code==(LV_EVENT_##c))) 
#define SCREEN_CHANGE(s) lv_scr_load(screen_##s); lvglAppEnableInputGroup(group_##s)
      
      // not an else-case
      if (SCREEN_EVENT(home, btn1, PRESSED)) {
	LEAF_NOTICE("home_btn1 event %d => change to FREQ screen", (int)code);
	SCREEN_CHANGE(freq);
      }
      else if (SCREEN_EVENT(home, btn2, PRESSED)) {
	LEAF_NOTICE("home_btn2 event %d => change to I2C screen", (int)code);
	SCREEN_CHANGE(i2c);
	i2c_scan();
      }
      else if (SCREEN_EVENT(home, btn3, PRESSED)) {
	LEAF_NOTICE("home_btn2 event %d => change to SETTINGS screen", (int)code);
	SCREEN_CHANGE(settings);
      }
      else if (SCREEN_EVENT(i2c, home, PRESSED)) {
	LEAF_NOTICE("i2c_home event %d => change to HOME screen", (int)code);
	SCREEN_CHANGE(home);
      }
      else if (SCREEN_EVENT(freq, home, PRESSED)) {
	LEAF_NOTICE("freq_home event %d => change to HOME screen", (int)code);
	SCREEN_CHANGE(home);
      }
      else if (SCREEN_EVENT(settings, home, PRESSED)) {
	LEAF_NOTICE("settings_home event %d => change to HOME screen", (int)code);
	SCREEN_CHANGE(home);
      }
    }
    else {
	    // drawing events
    }
  }

  virtual void i2cRefreshButtonEvent(lv_event_t *e, lv_obj_t *obj, lv_event_code_t code)  
  {
    if (isInputEvent(code)) {
      // input events,
      LEAF_NOTICE("Input event %d: %s", (int)code, lvglEventName(code));

      i2c_scan();
    }
  }
  
  virtual void start(void) {
    LEAF_ENTER(L_NOTICE);
    LVGLAppLeaf::start();

    if (screen) {
      debug_flush = true;
      LEAF_NOTICE("Screen size is %d x %d", screen_width, screen_height);

      lv_obj_t *screen_top = lv_layer_top();

      lvglAppStyleInit(&style_info, &lv_font_montserrat_10);
      lv_color_t c;
      lvglAppStyleInit(&style_btn, NULL, &(c=lv_palette_main(LV_PALETTE_GREY)));
      lvglAppStyleInit(&style_clock, &lv_font_montserrat_14, &(c=lv_palette_main(LV_PALETTE_BLUE_GREY)));

      top_clock = lvglAppLabelCreate(screen_top, &style_clock, LV_ALIGN_TOP_MID, 0, 2);
      top_host = lvglAppLabelCreate(screen_top, &style_info, LV_ALIGN_TOP_LEFT, 5, 5, device_id);
      top_vers = lvglAppLabelCreate(screen_top, &style_info, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
      lv_label_set_text_fmt(top_vers, "v%s b%d", FIRMWARE_VERSION, BUILD_NUMBER);
      top_comms = lvglAppLabelCreate(screen_top, &style_info, LV_ALIGN_BOTTOM_LEFT, 5, -5, "OFFLINE");
      top_ip = lvglAppLabelCreate(screen_top, &style_info, LV_ALIGN_BOTTOM_MID, 20, -5);
      top_batt = lvglAppLabelCreate(screen_top, &style_info, LV_ALIGN_TOP_RIGHT, -5, 0);
      top_btn1 = lvglAppLabelCreate(screen_top, &style_btn, LV_ALIGN_TOP_LEFT, 0, 30, "<NEXT");
      top_btn2 = lvglAppLabelCreate(screen_top, &style_btn, LV_ALIGN_TOP_LEFT, 0, 80, "<PREV");
      top_btn3 = lvglAppLabelCreate(screen_top, &style_btn, LV_ALIGN_TOP_LEFT, 0, 130, "<ENTR");
      top_btn4 = lvglAppLabelCreate(screen_top, &style_btn, LV_ALIGN_TOP_LEFT, 0, 180, "<RST");

#define CREATE_SCREEN(s) \
      screen_##s=lv_obj_create(NULL);		\
      group_##s=lv_group_create();		\
      lv_group_set_default(group_##s)

      CREATE_SCREEN(home);
      home_btn1 = lvglAppButtonCreate(screen_home, "Frequency Count", LV_ALIGN_CENTER, 0, -50);
      home_btn2 = lvglAppButtonCreate(screen_home, "I2C Bus", LV_ALIGN_CENTER, 0, 0);
      home_btn3 = lvglAppButtonCreate(screen_home, "Settings", LV_ALIGN_CENTER, 0, 50);

      CREATE_SCREEN(freq);
      lvglAppStyleInit(&style_count, &lv_font_unscii_16/*montserrat_48*/);
      lvglAppStyleInit(&style_freq, &lv_font_unscii_16/*montserrat_32*/);

      freq_home = lvglAppButtonCreate(screen_freq, LV_SYMBOL_HOME, LV_ALIGN_TOP_MID, -screen_width/4, 20);
      freq_count = lvglAppLabelCreate(screen_freq, &style_count, LV_ALIGN_TOP_LEFT, 55, 77, counter_run?"":"OFF");
      freq_countfreq = lvglAppLabelCreate(screen_freq, &style_freq, LV_ALIGN_TOP_RIGHT, -10, 77);
      freq_pwmfreq = lvglAppLabelCreate(screen_freq, &style_freq, LV_ALIGN_TOP_LEFT, 55, 130);
      if (pwm_run) {
	lv_label_set_text_fmt(freq_pwmfreq, "%dHz", pwm_freq);
      }
      else {
        lv_label_set_text_fmt(freq_pwmfreq, "OFF  (%dHz)", pwm_freq);
      }

      freq_millivolts = lvglAppLabelCreate(screen_freq, &style_freq, LV_ALIGN_TOP_LEFT, 55, 180);
      freq_milliamps = lvglAppLabelCreate(screen_freq, &style_freq, LV_ALIGN_TOP_RIGHT, -10, 180);
      
      CREATE_SCREEN(i2c);
      i2c_home = lvglAppButtonCreate(screen_i2c, LV_SYMBOL_HOME, LV_ALIGN_TOP_MID, -screen_width/4, 20);
      i2c_refresh = lvglAppButtonCreate(screen_i2c, LV_SYMBOL_REFRESH, LV_ALIGN_TOP_MID, screen_width/4, 20, LVGL_EVENT_HANDLER(i2cRefreshButtonEvent));
      i2c_list =  lv_list_create(screen_i2c);
      lv_obj_set_height(i2c_list, screen_height/2);
      lv_obj_align(i2c_list, LV_ALIGN_TOP_MID, 0, screen_height/4);
      
      CREATE_SCREEN(settings);
      settings_home = lvglAppButtonCreate(screen_settings, LV_SYMBOL_HOME, LV_ALIGN_TOP_MID, -screen_width/4, 20);

      lv_group_set_default(NULL);
      lv_scr_load(screen_home);
      lvglAppEnableInputGroup(group_home);
      stacx_heap_check(HERE, L_NOTICE);
    }
    else {
      LEAF_ALERT("No screen resource");
    }
    LEAF_LEAVE;
  }



#if SCREEN_T14
  void button(enum fernvale_button b, bool is_press)
  {
    LEAF_NOTICE("button %s(%d) %s", fernvale_button_names[b], (int)b , PRESS(is_press));
    //update();

    buttons[(int)b] = is_press;

    fernvale_last_button = b;
    fernvale_last_button_is_press = is_press;

    if (!is_press) return;

#if 0
    switch (b) {
    case NONE:
      break;
    case BUT_1:
    case BUT_A:
      app_count=0;
      last_interval_time=0;
      last_interval_count=0;
      if (counter_run) {
	lv_label_set_text(freq_count, "");
      }
      else {
	lv_label_set_text(freq_count, "OFF");
      }
      lv_label_set_text(freq_countfreq, "");
      break;
    case BUT_2:
    case BUT_B:
      setValue("counter_run", ABILITY(!counter_run), VALUE_SET_DIRECT, VALUE_SAVE);
      if (counter_run) {
	attachInterrupt(counter_pin, counterISR, RISING);
	tone(pwm_pin, 3000);
      }
      else {
	detachInterrupt(counter_pin);
	noTone(pwm_pin);
	if (app_count==0) {
	  lv_label_set_text(freq_count, "OFF");
	  lv_label_set_text(freq_countfreq, "");
	}
      }
      break;
    case BUT_3:
    case BUT_C:
      setValue("pwm_run", ABILITY(!pwm_run), VALUE_SET_DIRECT, VALUE_SAVE);
      if (pwm_run) {
	tone(pwm_pin, pwm_freq);
	lv_label_set_text_fmt(freq_pwmfreq, "%dHz", pwm_freq);
      }
      else {
	noTone(pwm_pin);
	lv_label_set_text_fmt(freq_pwmfreq, "OFF  (%dHz)", pwm_freq);
      }
      break;
    case LEFT:
      if (pwm_freq >0) {
	int new_freq = pwm_freq - 500;
	if (new_freq < 0) new_freq=0;
	setValue("pwm_freq", String(new_freq), VALUE_SET_DIRECT, VALUE_SAVE);
	if (pwm_freq < 0) pwm_freq=0;
	LEAF_NOTICE("PWM freq now %d", pwm_freq);
	if (pwm_run) {
	  tone(pwm_pin, pwm_freq);
	}
      }
      break;
    case RIGHT:
      if (pwm_freq <150000) {
	int new_freq = pwm_freq+=500;
	setValue("pwm_freq", String(new_freq), VALUE_SET_DIRECT, VALUE_SAVE);
	LEAF_NOTICE("PWM freq now %d", pwm_freq);
        if (pwm_run) {
	  tone(pwm_pin, pwm_freq);
	}
      }
      break;
    default:
      LEAF_NOTICE("  no handler");
    }
#endif
    
    refresh();
  }
#endif


  bool wants_topic(String type, String name, String topic)
  {
    if (topic.startsWith("_")) return true;
    return AbstractAppLeaf::wants_topic(type,name,topic);
  }

  bool mqtt_receive(String type, String name, String topic, String payload, bool direct)
  {
    LEAF_ENTER(L_DEBUG);
    bool handled = false;

    LEAF_INFO("RECV %s/%s => [%s <= %s]", type.c_str(), name.c_str(), topic.c_str(), payload.c_str());

    WHEN("_comms_state", {
	if (top_comms != NULL) {
	  lv_label_set_text(top_comms, payload.c_str());
	}
      })
    ELSEWHEN("_ip_connect", {
	lv_label_set_text(top_ip, ipLeaf->ipAddressString().c_str());
    })
    ELSEWHEN("_ip_disconnect", {
	lv_label_set_text(top_ip, "");
    })
    ELSEWHEN("_pubsub_connect", {
    })
    ELSEWHEN("_pubsub_disconnect", {
    })
    ELSEWHENFROM("btn1", "event/press", {
	button(BUT_1, true);
    })
    ELSEWHENFROM("btn1", "event/release", {
	button(BUT_1, false);
    })
    ELSEWHENFROM("btn2", "event/press", {
	button(BUT_2, true);
    })
    ELSEWHENFROM("btn2", "event/release", {
	button(BUT_2, false);
    })
    ELSEWHENFROM("btn3", "event/press", {
	button(BUT_3, true);
    })
      ELSEWHENFROM("btn3", "event/release", {
	  button(BUT_3, false);
    })
    ELSEWHENPREFIXAND("event/button/", name=="joc", {
	bool is_press = (payload=="press");
	WHEN("up"    , button(UP    , is_press))
	ELSEWHEN("down"  , button(DOWN  , is_press))
	ELSEWHEN("left"  , button(LEFT  , is_press))
	ELSEWHEN("right" , button(RIGHT , is_press))
	ELSEWHEN("center", button(CENTER, is_press))
	ELSEWHEN("but_a" , button(BUT_A , is_press))
	ELSEWHEN("but_b" , button(BUT_B , is_press))
	ELSEWHEN("but_c" , button(BUT_C , is_press))
	else {
	  handled=false;
	}
    })
    ELSEWHEN("status/button",{/*ignore these*/})
    ELSEWHENFROM("battlvl", "status/battery", {
	int new_value = payload.toInt();
	if ((battery_millivolts<0) || (abs(battery_millivolts - new_value) >= battery_threshold)) {
	  battery_millivolts = new_value;
	  refresh();
	  mqtt_publish("status/battery", String(new_value));
	}
	else {
	  battery_millivolts = new_value;
	}
    })
    ELSEWHENFROM("current", "status/millivolts", {
	float new_value = payload.toFloat();
	static unsigned long last_millivolts = 0;
	bool stale = millis() > (last_millivolts + 60000);
	
	if (isnan(millivolts) || stale
	    || (fabs(millivolts - new_value) >= millivolts_threshold)) {
	  millivolts = new_value;
	  refresh();
	  mqtt_publish("status/millivolts", String(millivolts, 1));
	}
	else {
	  millivolts = new_value;
	}
    })
    ELSEWHENFROM("current", "status/milliamps", {
	float new_value = payload.toFloat();
	static unsigned long last_milliamps = 0;
	bool stale = millis() > (last_milliamps + 60000);
	if (isnan(milliamps) || stale
	    || (fabs(milliamps - new_value) >= milliamps_threshold)) {
	  milliamps = new_value;
	  refresh();
	  mqtt_publish("status/milliamps", String(milliamps, 1));
	}
	else {
	  milliamps = new_value;
	}
    })
    else {
      handled = AbstractAppLeaf::mqtt_receive(type, name, topic, payload);
      if (!handled) {
	LEAF_WARN("App did not handle %s/%s => %s <= %s", type.c_str(), name.c_str(), topic.c_str(), payload.c_str());
      }
    }
    LEAF_BOOL_RETURN(handled);
  }

};

void fernvaleKeypadRead(lv_indev_drv_t *indev, lv_indev_data_t *data) {

  data->state = fernvale_last_button_is_press?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
  if (fernvale_last_button==NONE) {
    return;
  }

  bool known_key=true;
  switch (fernvale_last_button) {
  case UP:
    data->key=LV_KEY_UP;
    break;
  case DOWN:
    data->key=LV_KEY_DOWN;
    break;
  case LEFT:
    data->key=LV_KEY_LEFT;
    break;
  case RIGHT:
    data->key=LV_KEY_RIGHT;
    break;
  case CENTER:
    data->key=LV_KEY_ENTER;
    break;
#if 0
  case BUT_A:
    data->key=LV_KEY_PREV;
    break;
  case BUT_B:
    data->key=LV_KEY_ESC;
    break;
  case BUT_C:
    data->key=LV_KEY_NEXT;
    break;
#endif
  default:
    known_key=false;
  }
  if (known_key) {
    INFO("Pass keypad press %s %s to LVGL",
	   fernvale_button_names[fernvale_last_button],
	   PRESS(fernvale_last_button_is_press));
  }

  fernvale_last_button = NONE;
}



void fernvaleEncoderRead(lv_indev_drv_t *indev, lv_indev_data_t *data) {

  data->state = fernvale_last_button_is_press?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
  if (fernvale_last_button==NONE) {
    return;
  }

  bool known_key = true;
  switch (fernvale_last_button) {
  case BUT_1:
    //WARN("encoder but1 pressed (right)");
    data->key=LV_KEY_RIGHT;
    break;
  case BUT_2:
    //WARN("encoder but2 pressed (left)");
    data->key=LV_KEY_LEFT;
    break;
  case BUT_3:
    //WARN("encoder but3 pressed (enter)");
    data->key=LV_KEY_ENTER;
    break;
  default:
    known_key = false;
  }
  if (known_key) {
    INFO("Pass encoder press %s %s to LVGL",
	   fernvale_button_names[fernvale_last_button],
	   PRESS(fernvale_last_button_is_press));
  }
}

// local Variables:
// mode: C++
// c-basic-offset: 2
// End:
