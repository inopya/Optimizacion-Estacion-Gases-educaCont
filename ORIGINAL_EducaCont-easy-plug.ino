/*  ------____________________ Proyecto EducaCont ____________________------ 
 *                   Estacion de Medida de Calidad del Aire
 *                   --------------------------------------
 *  Innova Didactic --> http://shop.innovadidactic.com/index.php
 *  Arduinoblocks --> http://www.arduinoblocks.com/
 *  -------------------------------------------------------------------------
 *  Elementos mínimos que debe incorporar el proyecto  
 *    - RTC I2C DS3231 
 *    - Módulo adaptador MicroSD 
 *    - Sensor de temperatura y humedad DHTxx, preferiblemente DHT22
 *    - Sensor MiCS-4514 para medida de NO2 y CO
 *    - Sensor detector de partículas de polvo en el aire KS0196 PM2.5 Shield
 *    
 *  En la tarjeta micro SD debe existir el fichero SD.csv
 */
// Librerias ------------------------------------------------------------
// para el RTC I2C
#include <DS3231.h>
// Inicializa el DS3231 usando el interface hardware
DS3231  rtc(SDA, SCL);
// para el modulo microSD
#include <SPI.h>     
#include <SD.h>    
// para el sensor DHT22
#include "DHT.h"
#define DHTTYPE DHT22   // DHT 22 
const int DHTPin = 9;     // Lo conectamos al pin digital 9 
DHT dht(DHTPin, DHTTYPE); //Creamos el objeto dht con los datos definidos
// para el sensor MICS-4514 NO2 y CO
#define PRE_PIN   8    // precalentamiento
#define VNOX_PIN  A0   // pin datos de NO2
#define VRED_PIN  A1   // pin datos de CO
#define PRE_HEAT_SECONDS 10   // tiempo de precalentamiento
// Para usar el modo sleep
#include <SnoozeLib.h>  
// Zona de declaracion de variables --------------------------------------
// para la SD
const int chipSelect = 10;   // pin de seleccion microSD
String datos;               // variable datos
// para el sensor MICS-4514
float vnox_value = 0;     // lectura datoa de NO2
float vred_value = 0;     // lectura datoa de CO
float Rsnox = 0; // resistencia del sensor
float RsCO = 0; // resistencia del sensor
float ppbNO2 = 0; //ppb de NO2
float ppmCO = 0; //ppm de CO
// para el sensor de particulas KS0196 PM2.5
int pinMedida = 6; // Sensor conectado a pin analógico A6
int ledPower = 2;   // El LED va conectado al pin digital 2
int tiempoMuestreo = 280;
int deltaTime = 40;
int sleepTime = 9680;
float Vmed = 0;
float Vcal = 0; 
float densidadPolvo = 0;
float PM02_5 = 0;
int discriminar = 0; // se usa para discriminar las dos primeras tomas de datos
int iteraciones = 0; // para controlar el número de medidas antes de pasar a modo sleep               
void setup()
{
  // Configuracion de la conexión serie
  Serial.begin(115200);
Serial.println("===========================================================");
Serial.println("--------------------- Configuraciones ---------------------");
Serial.println("===========================================================");
  // Inicializa el objeto rtc
  rtc.begin();
  //----- Inicializacion Memoria micoSD ----------------
  Serial.println("Inicializando la tarjeta micro SD ...");
  if (!SD.begin(chipSelect))
  {
    Serial.println("Error al iniciar la micro SD");
    return;
  }
  Serial.println("Tarjeta micro SD iniciada correctamente");
//------------------ Inicializacion SENSOR DHTxx ---------------------------  
  dht.begin(); // Inicializamos el objeto dht
  Serial.println("DHTxx inicializado correctamente...");
//----------------- Inicializacion SENSOR MICS-4514 -------------------------
  pinMode(PRE_PIN, OUTPUT);
  Serial.println("Precalentando el sensor MiCS-4514 durante 10s ...");
  // Espera para precalentamiento durante 10 segundos
  digitalWrite(PRE_PIN, 1);
  delay(PRE_HEAT_SECONDS * 1000);
  digitalWrite(PRE_PIN, 0);
  Serial.println("Sensor MiCS-4514 listo ...");
// configuramos pin del sensor de particulas
  pinMode(ledPower, OUTPUT); // Pin digital 2 como salida  
  Serial.println("Sensor de particulas KS0196 PM2.5 configurado ...");

Serial.println("===========================================================");
Serial.println("-------------------------- DATOS --------------------------");
Serial.println("===========================================================");
}
 
