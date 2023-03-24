//
// This class implements the display logic for an environmetal sensor
//
#pragma once

#include "stacx/abstract_app.h"
#include "ESP32Tone.h"

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
  int pwm_pin = 13;
  int pwm_freq = 3000;

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
  bool mode_hz = true; // hz or ppm
  bool counter_run = true;
  bool pwm_run = true;
  unsigned long last_interval_time = 0;
  unsigned long last_interval_count = 0;

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
    this->impersonate_backplane=true;
    // default variables or constructor argument processing goes here
  }

  virtual void setup(void);
  virtual void start(void);
  virtual void loop(void);
  virtual void refresh(void);
  void button(int b);
  virtual bool wants_topic(String type, String name, String topic);
  virtual bool mqtt_receive(String type, String name, String topic, String payload, bool direct=false);
  virtual bool commandHandler(String type, String name, String topic, String payload);

#ifdef USE_TFT
protected: // LVGL screen resources

  lv_obj_t *screen_home;
  lv_style_t style_clock;
  lv_style_t style_count;
  lv_style_t style_freq;
  lv_style_t style_btn;

  lv_obj_t *home_clock;
  lv_obj_t *home_host;
  lv_obj_t *home_vers;
  lv_obj_t *home_ip;
  lv_obj_t *home_batt;

  lv_obj_t *home_count;
  lv_obj_t *home_countfreq;
  lv_obj_t *home_pwmfreq;

  lv_obj_t *home_btn1;
  lv_obj_t *home_btn2;
  lv_obj_t *home_btn3;
  lv_obj_t *home_btn4;

#endif
};


void FernvaleAppLeaf::setup(void) {
  AbstractAppLeaf::setup();
  LEAF_ENTER(L_NOTICE);
#if USE_TFT
  screen = (TFTLeaf *)get_tap("screen");
#elif USE_OLED
  screen = (OledLeaf *)get_tap("screen");
#endif

  registerLeafIntValue("counter_pin", &pwm_pin);
  registerLeafIntValue("pwm_pin", &pwm_pin);
  registerLeafIntValue("pwm_freq", &pwm_freq);
  
  pinMode(counter_pin, INPUT_PULLUP);
  pinMode(pwm_pin, OUTPUT);

  registerCommand(HERE, "freq", "Set pwm frequency on pin 13");

  LEAF_LEAVE;
}

