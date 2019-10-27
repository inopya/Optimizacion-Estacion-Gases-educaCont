/*
          INOPYA SYSTEMS

  #       _\|/_   A ver..., ¿que tenemos por aqui?
  #       (O-O)        
  # ---oOO-(_)-OOo-----------------------------------------
   
  Correccion del firmware de la Estacion de medicion de calidad del aire 
  del Proyecto EducaCont para la optimizacion de uso de memoria SRAM 
  solventando asi la imposibilidad de añadir nuevos sensores y el adecuado aprovechamiento 
  del kit de expansion que se puede adquirir para dicha estacion.
  
  Puede consultarse el codigo original (con sus defectillos) en:
  https://sites.google.com/view/educacont/documentaci%C3%B3n/firmwares
  https://drive.google.com/file/d/1D3JPS6QPwnk5GonNRx_Yl9s7XHZ3LTxn/view?usp=drive_web

  El original no soporta sensores de presion o ampliaciones de otro tipo debido a una mala gestion de la memoria.
  Se ha optimizado el codigo para corregir dichos problemas y añadido el sensor de presion BMP180.
  
  Una vez optimizado el codigo para corregir los problemas derivados de esa mala gestion de memoria, 
  se ha añadido el sensor de presion BMP180 a modo de ejemplo.
  
  Con solo usar la macro F() para el compilador, liberamos 671 bytes en cadenas de texto.
  Podemos asi mismo, aligerar tambien el uso de memoria Flash si condicionamos dichas cadenas de texto con
  #ifdef / #endif, ya que si bien pueden tener alguna utilidad en modo 'DEBUG', 
  son totalmente innecesarias en el modo de operacion normal.
  
  Se ha reestructurado el codigo y sus comentarios para una mayor legibilidad.
  
  ** Recordemos siempre que el codigo se escribe una vez, pero posiblemente se lea muchas veces y/o por muchos. **




  
  ###########################################################
  # ******************************************************* #
  # *            ARDUINO PARA PRINCIPIANTES               * #
  # *     ESTACION PARA MEDICION DE CALIDAD DEL AIRE      * #
  # *                   Autor: Proyecto EducaCont         * #
  # *  Corregida y optimizada: Eulogio López Cayuela      * #
  # *                   https://github.com/inopya         * #
  # *                                                     * #
  # *       Versión v1.0      Fecha: 24/10/2019           * #
  # ******************************************************* #
  ###########################################################
  
*/


#define __VERSION__   "Estacion Calidad Aire EducaCont,\nCorregida por INOPYA\n\t https://github.com/inopya\n"

//#define SERIAL_MODE     //descomentar si se desea que imprima mensajes e informacion por Serial
//#define DEBUG_MODE      //descomentar para tener informacion completa del bmp180 (debug)



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        IMPORTACION DE LIBRERIAS 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/
#include <Wire.h>                     // biblioteca para comunicaciones I2C
#include <SFE_BMP180.h>               // biblioteca para el sensor de presion y temperatura
#include <DS3231.h>                   // para el RTC I2C
#include <SPI.h>                      // para el modulo microSD   
#include <SD.h>                       // para el modulo microSD   
#include "DHT.h"                      // para el sensor DHT22
#include <SnoozeLib.h>                // Para usar el modo sleep


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DECLARACION DE CONSTANTES  Y  VARIABLES GLOBALES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

/* Para el sensor DHT22 */
#define DHTTYPE    DHT22              // tipo de sensor, DHT 22 
#define DHTPin         9              // Lo conectamos al pin digital 9 


/* Para la SD */
#define chipSelect    10              // pin de seleccion microSD

/* Para el sensor barometrico BMP180 */
#define ALTITUD   407.0               // Altitud de La Tejica (Sorbas) en metros
float Temperatura = 0;                // variable para la temperatura leida del BMP180 (sin uso)
float PresionRelativaCotaCero = 0;    // Presion relativa a nivel de mar (sin uso)
float PresionABS = 0;                 // variable para la presion(absoluta) leida del BMP180

