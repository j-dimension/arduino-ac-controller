#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2  //Sensor DS18B20 am digitalen Pin 2
OneWire oneWire(ONE_WIRE_BUS); //
//Übergabe der OnewWire Referenz zum kommunizieren mit dem Sensor.
DallasTemperature sensors(&oneWire);
int sensorCount;
void setup(void) { 
 Serial.begin(9600); //Starten der seriellen Kommunikation mit 9600 baud
 Serial.println("Temperatursensor - DS18B20"); 
 sensors.begin(); //Starten der Kommunikation mit dem Sensor
 sensorCount = sensors.getDS18Count(); //Lesen der Anzahl der angeschlossenen Temperatursensoren.
} 
void loop(void){ 
 if(sensorCount ==0){
   Serial.println("Es wurde kein Temperatursensor gefunden!");
   Serial.println("Bitte überprüfe deine Schaltung!");
 }
 //Es können mehr als 1 Temperatursensor am Datenbus angschlossen werden.
 //Anfordern der Temperaturwerte aller angeschlossenen Temperatursensoren.
 sensors.requestTemperatures(); 
 //Ausgabe aller Werte der angeschlossenen Temperatursensoren.
 for(int i=0;i<sensorCount;i++){
  Serial.print(i); 
  Serial.println(". Temperatur :"); 
  printValue(sensors.getTempCByIndex(i), "°C");
  printValue(sensors.getTempFByIndex(i), "°F");
 }
 
 delay(1000); //Pause von 1 Sekunde.
} 
void printValue(float value, String text){
  Serial.print("\t\t");
  Serial.print(value);
  Serial.println(text);
}