void FernvaleAppLeaf::start()
{
  AbstractAppLeaf::start();
  LEAF_WARN("Fernvale (v%sb%d)", FIRMWARE_VERSION, (int)BUILD_NUMBER);


#if USE_TFT
  if (screen) {
    LEAF_NOTICE("Setup home screen");
    screen_home = lv_obj_create(NULL);

    lv_style_init(&style_clock);
    lv_style_set_text_font(&style_clock, &lv_font_montserrat_32);
    lv_style_set_text_color(&style_clock, lv_palette_main(LV_PALETTE_BLUE_GREY));

    lv_style_init(&style_count);
    lv_style_set_text_font(&style_count, &lv_font_unscii_16/*montserrat_48*/);

    lv_style_init(&style_freq);
    lv_style_set_text_font(&style_freq, &lv_font_unscii_16/*montserrat_32*/);

    lv_style_init(&style_btn);
    lv_style_set_text_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));

    home_clock = lv_label_create(screen_home);
    lv_obj_add_style(home_clock, &style_clock, 0);
    lv_label_set_text(home_clock, "");
    lv_obj_align(home_clock, LV_ALIGN_TOP_MID, 5, 0);

    home_host = lv_label_create(screen_home);
    lv_label_set_text(home_host, device_id);
    lv_obj_align(home_host, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    home_vers = lv_label_create(screen_home);
    lv_label_set_text_fmt(home_vers, "v%s b%d", FIRMWARE_VERSION, BUILD_NUMBER);
    lv_obj_align(home_vers, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

    home_ip = lv_label_create(screen_home);
    lv_label_set_text(home_ip, "OFFLINE");
    lv_obj_align(home_ip, LV_ALIGN_BOTTOM_MID, 20, -5);

    home_batt = lv_label_create(screen_home);
    lv_label_set_text(home_batt, "");
    lv_obj_align(home_batt, LV_ALIGN_TOP_RIGHT, -5, 0);

    home_count = lv_label_create(screen_home);
    lv_obj_align(home_count, LV_ALIGN_TOP_LEFT, 55, 77);
    lv_obj_add_style(home_count, &style_count, 0);
    lv_label_set_text(home_count, "");

    home_countfreq = lv_label_create(screen_home);
    lv_obj_align(home_countfreq, LV_ALIGN_TOP_RIGHT, -10, 77);
    lv_obj_add_style(home_countfreq, &style_freq, 0);
    lv_label_set_text(home_countfreq, "");

    home_pwmfreq = lv_label_create(screen_home);
    lv_obj_align(home_pwmfreq, LV_ALIGN_TOP_LEFT, 55, 130);
    lv_obj_add_style(home_pwmfreq, &style_freq, 0);
    if (pwm_run) {
      lv_label_set_text_fmt(home_pwmfreq, "%dHz", pwm_freq);
    }
    else {
      lv_label_set_text(home_pwmfreq, "");
    }

    home_btn1 = lv_label_create(screen_home);
    lv_obj_add_style(home_btn1, &style_btn, 0);
    lv_obj_align(home_btn1, LV_ALIGN_TOP_LEFT, 0, 30);
    lv_label_set_text(home_btn1, "<CLR");

    home_btn2 = lv_label_create(screen_home);
    lv_obj_add_style(home_btn2, &style_btn, 0);
    lv_obj_align(home_btn2, LV_ALIGN_TOP_LEFT, 0, 80);
    lv_label_set_text(home_btn2, counter_run?"<CNT":"</CNT");

    home_btn3 = lv_label_create(screen_home);
    lv_obj_add_style(home_btn3, &style_btn, 0);
    lv_obj_align(home_btn3, LV_ALIGN_TOP_LEFT, 0, 130);
    lv_label_set_text(home_btn3, pwm_run?"<PWM":"</PWM");

    home_btn4 = lv_label_create(screen_home);
    lv_obj_add_style(home_btn4, &style_btn, 0);
    lv_obj_align(home_btn4, LV_ALIGN_TOP_LEFT, 0, 180);
    lv_label_set_text(home_btn4, "<RST");

    lv_scr_load(screen_home);
  }
  lv_timer_handler();
  
#endif

  attachInterrupt(counter_pin, counterISR, FALLING);
  if (pwm_freq > 0) {
    tone(pwm_pin, pwm_freq);
  }
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

bool FernvaleAppLeaf::commandHandler(String type, String name, String topic, String payload) {
  LEAF_HANDLER(L_INFO);

    WHEN("freq", {
      pwm_freq = payload.toInt();
      if (pwm_freq == 0) {
	ALERT("PWM off");
	lv_label_set_text(home_pwmfreq, "OFF");
	noTone(pwm_pin);
      }
      else {
	ALERT("PWM freq %d", pwm_freq);
	lv_label_set_text_fmt(home_pwmfreq, "%d Hz", pwm_freq);
	tone(pwm_pin, pwm_freq);
      }
      })
    else {
      handled = AbstractAppLeaf::commandHandler(type, name, topic, payload);
    }
    
    LEAF_HANDLER_END;
}

void FernvaleAppLeaf::refresh(void)
{
  char buf[32];
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {
    strftime(buf, sizeof(buf), "%T", &timeinfo);
    lv_label_set_text(home_clock, buf);
  }
  if (battery_millivolts >= 0) {
    snprintf(buf, sizeof(buf), "%.2fV", battery_millivolts/1000.0);
    lv_label_set_text(home_batt, buf);
  }

  if (!counter_run && !pwm_run) return;

  unsigned long now = millis();
  unsigned long count = app_count;

  lv_label_set_text_fmt(home_count, "%lu", count);



  if (last_interval_time == 0) {
    last_interval_time = now;
    last_interval_count = count;
  }

  if (mode_hz) {
    if (now > (last_interval_time+1000)) {
      unsigned long counts_in_1sec = count - last_interval_count;
      unsigned long elapsed_msec = now - last_interval_time;
      float hz = 1000*(float)counts_in_1sec / (float)elapsed_msec;
      LEAF_DEBUG("Hz refresh counts=%lu elapsed=%lu hz=%f",
		 counts_in_1sec, elapsed_msec, hz);
      if (hz > 1000) {
	snprintf(buf, sizeof(buf), "%dHz", (int)hz);
      }
      else if (hz > 100) {
	snprintf(buf, sizeof(buf), "%.1fHz", hz);
      }
      else {
	snprintf(buf, sizeof(buf), "%.2fHz", hz);
      }
      lv_label_set_text(home_countfreq, buf);
      last_interval_time = now;
      last_interval_count = count;
    }
  }
  else {
    if (now > (last_interval_time+5000)) {
      unsigned long counts_in_5sec = count - last_interval_count;
      float elapsed_sec = (now - last_interval_time)/1000;
      float ppm = 12.0 * (float)counts_in_5sec / elapsed_sec;
      LEAF_DEBUG("ppm refresh counts=%lu elapsed=%.3f ppm=%f",
		counts_in_5sec, elapsed_sec, ppm);
      if (ppm > 1000) {
	snprintf(buf, sizeof(buf), "%dppm", (int)ppm);
      }
      else if (ppm > 100) {
	snprintf(buf, sizeof(buf), "%.1fppm", ppm);
      }
      else {
	snprintf(buf, sizeof(buf), "%.2fppm", ppm);
      }

      lv_label_set_text(home_countfreq, buf);
      last_interval_time=now;
      last_interval_count=count;
    }
  }

}


void FernvaleAppLeaf::button(int b)
{
  LEAF_NOTICE("button %d press", b);
  //update();

  switch (b) {
  case 1:
    app_count=0;
    last_interval_time=0;
    last_interval_count=0;
    lv_label_set_text(home_count, "");
    lv_label_set_text(home_countfreq, "");
    break;
  case 2:
    counter_run = !counter_run;
    if (counter_run) {
      attachInterrupt(counter_pin, counterISR, RISING);
      tone(pwm_pin, 3000);
      lv_label_set_text(home_btn2, "<CNT");
    }
    else {
      detachInterrupt(counter_pin);
      noTone(pwm_pin);
      lv_label_set_text(home_btn2, "</CNT");
    }
    break;
  case 3:
    pwm_run = !pwm_run;
    if (pwm_run) {
      lv_label_set_text(home_btn3, "<PWM");
      tone(pwm_pin, pwm_freq);
    }
    else {
      lv_label_set_text(home_btn3, "</PWM");
      noTone(pwm_pin);
    }
    last_interval_time=0;
    last_interval_count=0;
    lv_label_set_text(home_countfreq, "");
    break;
  }
  refresh();
}

bool FernvaleAppLeaf::wants_topic(String type, String name, String topic)
{
  if (topic.startsWith("_")) return true;
  return AbstractAppLeaf::wants_topic(type,name,topic);
}

bool FernvaleAppLeaf::mqtt_receive(String type, String name, String topic, String payload, bool direct)
{
  LEAF_ENTER(L_DEBUG);
  bool handled = false;

  LEAF_NOTICE("RECV %s/%s => [%s <= %s]", type.c_str(), name.c_str(), topic.c_str(), payload.c_str());

  WHEN("_ip_connect", {
      //setPixel(pixel_wifi, color_reset);
      //message("pixel", "set/value", "20");
      //message("pixel", "set/hue", "4000");
      lv_label_set_text(home_ip, ipLeaf->ipAddressString().c_str());
  })
  WHEN("_ip_disconnect", {
      //setPixel(pixel_wifi, color_off);
      lv_label_set_text(home_ip, "Offline");
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
  ELSEWHENFROM("battlvl", "status/battery", {
      battery_millivolts=payload.toInt();
  })
  else {
    handled = AbstractAppLeaf::mqtt_receive(type, name, topic, payload);
  }
  LEAF_BOOL_RETURN(handled);
}

// local Variables:
// mode: C++
// c-basic-offset: 2
// End:
