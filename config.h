// Configuration defaults for Stacx
//
// Preferences can be set either by editing here, using the persistent
// storage mechanism, or via the captive-portal setup process.
//

#define BUILD_NUMBER 67

#define FIRMWARE_VERSION "0.1"

#define EARLY_SERIAL 1
#define STACX_LEAF_VECTOR 1
#define DEBUG_LEVEL 2
#define DEBUG_FLUSH true
#define MAX_DEBUG_LEVEL 3
#define DEBUG_SYSLOG 0
#define DEBUG_FILES true
#define DEBUG_COLOR true
#define DEBUG_THREAD 1
#define IP_WIFI_OWN_LOOP true

#ifdef SCREEN_T14
#define HELLO_PIN 22
#endif

#define LVGL_BUFFER_ROWS 4
#define LVGL_BUFFER_FACTOR 1/10


#define USE_OTA 1
#define USE_TFT 1
#define USE_WDT 0
#define USE_TELNETD 1
#define APP_USE_WIFI true
#define APP_USE_LTE false

#define NETWORK_RECONNECT_SECONDS 60
#define PUBSUB_RECONNECT_SECONDS 90

#define APP_TOPIC "fernvale"
#define PUBSUB_HOST_DEFAULT "iotlab.accelerando.io"

#undef DEVICE_ID
#define DEVICE_ID "fernvale"

#define DEVICE_ID_APPEND_MAC 1
#define USE_WILDCARD_TOPIC true

#define IP_WIFI_USE_AP false
#define IP_WIFI_AP_0_SSID "Low Energy Bogon Research Lab"
#define IP_WIFI_AP_0_PASS "I Like Chocolate"
#define IP_WIFI_AP_1_SSID "Accelerando"
#define IP_WIFI_AP_1_PASS "sailboat-rabbit-banana"
#define IP_ENABLE_OTA 1

#define OTA_PASSWORD "changeme"
#define UPDATE_URL "http://firmware.accelerando.io/fernvale/fernvale.bin"

#define TIMEZONE_HOURS 10
#define TIMEZONE_MINUTES 0



