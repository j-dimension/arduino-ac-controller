#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2  //Sensor DS18B20 am digitalen Pin 2
OneWire oneWire(ONE_WIRE_BUS); //
//Übergabe der OnewWire Referenz zum kommunizieren mit dem Sensor.
DallasTemperature sensors(&oneWire);
int sensorCount;

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB
};
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 178, 162);
IPAddress myDns(192, 168, 178, 162);

// initialize the library instance:
EthernetClient client;

char server[] = "smarthome";  // also change the Host line in httpRequest()
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
// not working!!!
// const unsigned long postingInterval = 45 * 1000; // delay between updates, in milliseconds

// working:
unsigned long postingInterval = 300ul * 1000ul;

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  delay(1000);

  // start serial port:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    // https://arduino.stackexchange.com/questions/41717/arduino-uno-only-works-with-monitor-serial-opened
  }

  sensors.begin(); //Starten der Kommunikation mit dem Sensor
  sensorCount = sensors.getDS18Count(); //Lesen der Anzahl der angeschlossenen Temperatursensoren.

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



  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {


    if (sensorCount == 0) {
      Serial.println("Es wurde kein Temperatursensor gefunden!");
      Serial.println("Bitte überprüfe deine Schaltung!");
    }

    //Es können mehr als 1 Temperatursensor am Datenbus angschlossen werden.
    //Anfordern der Temperaturwerte aller angeschlossenen Temperatursensoren.
    sensors.requestTemperatures();
    //Ausgabe aller Werte der angeschlossenen Temperatursensoren.
    //for (int i = 0; i < sensorCount; i++) {
      //Serial.print(i);
      Serial.print("Temperatur: ");
      float t=sensors.getTempCByIndex(0);
      printValue(t, "°C");
      //printValue(sensors.getTempFByIndex(i), "°F");
    //}


    httpRequest(t);
  }

}

void printValue(float value, String text){
  Serial.print("\t\t");
  Serial.print(value);
  Serial.println(text);
}

// this method makes a HTTP connection to the server:
void httpRequest(float temperature) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // curl -X PUT --header "Content-Type: text/plain" --header "Accept: application/json" -d "21.2" "http://smarthome:8080/rest/items/Temp_Schlafen_Raw/state"

  // if there's a successful connection:
  if (client.connect(server, 8080)) {
    String sTemp=String(temperature);
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("PUT /rest/items/Temp_Kind2_Raw/state HTTP/1.1");
    client.println("Host: smarthome:8080");
    client.println("User-Agent: arduino-ethernet");
    //client.println("Connection: close");
    client.println("Content-Type: text/plain");
    client.println("Accept: application/json");
    client.print("Content-Length: ");
    client.println(String(sTemp.length()));
    // NEED to have an empty line after content length
    client.println();
    //client.println("Connection: close");
    client.print(sTemp);
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}