void loop()
{
//----------------- Sensor de Temperatura y Humedad Relativa ----------------
// Leer la temperatura o la humedad relativa toma alrededor de 250 ms
   float h = dht.readHumidity(); 
   float t = dht.readTemperature();
   if (isnan(h) || isnan(t)) {
      Serial.println("Error al obtener datos del sensor DHTxx");
      return;
   } 
//------------------------ Lectura NO2 y CO ---------------------------
  vnox_value = analogRead(VNOX_PIN)/409.2; // lee y convierte la lectura analogica a tension
  Rsnox = 22000/((5/vnox_value) - 1); // obtiene la resistencia del sensor a partir de 5V y una carga de 22 kOhm
  ppbNO2 = (.000008*Rsnox - .0194)*1000; // convierte Rsnox a concentration de NO2 en ppb    
  vred_value = analogRead(VRED_PIN)/409.2; // lee y convierte la lectura analogica a tension
  RsCO = 100000/((5/vred_value) - 1); // obtiene la resistencia del sensor a partir de 5V y una carga de 100 kOhm
  ppmCO = 911.19*pow(2.71828,(-8.577*RsCO/100000)); //convierte RsCO a concentracion de CO en ppm 
   
//-------------------------- Lectura PM2.5 ----------------------------
    digitalWrite(ledPower, LOW); // Activamos el LED IR
    delayMicroseconds(tiempoMuestreo); // Retardo de 280 us antes de medir
    Vmed = analogRead(pinMedida); // Lectura del pin analógico A2
    delayMicroseconds(deltaTime); // Retardo de 40 us
    digitalWrite(ledPower,HIGH); // Desactivamos el LED IR 
    delayMicroseconds(sleepTime); // Retardo de 9680 us antes de otra lectura
    // el retardo total de ser (tiempoMuestreo + deltaTime + sleepTime = 280 + 40 + 9680 = 10000us) de 10ms
    Vcal = Vmed * (5.0 / 1024.0); // Mapeo de 1023 valores enteros a 5V 
    densidadPolvo = float(0.172 * Vcal - 0.0999); // Ecuación lineal
    PM02_5 = float((Vmed/1024)-0.0356)*120000*0.035; // Ecuación lineal para PM2.5

 //Preparacion de los datos para escribir en el fichero
  datos = String (rtc.getDateStr()) + "," + String(rtc.getTimeStr())+     //reloj
  "," + String(h) + "," + String(t) + // HR y T
  "," + String(ppbNO2) + "," + String(ppmCO) + // sensor MICS-4514 NO2-CO
  "," + String(densidadPolvo) + "," + String(PM02_5) ; // sensor PM2.5

//------------------------ Memoria microSD -------------------------
  // Abrimos el fichero, si no existe lo crea
  File ficheroDatos = SD.open("SD.csv", FILE_WRITE);
  //Si el fichero esta habilitado escribe en el mismo 
  if (ficheroDatos) {
    ficheroDatos.println(datos);
    ficheroDatos.close();
    Serial.println(datos);
  }
  // Si no esta abierto, mensaje de error
  else {
    Serial.println("Error abriendo SD.csv");
  }
  delay(100);
  //snoozeLib.snooze(60000 - 10000); // Duermete 1 minuto (restamos 10000 para no sumar el delay(10000) al sleep 
  snoozeLib.snooze(3600000 - 10000); // Duermete 1h (restamos 10000 para no sumar el delay(10000) al sleep 
  //snoozeLib.snooze(7200000 - 10000); // Duermete 2h (restamos 10000 para no sumar el delay(10000) al sleep 
   snoozeLib.wakeup(); // Despierta
}
