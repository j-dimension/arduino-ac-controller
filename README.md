# arduino-ac-controller
An Arduino based AC controller for three rooms

* Consists of four Arduino Uno R3 with Ethernet shields
* 3x ac_sensor as temperature sensors to place in each room - sends temperature to an OpenHAB installation
* OpenHAB compares temperatures and creates a bit mask that indicates which room needs cooling
* 1x ac_controller that pulls the bitmask and switches 5 relais: 3 valves for the rooms, 1 valve for a pump and 1 valve for a fan / AC system
* code has "intelligence" to wait for the valves to be fully open before the AC is turned on
* Ethernet uses DHCP instead of fixed IP