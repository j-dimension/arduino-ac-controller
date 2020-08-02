#include <SPI.h>
#include <Ethernet.h>

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 178, 170);
IPAddress myDns(192, 168, 178, 170);

// initialize the library instance:
EthernetClient client;

char server[] = "smarthome";  // also change the Host line in httpRequest()
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds

// not working!!!
// const unsigned long postingInterval = 45 * 1000; // delay between updates, in milliseconds

// working:
unsigned long postingInterval = 600ul * 1000ul;

const int RELAIS1 = 2;
const int RELAIS_K1 = 2;
const int RELAIS2 = 3;
const int RELAIS_K2 = 3;
// arduino pin 4 is not available when using the ethernet shield
const int RELAIS3 = 5;
const int RELAIS_SZ = 5;
const int RELAIS4 = 6;
const int RELAIS_PUMP = 6;
const int RELAIS5 = 7;
const int RELAIS_AC = 7;

String activeRoomsK1K2SZ="";


void setup() {
  pinMode(RELAIS1,          OUTPUT);
  pinMode(RELAIS2,          OUTPUT);
  pinMode(RELAIS3,          OUTPUT);
  pinMode(RELAIS4,          OUTPUT);
  pinMode(RELAIS5,          OUTPUT);

  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // initially, switch off all relays
  digitalWrite(RELAIS1, HIGH);
  digitalWrite(RELAIS2, HIGH);
  digitalWrite(RELAIS3, HIGH);
  digitalWrite(RELAIS4, HIGH);
  digitalWrite(RELAIS5, HIGH);

  delay(1000);

  // start serial port:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    // https://arduino.stackexchange.com/questions/41717/arduino-uno-only-works-with-monitor-serial-opened
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop() {
  //digitalWrite(RELAIS1, LOW);
  //digitalWrite(RELAIS2, HIGH);
  //digitalWrite(RELAIS3, HIGH);
  //digitalWrite(RELAIS4, HIGH);
  //digitalWrite(RELAIS5, HIGH);
  //delay(1000);



  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  
  if (client.available()) {
    char c = client.read();
    activeRoomsK1K2SZ=activeRoomsK1K2SZ+c;
    //Serial.write(c);
  }
  
  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    //Serial.println("state: " + activeRoomsK1K2SZ);
    if(activeRoomsK1K2SZ.length()>3) {
      String switchState=activeRoomsK1K2SZ.substring(activeRoomsK1K2SZ.length()-3);
      Serial.println("K1K2SZ: " + switchState);
      char k1=switchState.charAt(0);
      char k2=switchState.charAt(1);
      char sz=switchState.charAt(2);
      if(k1 != '1' && k1 !='0') {
        Serial.print("Char ");
        Serial.print(k1);
        Serial.println(" is invalid for switch state K1");
      }
      if(k2 != '1' && k2 !='0') {
        Serial.print("Char ");
        Serial.print(k2);
        Serial.println(" is invalid for switch state K2");
      }
      if(sz != '1' && sz !='0') {
        Serial.print("Char ");
        Serial.print(sz);
        Serial.println(" is invalid for switch state SZ");
      }

      int cooledRooms=0;
      if(k1=='1') {
        Serial.println("Cooling K1 on");
        digitalWrite(RELAIS_K1, LOW);
        cooledRooms = cooledRooms +1;
      }
      if(k2=='1') {
        Serial.println("Cooling K2 on");
        digitalWrite(RELAIS_K2, LOW);
        cooledRooms = cooledRooms +1;
      }
      if(sz=='1') {
        Serial.println("Cooling SZ on");
        digitalWrite(RELAIS_SZ, LOW);
        cooledRooms = cooledRooms +1;
      }

      if(cooledRooms > 0) {
        // warten bis Klappen aufgefahren sind
        Serial.println("Cooling pump on");
        delay(40000);
        digitalWrite(RELAIS_PUMP, LOW);
        digitalWrite(RELAIS_AC, LOW);
      } else {
        Serial.println("Cooling pump off");
        digitalWrite(RELAIS_PUMP, HIGH);
        digitalWrite(RELAIS_AC, HIGH);
      }

      if(k1=='0') {
        Serial.println("Cooling K1 off");
        digitalWrite(RELAIS_K1, HIGH);
      }
      if(k2=='0') {
        Serial.println("Cooling K2 off");
        digitalWrite(RELAIS_K2, HIGH);
      }
      if(sz=='0') {
        Serial.println("Cooling SZ off");
        digitalWrite(RELAIS_SZ, HIGH);
      }
      
    }
    httpRequest();
  }


}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // http://smarthome:8080/rest/items/Cooling_ActiveRooms_K1K2SZ/state

  // if there's a successful connection:
  if (client.connect(server, 8080)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /rest/items/Cooling_ActiveRooms_K1K2SZ/state HTTP/1.1");
    client.println("Host: smarthome:8080");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}
