#include "config.h"

#include "stacx/stacx.h"

#include "stacx/leaf_fs.h"
#include "stacx/leaf_fs_preferences.h"
#include "stacx/leaf_power.h"
#include "stacx/leaf_battery_level.h"
#include "stacx/leaf_shell.h"
#include "stacx/leaf_ip_esp.h"
#include "stacx/leaf_pubsub_mqtt_esp.h"
#include "stacx/leaf_wire.h"
#include "stacx/leaf_button.h"
//#include "stacx/leaf_tft.h"
#include "libraries/lv_conf.h"
#include "stacx/leaf_lvgl.h"
#include "stacx/leaf_tone.h"
#include "stacx/leaf_sdcard.h"

#include "app_fernvale.h"

SET_LOOP_TASK_STACK_SIZE(16*1024); // 16kB


Leaf *leaves[] = {
        new FSLeaf("fs", FS_DEFAULT, FS_ALLOW_FORMAT),
	new FSPreferencesLeaf("prefs"),
	new PowerLeaf("power"),
	new ShellLeaf("shell", "Fernvale", shell_prompt),

	// Wifi comms 
	new IpEspLeaf("wifi", "fs"),
	new PubsubEspAsyncMQTTLeaf(
	  "wifimqtt","wifi,fs",
	  PUBSUB_SSL_DISABLE,
	  PUBSUB_DEVICE_TOPIC_DISABLE),

	new WireBusLeaf("wire", /*sda=*/21, /*scl=*/22),
	//new SDCardLeaf("sd", &SD, /*ss=*/13, /*sck=*/14, /*mosi=*/15, /*miso=*/12),
	(new BatteryLevelLeaf("battlvl",   LEAF_PIN(34),100000,100000,12,3))->setMute(),
	new ToneLeaf("speaker",          LEAF_PIN(27), 2500, 1000, false),

	(new ButtonLeaf("btn1", LEAF_PIN(38), LOW, false))->setMute(),
	(new ButtonLeaf("btn2", LEAF_PIN(37), LOW, false))->setMute(),
	(new ButtonLeaf("btn3", LEAF_PIN(39), LOW, false))->setMute(),
	new LVGLLeaf("screen", 3),
	new FernvaleAppLeaf("app", "fs,screen,wifi,wifimqtt,btn1,btn2,btn3,battlvl"),
	//new SDCardLeaf("sdcard"),
	NULL
};

