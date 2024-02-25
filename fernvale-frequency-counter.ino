#include "config.h"


#include "stacx.h"

#include "leaf_fs.h"
#include "leaf_fs_preferences.h"
#include "leaf_power.h"
#include "leaf_battery_level.h"
#include "leaf_shell.h"
#include "leaf_ip_esp.h"
#include "leaf_pubsub_mqtt_esp.h"
#include "leaf_ip_null.h"
#include "leaf_ina219.h"
//#include "leaf_ina3221.h"
#include "leaf_pubsub_null.h"
#include "leaf_wire.h"
#include "leaf_button.h"
#include "leaf_joyofclicks.h"
#include "leaf_lvgl_etft.h"
#include "leaf_tone.h"
#include "leaf_sdcard.h"
#include "leaf_scd40.h"
#include "abstract_app.h"

#include "app_fernvale.h"

SET_LOOP_TASK_STACK_SIZE(16*1024); // 16kB

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

#ifdef SETUP_HEAP_CHECK
#define HEAPCHECK stacx_heap_check(HERE)
#else
#define HEAPCHECK 
#endif

#define LEAF_NEW(class,args) WARN("CREATE "#class); l=new class args;leaves.push_back(l); HEAPCHECK ; l

void leaf_allocate() 
{
  Leaf *l;
  LEAF_NEW(FSLeaf, ("fs", FS_DEFAULT, FS_ALLOW_FORMAT));
  LEAF_NEW(FSPreferencesLeaf, ("prefs"));
  //LEAF_NEW(PowerLeaf,("power"));
  LEAF_NEW(ShellLeaf,("shell", "Fernvale", shell_prompt));
  LEAF_NEW(LVGLeTFTLeaf,("screen", 3));

  // Wifi comms 
  LEAF_NEW(IpEspLeaf,("wifi"));
  LEAF_NEW(PubsubEspAsyncMQTTLeaf,(
		     "wifimqtt","wifi,fs",
		     PUBSUB_SSL_DISABLE,
		     PUBSUB_DEVICE_TOPIC_DISABLE
	     ));
  LEAF_NEW(IpNullLeaf, ("nullip", "fs"))->setTrace(L_WARN)->inhibit();
  LEAF_NEW(IpNullLeaf,("nullip", "fs"))->setTrace(L_WARN)->inhibit();
  LEAF_NEW(PubsubNullLeaf,("nullmqtt", "ip"))->setTrace(L_WARN)->inhibit();

#if SCREEN_T14
  LEAF_NEW(WireBusLeaf,("wire", /*sda=*/21, /*scl=*/22));
  LEAF_NEW(ToneLeaf,("speaker",          LEAF_PIN(27), 2500, 1000, false));
  LEAF_NEW(BatteryLevelLeaf,("battlvl",   LEAF_PIN(34),100000,100000,12,3))->setMute();
  LEAF_NEW(ButtonLeaf,("btn1", LEAF_PIN(38), LOW, false))->setMute();
  LEAF_NEW(ButtonLeaf,("btn2", LEAF_PIN(37), LOW, false))->setMute();
  LEAF_NEW(ButtonLeaf,("btn3", LEAF_PIN(39), LOW, false))->setMute();
  LEAF_NEW(INA219Leaf,("current",NO_TAPS,0,100,1000))->setMute();
  LEAF_NEW(JoyOfClicksLeaf,("joc", 0x21))->setMute();
#elif SCREEN_CYB_2432S028
  LEAF_NEW(WireBusLeaf, ("wire", /*sda=*/22, /*scl=*/27));
  LEAF_NEW(ToneLeaf, ("speaker",          LEAF_PIN(26), 2500, 1000, false));
#else
	#error I dont know who I am
#endif
  //LEAF_NEW(SDCardLeaf, ("sd", &SD, /*ss=*/13, /*sck=*/14, /*mosi=*/15, /*miso=*/12));
  //(LEAF_NEW(INA3221Leaf, ("current3",NO_TAPS,0,100,1000))->setMute();
  //LEAF_NEW(Scd40Leaf, ("co2"));

  LEAF_NEW(FernvaleAppLeaf, ("app", "fs,screen,wifi,wifimqtt,btn1,btn2,btn3,battlvl,joc,current,co2"))->setTrace(L_NOTICE);
}

  
    
  

