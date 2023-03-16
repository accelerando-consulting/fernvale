//
// This class implements the display logic for an environmetal sensor
//
#pragma once

#include "stacx/abstract_app.h"

//RTC_DATA_ATTR int saved_something = 0;

unsigned long app_count=0;

void ARDUINO_ISR_ATTR counterISR(void) {
  app_count++;
}

void shell_prompt(char *buf, uint8_t len) 
{
  AbstractIpLeaf *ipLeaf = (AbstractIpLeaf *)Leaf::get_leaf_by_type(leaves, "ip");
  AbstractPubsubLeaf *pubsubLeaf = (AbstractPubsubLeaf *)Leaf::get_leaf_by_type(leaves, "pubsub");

  int pos = 0;
  pos += snprintf(buf, len, "%s v%sb%d %s", device_id, FIRMWARE_VERSION, BUILD_NUMBER, comms_state_name[stacx_comms_state]);

  if (ipLeaf) {
    bool h = ipLeaf->isConnected();
    bool ph = pubsubLeaf && pubsubLeaf->isConnected();
    pos+= snprintf(buf+pos, len-pos, " IP %s/%s", HEIGHT(h), HEIGHT(ph));
  }
  pos+= snprintf(buf+pos, len-pos, "> ");
}


class FernvaleAppLeaf : public AbstractAppLeaf
{
protected: // configuration preferences, see setup() for defaults.
  bool mute=false;

  int report_interval;
  int report_min_sec;
  int report_count_interval;
  int battery_millivolts = -1;
  int refresh_msec = 200;
  int counter_pin = 27;
  

  static const uint32_t color_none   = 0x000000; // black
  static const uint32_t color_off    = 0x400000; // red
  static const uint32_t color_on    = 0x002000; // green
  static const uint32_t color_stby  = 0x301000; // orange
  static const uint32_t color_count = 0x000040; // blue
  static const uint32_t color_reset = 0x200020; // magenta
  static const uint32_t color_tuning = 0x202000; // yellow

  static const int pixel_comms = 1;
  static const int pixel_wifi = 2;
  
protected: // ephemeral state
  unsigned long last_refresh = 0;
  unsigned long last_report = 0;
  unsigned long last_report_count = 0;
  unsigned long update_limit_msec = 500;

#if USE_TFT
  TFTLeaf *screen;
#elif USE_OLED
  OledLeaf *screen;
#else
  Leaf *screen = NULL;
#endif

public:
  FernvaleAppLeaf(String name, String target)
    : AbstractAppLeaf(name,target)
    , Debuggable(name)
  {
    LEAF_ENTER(L_INFO);
    this->impersonate_backplane=true;
    // default variables or constructor argument processing goes here
    LEAF_LEAVE;
  }

  virtual void setup(void);
  virtual void start(void);
  virtual void loop(void);
  virtual void refresh(void);
  void button(int b);
  virtual void mqtt_do_subscribe();
  virtual bool mqtt_receive(String type, String name, String topic, String payload);

#ifdef USE_TFT  
protected: // LVGL screen resources

  lv_obj_t *screen_home;
  lv_style_t style_clock;
  lv_style_t style_value;

  lv_obj_t *home_clock;
  lv_obj_t *home_host;
  lv_obj_t *home_vers;

  lv_obj_t *home_count;
  lv_obj_t *home_freq;

#endif
};

void FernvaleAppLeaf::refresh(void) 
{
  char buf[32];
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(buf, sizeof(buf), "%T", &timeinfo);
    lv_label_set_text(home_clock, buf);
  }

  if (app_count > 0) {
    snprintf(buf, sizeof(buf), "%lu pl", app_count);
    lv_label_set_text(home_count, buf);
  }
}


void FernvaleAppLeaf::setup(void) {
  AbstractAppLeaf::setup();
  LEAF_ENTER(L_NOTICE);
#if USE_TFT
  screen = (TFTLeaf *)get_tap("screen");
#elif USE_OLED
  screen = (OledLeaf *)get_tap("screen");
#endif

  pinMode(counter_pin, INPUT_PULLUP);

  LEAF_LEAVE;
}


void FernvaleAppLeaf::start()
{
  AbstractAppLeaf::start();
  LEAF_WARN("Fernvale (v%sb%d)", FIRMWARE_VERSION, (int)BUILD_NUMBER);

  if (!screen) return;
    
#if USE_TFT
  LEAF_NOTICE("Setup home screen");
  screen_home = lv_obj_create(NULL);

  lv_style_init(&style_clock);
  lv_style_set_text_font(&style_clock, &lv_font_montserrat_48);

  lv_style_init(&style_value);
  lv_style_set_text_font(&style_value, &lv_font_montserrat_48);


  home_clock = lv_label_create(screen_home);
  lv_obj_add_style(home_clock, &style_clock, 0);
  lv_label_set_text(home_clock, "");
  lv_obj_align(home_clock, LV_ALIGN_TOP_MID, 0, 0);

  home_host = lv_label_create(screen_home);
  lv_label_set_text(home_host, device_id);
  lv_obj_align(home_host, LV_ALIGN_BOTTOM_LEFT, 0, 0);

  home_vers = lv_label_create(screen_home);
  lv_label_set_text(home_vers, FIRMWARE_VERSION);
  lv_obj_align(home_vers, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

  home_count = lv_label_create(screen_home);
  lv_obj_align(home_count, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_add_style(home_count, &style_value, 0);
  lv_label_set_text(home_count, "0 pulses");

  home_freq = lv_label_create(screen_home);
  lv_obj_align(home_freq, LV_ALIGN_TOP_MID, 0, 120);
  lv_obj_add_style(home_freq, &style_value, 0);
  lv_label_set_text(home_freq, "0 Hz");

  lv_scr_load(screen_home);

#endif

  attachInterrupt(counter_pin, counterISR, RISING);
}

void FernvaleAppLeaf::loop()
{
  AbstractAppLeaf::loop();
  unsigned long now=millis();
  
  if (now > (last_refresh+refresh_msec)) {
    refresh();
    last_refresh=now;
  }
}



void FernvaleAppLeaf::button(int b)
{
  LEAF_NOTICE("button %d press", b);
  //update();
}


void FernvaleAppLeaf::mqtt_do_subscribe() {
  AbstractAppLeaf::mqtt_do_subscribe();
}

bool FernvaleAppLeaf::mqtt_receive(String type, String name, String topic, String payload)
{
  LEAF_ENTER(L_DEBUG);
  bool handled = AbstractAppLeaf::mqtt_receive(type, name, topic, payload);

  LEAF_NOTICE("RECV %s/%s => [%s <= %s]", type.c_str(), name.c_str(), topic.c_str(), payload.c_str());

  WHEN("_ip_connect", {
      //setPixel(pixel_wifi, color_reset);
      //message("pixel", "set/value", "20");
      //message("pixel", "set/hue", "4000");
    })
    WHEN("_ip_disconnect", {
	//setPixel(pixel_wifi, color_off);
      })
    WHEN("_pubsub_connect", {
	//setPixel(pixel_wifi, color_on);
      })
    WHEN("_pubsub_disconnect", {
	//setPixel(pixel_wifi, color_reset);
      })
    WHENFROM("btn1", "event/press", {
	button(1);
      })
    WHENFROM("btn2", "event/press", {
	button(2);
      })
    ELSEWHENFROM("btn3", "event/press", {
	button(3);
      })

    LEAF_LEAVE;
  RETURN(handled);
}

// local Variables:
// mode: C++
// c-basic-offset: 2
// End:
