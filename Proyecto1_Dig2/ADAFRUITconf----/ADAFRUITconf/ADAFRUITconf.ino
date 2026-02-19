#include "config.h"
#include <Wire.h>

/************************ PROTOTIPOS *******************************/
void handleMessageangulox1(AdafruitIO_Data *data);
void handleMessageanguloy1(AdafruitIO_Data *data);
void handleMessagedisparorx(AdafruitIO_Data *data);
void handleMessageautorizacionrx(AdafruitIO_Data *data);
void procesarUART(String data);
void enviarPaqueteUART();

// variables globales
int16_t angulo_x = 0; 
int16_t angulo_y = 0; 
uint16_t dist = 0; 
int disp = 0; 
int c = 0; 
int mot = 0; 

#define IO_LOOP_DELAY 10000
unsigned long lastUpdate = 0;

// ===== FEEDS DE SALIDA =====
AdafruitIO_Feed *anguloxFeed = io.feed("angulo x");
AdafruitIO_Feed *anguloyFeed = io.feed("angulo y");
AdafruitIO_Feed *autoFeed = io.feed("autorización");
AdafruitIO_Feed *disparoFeed = io.feed("disparo");
AdafruitIO_Feed *distanciaFeed = io.feed("distancia del Objetivo");
AdafruitIO_Feed *motorFeed = io.feed("motor");

// ===== FEEDS DE ENTRADA =====
AdafruitIO_Feed *angulox1Feed = io.feed("angulo x1");
AdafruitIO_Feed *anguloy1Feed = io.feed("angulo y1");
AdafruitIO_Feed *disparoRXFeed = io.feed("disparorx");

// 🔁 CAMBIO AQUÍ → ahora se llama ejecutar
AdafruitIO_Feed *autoRXFeed = io.feed("ejecutar");

String bufferUART = "";

void setup() {

  Serial.begin(115200);
  while(!Serial);

  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Serial.print("Connecting to Adafruit IO");
  io.connect();

  angulox1Feed->onMessage(handleMessageangulox1);
  anguloy1Feed->onMessage(handleMessageanguloy1);
  disparoRXFeed->onMessage(handleMessagedisparorx);
  autoRXFeed->onMessage(handleMessageautorizacionrx);

  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
  Serial.println("ESP32 listo para recibir datos desde Adafruit");

  // Obtener valores iniciales
  angulox1Feed->get();
  anguloy1Feed->get();
  disparoRXFeed->get();
  autoRXFeed->get();   // ← IMPORTANTE
}

void loop() {

  io.run();

  static unsigned long lastUART = 0;

  if (millis() - lastUART > 500) {
    enviarPaqueteUART();
    lastUART = millis();
  }

  // ===== RECEPCIÓN DESDE NANO =====
  while (Serial2.available()) {

    char cIn = Serial2.read();

    if (cIn == '<') {
      bufferUART = "";
    }
    else if (cIn == '>') {
      procesarUART(bufferUART);
    }
    else {
      bufferUART += cIn;
    }
  }

  // ===== ACTUALIZAR ADAFRUIT CADA 10 SEGUNDOS =====
  if (millis() > lastUpdate + IO_LOOP_DELAY) {

    Serial.println("Actualizando feeds en Adafruit...");

    anguloxFeed->save(angulo_x); 
    anguloyFeed->save(angulo_y);
    distanciaFeed->save(dist);
    disparoFeed->save(disp);
    autoFeed->save(c);
    motorFeed->save(mot); 

    lastUpdate = millis();
  }
}

// ===== PROCESAR DATOS DEL NANO =====
void procesarUART(String data) {

  int ax = angulo_x;
  int ay = angulo_y;
  int distancia = dist;
  int autoInt = c;
  int mot1 = mot;

  sscanf(data.c_str(), "%d,%d,%d,%d,%d",
         &ax, &ay, &distancia, &autoInt, &mot1);

  angulo_x = ax;
  angulo_y = ay;
  dist = distancia;
  c = autoInt;
  mot = mot1; 

  Serial.println("Datos recibidos del Nano:");
  Serial.println(data);
}

// ===== CALLBACKS DESDE ADAFRUIT =====

void handleMessageangulox1(AdafruitIO_Data *data) {

  angulo_x = data->toInt();

  Serial.print("Nuevo angulo X desde IO -> ");
  Serial.println(angulo_x);

  enviarPaqueteUART();
}

void handleMessageanguloy1(AdafruitIO_Data *data) {

  angulo_y = data->toInt();

  Serial.print("Nuevo angulo Y desde IO -> ");
  Serial.println(angulo_y);

  enviarPaqueteUART();
}

void handleMessagedisparorx(AdafruitIO_Data *data) {

  disp = data->toInt();

  Serial.print("Nuevo disparo desde IO -> ");
  Serial.println(disp);

  enviarPaqueteUART();
}

void handleMessageautorizacionrx(AdafruitIO_Data *data) {

  c = data->toInt();

  Serial.print("Nuevo ejecutar desde IO -> ");
  Serial.println(c);

  enviarPaqueteUART();
}

void enviarPaqueteUART() {

  String paquete = "";

  paquete += abs(angulo_x);
  paquete += ",";
  paquete += abs(angulo_y);
  paquete += ",";
  paquete += dist;
  paquete += ",";
  paquete += disp;
  paquete += ",";
  paquete += c;

  Serial.print("Enviando al Arduino -> ");
  Serial.println(paquete);

  Serial2.println(paquete);
}