/* Para el sensor  MICS-4514 */
#define PRE_PIN              8        // precalentamiento
#define VNOX_PIN            A0        // pin datos de NO2
#define VRED_PIN            A1        // pin datos de CO
#define PRE_HEAT_SECONDS    10        // tiempo de precalentamiento

/* Para el sensor de particulas KS0196 PM2.5 */
#define pinMedida            6        // Sensor conectado a pin analógico A6
#define ledPower             2        // El LED va conectado al pin digital 2
int tiempoMuestreo = 280;
int deltaTime = 40;
int sleepTime = 9680;


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    Creamos las instancia de los objetos
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

/* Objeto barometro basado en el BMP180 */
SFE_BMP180 sensorBMP180; 

/* Reloj de Tiempo Reald DS3231 */  
DS3231  rtc(SDA, SCL);

/* Higrometro DHT22 */
DHT dht(DHTPin, DHTTYPE); 



//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 
//***************************************************************************************************
//         FUNCION DE CONFIGURACION
//***************************************************************************************************
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 
     
void setup()
{
  /* Configuracion de la conexión serie */

  Serial.begin(115200);
  Serial.println(F(__VERSION__));

    
  #ifdef SERIAL_MODE
    Serial.println(F("==========================================================="));
    Serial.println(F("--------------------- Configuraciones ---------------------"));
    Serial.println(F("==========================================================="));
  #endif

  /* Inicializacion del barometro BMP180 */
  if (sensorBMP180.begin()){
    #ifdef SERIAL_MODE
      Serial.println(F("BMP180 iniciado"));
    #endif
  }
  else {
    #ifdef SERIAL_MODE
      Serial.println(F("BMP180 no presente"));
    #endif
    return;
  } 
   
  /*  Inicializa el objeto rtc */
  rtc.begin();
  
  /* Inicializacion Memoria micoSD */
  #ifdef SERIAL_MODE  
    Serial.println(F("Inicializando la tarjeta micro SD ..."));
  #endif    
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Error al iniciar la micro SD"));
    return;
  }
  #ifdef SERIAL_MODE 
    Serial.println(F("Tarjeta micro SD iniciada correctamente"));
  #endif
  
  /* Inicializacion SENSOR DHTxx */  
  dht.begin();
  #ifdef SERIAL_MODE 
    Serial.println(F("DHTxx inicializado correctamente..."));
  #endif
  
  /* Inicializacion SENSOR MICS-4514 */
  pinMode(PRE_PIN, OUTPUT);
  
  #ifdef SERIAL_MODE 
    Serial.println(F("Precalentando el sensor MiCS-4514 durante 10s ..."));
  #endif
  
  /* precalentar el sensor de gases durante 10 segundos */
  digitalWrite(PRE_PIN, 1);
  delay(PRE_HEAT_SECONDS * 1000);
  digitalWrite(PRE_PIN, 0);
  
  #ifdef SERIAL_MODE 
    Serial.println(F("Sensor MiCS-4514 listo ..."));
  #endif
  
  /* configurar pin del sensor de particulas */
  pinMode(ledPower, OUTPUT);        // Pin 'ledPower' como salida 
  #ifdef SERIAL_MODE  
    Serial.println(F("Sensor de particulas KS0196 PM2.5 configurado ...\n"));
  #endif
  
  #ifdef SERIAL_MODE     
    Serial.println(F("==========================================================="));
    Serial.println(F("-------------------------- DATOS --------------------------"));
    Serial.println(F("==========================================================="));
  #endif
}



//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 
//***************************************************************************************************
//  BUCLE PRINCIPAL DEL PROGRAMA   (SISTEMA VEGETATIVO)
//***************************************************************************************************
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 

