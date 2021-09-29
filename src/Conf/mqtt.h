/**
 * @Date:   2018-06-08T15:11:04+02:00
 * @Last modified time: 2018-06-08T15:17:47+02:00
 */

 /* topics */
 #define CAPT_TOPIC    "box/capt"
 #define LIGHT_TOPIC     "box/trigger" /* 1=on, 0=off */

 /* this is the IP of PC/raspberry where MQTT Server is installed
 on Wins use "ipconfig"
 on Linux use "ifconfig" to get its IP address */
 const char* mqtt_server = "192.168.1.11";
