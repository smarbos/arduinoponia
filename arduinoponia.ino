//------------------------------------------------------------------------------
//
//      Arduinoponia (https://github.com/smarbos/arduinoponia)
//
//      Por:
//          https://github.com/ncastrohub
//      y
//          https://github.com/smarbos
//
//------------------------------------------------------------------------------

//Libraries
// Sensor Humedad Ambiente
#include <DHT.h>

// Reloj
#include <DS3231.h>

#include <SPI.h>

// Tarjeta SD
#include <SD.h>

//Constants
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


//Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

int valorHumedadTierra;
int valorHumedadTierraDigital;

int valorLuz;
int valorLuzDigital;

//------SD CARD-----//
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 53;
File myFile;

//Configuration
String luz_cantidad_horas;
String luz_horario_encendido;
String riego_cantidad_agua;
String riego_cada_cantidad_horas;

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);


void setup()
{
    Serial.begin(9600);

    //Sensor ambiente
    dht.begin();
    SD.begin(53);

    //Reloj
    rtc.begin();

    // Aca se puede configurar el reloj
    //rtc.setDOW(SATURDAY);     // Set Day-of-Week to SUNDAY
    //rtc.setTime(00, 42, 20);     // Set the time to 12:00:00 (24hr format)
    //rtc.setDate(2, 7, 2016);   // Set the date to January 1st, 2014

    // Ping 10 indica la luz encendida
    pinMode(10, OUTPUT);

    // Pin 53 es utilizado por la SD para escribir
    pinMode(53, OUTPUT);

    Serial.println("Inicializando tarjeta SD...");


  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("[ERROR] No se pudo leer la memoria SD.");

    return;
  } else {
    Serial.println("[OK] Tarjeta SD funcionando.");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("[ERROR] No se puede leer la SD. Esta formateada correctamente?");
    return;
  }

  myFile = SD.open("data.log", FILE_WRITE);
  if (myFile) {
    Serial.println("[OK] Archivo de registro abierto.");
  } else {
    Serial.println("[ERROR] No se pudo abrir el archivo de registro.");
  }

  readSDSettings();

}

void readSDSettings(){
     char character;
     String settingName;
     String settingValue;
     myFile = SD.open("settings.ini");
     if (myFile) {
         while (myFile.available()) {
             character = myFile.read();
             while((myFile.available()) && (character != '[')){
                 character = myFile.read();
             }
             character = myFile.read();
             while((myFile.available()) && (character != '=')){
                 settingName = settingName + character;
                 character = myFile.read();
             }
             character = myFile.read();
             while((myFile.available()) && (character != ']')){
                 settingValue = settingValue + character;
                 character = myFile.read();
             }
             if(character == ']'){

                 //Debuuging Printing
                 Serial.print("Name:");
                 Serial.println(settingName);
                 Serial.print("Value :");
                 Serial.println(settingValue);


                 applySetting(settingName,settingValue);

                 // Reset Strings
                 settingName = "";
                 settingValue = "";
             }
         }
         // Cierra el archivo:
         myFile.close();
     } else {
         // si no se pudo abrir el archivo, aviso.
         logDataln("[ERROR] No se pudo abrir el archivo de configuracion.");
     }
 }


void applySetting(String settingName, String settingValue) {
   if(settingName == "luz_cantidad_horas") {
       luz_cantidad_horas=settingValue;
   }
   if(settingName == "luz_horario_encendido") {
       luz_horario_encendido=settingValue;
   }
   if(settingName == "riego_cantidad_agua") {
       riego_cantidad_agua=settingValue;
   }
   if(settingName == "riego_cada_cantidad_horas") {
       riego_cada_cantidad_horas=settingValue;
   }
}

void logDataln(String data){
  Serial.println(data);
  myFile.println(data);
}

void logData(String data){
  Serial.print(data);
  myFile.print(data);
}

void loop()
{

  myFile = SD.open("data.log", FILE_WRITE);
  logDataln("[-----------START-----------]");

  // Mostrar dia de la semana
  logDataln(rtc.getDOWStr());

  // Mostrar fecha
  logDataln(rtc.getDateStr());



  // Mostrar hora
  logDataln(rtc.getTimeStr());

  // Sensar humedad y temperatura ambiente
  hum = dht.readHumidity();
  temp= dht.readTemperature();

  // Logear humedad ambiente
  logData("Humedad Ambiente: ");
  logData(String(hum));
  logDataln(" %");

  // Logear temperatura ambiente
  logData("Temperatura Ambiente: ");
  logData(String(temp));
  logDataln("° C");

  // Sensar humedad del sustrato analogico
  valorHumedadTierra = analogRead(0);
  valorHumedadTierra = map(valorHumedadTierra, 0, 1024, 100, 0);
  //valorHumedadTierra = valorHumedadTierra+100;

  // Sensar humedad del sustrato digital
  valorHumedadTierraDigital = digitalRead(3);

  //Logear humedad del sustrato
  logData("Humedad sustrato:");
  logData(String(valorHumedadTierra));
  logData("% | ");
  logDataln(String(valorHumedadTierraDigital));

  if (valorHumedadTierra <= 300)
     logDataln(" Encharcado");
  if ((valorHumedadTierra > 300) and (valorHumedadTierra <= 700))
      logDataln("Humedo, no regar");
  if (valorHumedadTierra > 700)
      logDataln(" Seco, necesitas regar");

  // Sensar luz
  valorLuzDigital = digitalRead(4);

  if(valorLuzDigital == 1){
    logDataln("Luz Apagada");
    digitalWrite(10, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  else{
    logDataln("Luz Prendida");
    digitalWrite(10, LOW);   // turn the LED on (HIGH is the voltage level)
  }

  logDataln("[------------END------------]");
  logDataln("[===========================]");

  myFile.close();
  delay(60000); //Delay 1 min.
}