void loop()
{

  #ifdef DEBUG_MODE
    mostrar_bmp180();
  #endif

  /* ==================================================== */
  /* Obtener Temperatura y Humedad Relativa del DHTxx  */
  // Leer la temperatura o la humedad relativa toma alrededor de 250 ms
  float h = dht.readHumidity(); 
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    #ifdef SERIAL_MODE 
      Serial.println(F("Error al obtener datos del sensor DHTxx"));
    #endif
    return;
  } 

  /* Lectura del NO2 y CO */
  float vnox_value = analogRead(VNOX_PIN)/409.2;          // lee y convierte la lectura analogica a tension
  float Rsnox = 22000/((5/vnox_value) - 1);               // obtiene la resistencia del sensor a partir de 5V y una carga de 22 kOhm
  float ppbNO2 = (.000008*Rsnox - .0194)*1000;            // convierte Rsnox a concentration de NO2 en ppb    
  float vred_value = analogRead(VRED_PIN)/409.2;          // lee y convierte la lectura analogica a tension
  float RsCO = 100000/((5/vred_value) - 1);               // obtiene la resistencia del sensor a partir de 5V y una carga de 100 kOhm
  float ppmCO = 911.19*pow(2.71828,(-8.577*RsCO/100000)); //convierte RsCO a concentracion de CO en ppm 

  /* ==================================================== */
  /* Lectura del sensor PM2.5 */
  digitalWrite(ledPower, LOW);                            // Activamos el LED IR
  delayMicroseconds(tiempoMuestreo);                      // Retardo de 280 us antes de medir
  int Vmed = analogRead(pinMedida);                       // Lectura del pin analógico A2
  delayMicroseconds(deltaTime);                           // Retardo de 40 us
  digitalWrite(ledPower,HIGH);                            // Desactivamos el LED IR 
  delayMicroseconds(sleepTime);                           // Retardo de 9680 us antes de otra lectura
  
  float Vcal = Vmed * (5.0 / 1024.0);                     // Mapeo de 1023 valores enteros a 5V 
  float densidadPolvo = float(0.172 * Vcal - 0.0999);     // Ecuación lineal
  float PM02_5 = float((Vmed/1024)-0.0356)*120000*0.035;  // Ecuación lineal para PM2.5
  // el retardo total es de (tiempoMuestreo + deltaTime + sleepTime = 280 + 40 + 9680 = 10000us) de 10ms

  /* ==================================================== */
  /* Lectura del BMP180 */
  obtenerDatosSensorBMP180();

  /* ==================================================== */
  /* Preparacion de los datos para escribir en el fichero */
  String datos = String (rtc.getDateStr()) + "," + String(rtc.getTimeStr())+      // fecha y hora
  "," + String(h) + "," + String(t) +                                             // humedad y temepratura 
  "," + String(ppbNO2) + "," + String(ppmCO) +                                    // sensor MICS-4514 NO2-CO
  "," + String(densidadPolvo) + "," + String(PM02_5) +                            // sensor PM2.5
  "," + String(PresionABS);                                                       // sensor BMP180

  /* ==================================================== */
  /* Escribir informacion en la Memoria microSD */
  File ficheroDatos = SD.open("SD.csv", FILE_WRITE);  // Abrimos el fichero, si no existe lo crea
  if (ficheroDatos) {                                 //Si el fichero esta habilitado escribe en el mismo 
    ficheroDatos.println(datos);
    ficheroDatos.close();
    #ifdef DEBUG_MODE
      Serial.println(datos);
    #endif
  }
  else {                                              // Si no esta abierto, mensaje de error
    #ifdef SERIAL_MODE
      Serial.println(F("Error abriendo SD.csv"));
    #endif
  }

  /* ==================================================== */
  /* Modo sleep para reducir consumo del ATMega328p */
  delay(100);
  snoozeLib.snooze(5000);                 //solo 5 segundos, para ir mas rapido en el modo DEBUG
  //snoozeLib.snooze(60000 - 10000);      // Duermete 1 minuto (restamos 10000 para no sumar el delay(10000) al sleep 
  //snoozeLib.snooze(3600000 - 10000);    // Duermete 1h (restamos 10000 para no sumar el delay(10000) al sleep 
  //snoozeLib.snooze(7200000 - 10000);    // Duermete 2h (restamos 10000 para no sumar el delay(10000) al sleep 
   snoozeLib.wakeup();                    // Despierta
}



