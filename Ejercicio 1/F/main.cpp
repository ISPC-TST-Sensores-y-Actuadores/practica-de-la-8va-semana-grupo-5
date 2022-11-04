#include <Arduino.h>
#include <MFRC522.h> // para la RFID
#include <SPI.h> // para el modulo de tarjeta RFID y SD
#include <SD.h> // para la tarjeta SD
#include <RTClib.h> // para el RTC

// definir pines para RFID
#define CS_RFID 10
#define RST_RFID 9
// definir el pin de seleccion para el modulo de la tarjeta SD
#define CS_SD 4 

// Crea un archivo para almacenar los datos
File myFile;

// Instancia de la clase para RFID
MFRC522 rfid(CS_RFID, RST_RFID); 

// Variable para contener el UID de la etiqueta
String uidString;

// Instancia de la clase para RTC
RTC_DS1307 rtc;

// Definir hora de entrada
const int checkInHour = 9;
const int checkInMinute = 5;

// Variable para mantener el registro de usuario
int userCheckInHour;
int userCheckInMinute;

// Pines para LED y buzzer
const int redLED = 6;
const int greenLED = 7;
const int buzzer = 5;

void setup() {
  
  // Establecer LED y buzzer como salidas 
  pinMode(redLED, OUTPUT);  
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Inicializar puerto serie
  Serial.begin(9600);
  while(!Serial); 
  
  // Inicializacion del bus SPI 
  SPI.begin(); 
  // Inicializar MFRC522 
  rfid.PCD_Init(); 

  // Configuracion de la tarjeta SD
  Serial.print("Inicializando la tarjeta SD...");
  if(!SD.begin(CS_SD)) {
    Serial.println("Fallo la inicializacion!");
    return;
  }
  Serial.println("inicializacion realizada.");

  // Configuracion del RTC  
  if(!rtc.begin()) {
    Serial.println("No se pudo encontrar el RTC");
    while(1);
  }
  else {
    // la linea siguiente establece el RTC en la fecha y hora en que se compilo este boceto 
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if(!rtc.isrunning()) {
    Serial.println(" RTC NO se esta ejecutando!");
  }
}

void loop() {
  //look for new cards
  if(rfid.PICC_IsNewCardPresent()) {
    readRFID();
    logCard();
    verifyCheckIn();
  }
  delay(10);
}

void readRFID() {
  rfid.PICC_ReadCardSerial();
  Serial.print("Tag UID: ");
  uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " + 
    String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
  Serial.println(uidString);
 
  // Hacer sonar el buzzer cuando se lee una tarjeta 
  tone(buzzer, 2000); 
  delay(100);        
  noTone(buzzer);
  
  delay(100);
}

void logCard() {
  //  Habilita el pin de seleccion del chip de la tarjeta SD 
  digitalWrite(CS_SD,LOW);
  
  // Abrir archivo
  myFile=SD.open("DATA.txt", FILE_WRITE);

  // Si el archivo se abrio correctamente
  if (myFile) {
    Serial.println("File opened ok");
    myFile.print(uidString);
    myFile.print(", ");   
    
    // Guarde tiempo en la tarjeta SD 
    DateTime now = rtc.now();
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(',');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.println(now.minute(), DEC);
    
    // Imprime la hora en el monitor serie
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.println(now.minute(), DEC);
    Serial.println("escrito con exito en la tarjeta SD");
    myFile.close();

    // Guardar hora de entrada
    userCheckInHour = now.hour();
    userCheckInMinute = now.minute();
  }
  else {
    Serial.println("error opening data.txt");  
  }
  // Desactiva el pin de seleccion del chip de la tarjeta SD 
  digitalWrite(CS_SD,HIGH);
}

void verifyCheckIn(){
  if((userCheckInHour < checkInHour)||((userCheckInHour==checkInHour) && (userCheckInMinute <= checkInMinute))){
    digitalWrite(greenLED, HIGH);
    delay(2000);
    digitalWrite(greenLED,LOW);
    Serial.println("De nada!");
  }
  else{
    digitalWrite(redLED, HIGH);
    delay(2000);
    digitalWrite(redLED,LOW);
    Serial.println("Llegas tarde...");
  }
}