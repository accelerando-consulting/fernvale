// Configuration defaults for Stacx
//
// Preferences can be set either by editing here, using the persistent
// storage mechanism, or via the captive-portal setup process.
//

#define BUILD_NUMBER 4

#define FIRMWARE_VERSION "0.1"

#define DEBUG_SYSLOG 1
#define DEBUG_FILES true
#define DEBUG_COLOR true
#define DEBUG_THREAD 1
#define HELLO_PIN 22
#define USE_OTA 1
#define USE_TFT 1
#define USE_WDT 1
#define USE_TELNETD 1

#define NETWORK_RECONNECT_SECONDS 60
#define PUBSUB_RECONNECT_SECONDS 90

#define APP_TOPIC "fernvale"
#define PUBSUB_HOST_DEFAULT "iotlab.accelerando.io"

#define DEVICE_ID "fernvale"
#define DEVICE_ID_APPEND_MAC 1
#define USE_WILDCARD_TOPIC true

#define IP_WIFI_USE_AP false
#define IP_WIFI_AP_0_SSID "Provision"
#define IP_WIFI_AP_0_PASS "Provision"
#define IP_ENABLE_OTA 1

#define OTA_PASSWORD "changeme"
#define UPDATE_URL "http://firmware.accelerando.io/fernavle/fernvale.bin"

#define TIMEZONE_HOURS 10
#define TIMEZONE_MINUTES 0