//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 
//***************************************************************************************************
//  BLOQUE DE FUNCIONES: LECTURA SENSORES, TOMA DE DECISIONES, ...
//***************************************************************************************************
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm 

/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    BAROMETRO 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
//  BAROMETRO, usando sensor BMP180
//========================================================
void obtenerDatosSensorBMP180()
{
  char estado;
  double T,P,p0,a;
  boolean FLAG_fallo_BMP180 = false;
  
  /* 
   * Primero se debe hacer una lectura de la temepratura para poder hacer una medida de presion.
   * Se inicia el proceso de lectura de la temperatura.
   * Si se realiza sin errores, se devuelve un numero de (ms) de espera, si no, la funcion devuelve 0.
   */
  
  estado = sensorBMP180.startTemperature();
  if (estado != 0) {
    delay(estado);  // pausa para que se complete la medicion en el sensor.

    // Obtencion de la medida de temperatura que se almacena en T:
    // Si la lectura el correcta la funcion devuelve 1, si se producen errores, devuelve 0.

    estado = sensorBMP180.getTemperature(T);
    if (estado != 0) {
      Temperatura = T;  //Asignacion a variable global
      
      /* 
       * Se inicia el proceso de lectura de la presion.
       * El parametro para la resolucion de muestreo varia de 0 a 3 (a mas resolucion, mayor tiempo necesario).
       * Si se realiza sin errores, se devuelve un numero de (ms) de espera, si no, la funcion devuelve 0.
       */

      estado = sensorBMP180.startPressure(3);
      if (estado != 0) { 
        delay(estado); // pausa para que se complete la medicion en el sensor.
        
        // Obtencion de la medida de Presion que se almacena en P:
        // Si la lectura el correcta la funcion devuelve 1, si se producen errores, devuelve 0.
        estado = sensorBMP180.getPressure(P,T);

        if (estado != 0) {
          PresionABS = P;  //Asignacion a variable global

          /* 
           * El sensor devuelve presion absoluta. Para compensar el efecto de la altitud
           * usamos la funcion interna de la libreria del sensor llamada: 'sealevel'
           * P = presion absoluta en (mb) y ALTITUD = la altitud del punto en que estomos (m).
           * Resultado: p0 = presion compensada a niveldel mar en (mb)
           */

          p0 = sensorBMP180.sealevel(P,ALTITUD);  // 407 metros (SORBAS, La Tejica)
          PresionRelativaCotaCero = p0;           //Asignacion a variable global
        }
        else FLAG_fallo_BMP180 = true; //error en las lecturas/obtencion de datos
      }
      else FLAG_fallo_BMP180 = true;
    }
    else FLAG_fallo_BMP180 = true;
  }
  else FLAG_fallo_BMP180 = true;

  /*
   * Ante fallos de lectura del sensor barometrico establecemos valores fuera de rango
   * para facilitar la captura del error desde programas externos
   */
  if (FLAG_fallo_BMP180 == true){
    Temperatura = -100;
    PresionABS = -100;
    PresionRelativaCotaCero = -100;
  }
}


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    DEBUG 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void mostrar_bmp180()
{
  Serial.println(""); 
  Serial.print(Temperatura);              //mostramos la temperatura
  Serial.print("**");
  Serial.print(PresionRelativaCotaCero);  //mostramos la presion RELATIVA
  Serial.print("**");
  Serial.println(PresionABS);               //mostramos la presion ABSOLUTA   
}
