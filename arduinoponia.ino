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
File mySettingsFile;

//Configuration
int luz_horario_encendido;
int luz_horario_apagado;
int riego_umbral;
String today;
String currentTime;
bool luz_prendida = false;

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
    //rtc.setTime(20, 27, 00);     // Set the time to 12:00:00 (24hr format)
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


         applySetting(settingName,settingValue.toInt());

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

void applySetting(String settingName, int settingValue) {
  
   if(settingName == "luz_horario_encendido") {
       luz_horario_encendido=settingValue;
   }
   
   if(settingName == "luz_horario_apagado") {
       luz_horario_apagado=settingValue;
   }

   if(settingName == "riego_umbral") {
       riego_umbral=settingValue;
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

void doLog(){
  myFile = SD.open("data.log", FILE_WRITE);

  if(today.length()==0){
    today = rtc.getDateStr();
    logDataln("-----------[today.length()==0]-----------");
    logDataln("-----------["+String(today)+"]-----------");
  }
  
  if(today != rtc.getDateStr()){
    today = rtc.getDateStr();
    logDataln("-----------[today != rtc.getDateStr()]-----------");
    logDataln("-----------["+String(today)+"]-----------");
  }

  // Trae la hora actual
  currentTime = rtc.getTimeStr();
  
  // Mostrar hora
  logDataln(currentTime);

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

  // Sensar humedad del sustrato digital
  valorHumedadTierraDigital = digitalRead(3);

  //Logear humedad del sustrato
  logData("Humedad sustrato:");
  logData(String(valorHumedadTierra));
  logData("% | ");
  logDataln(String(valorHumedadTierraDigital));

  // Sensar luz
  valorLuzDigital = digitalRead(4);

  if(valorLuzDigital == 1){
    logDataln("Sensor Luz: Luz Apagada");
  }
  else{
    logDataln("Sensor Luz: Luz Prendida");
  }

  if(luz_prendida){
    logDataln("Estado de la luz: PRENDIDA");
  }
  else {
    logDataln("Estado de la luz: APAGADA");
  }

  logDataln("[---------------------------]");
  
  myFile.close();
}

void regar(){
  logDataln("[Activo riego]");
  logDataln("[---------------------------]");
  //delay(5000); //Delay 5 seg.
  logDataln("[Desactivo riego]");
  logDataln("[---------------------------]");
}

void prenderLuz(){
  luz_prendida=true;
  digitalWrite(10, HIGH);
  logDataln("[Prendo luz]");
  logDataln("[---------------------------]");
}

void apagarLuz(){
  digitalWrite(10, LOW);
  luz_prendida=false;
  logDataln("[Apago luz]");
  logDataln("[---------------------------]");
}

void loop()
{
  // Get hora actual
  String horaActual = currentTime.substring(0, currentTime.indexOf(':'));
  
  //logDataln("Hora actual: "+String(horaActual));
  //logDataln("Hora encendido luz: "+String(luz_horario_encendido));
  //logDataln("Hora apagado luz: "+String(luz_horario_apagado));
  
  // Si es hora de prender la luz, la prendo
  if(horaActual.toInt()>=luz_horario_encendido && horaActual.toInt()<luz_horario_apagado){ // 20>=19 && 20<20
     prenderLuz();
  }

  // Si es hora de apagar la luz, la apago
  if(horaActual.toInt()>=luz_horario_apagado){ // 20<19 && 20>=20
    apagarLuz();
  }

  // Mido la humedad de la tierra, si es  menor a riego_umbral y la luz esta apagada, riego.
  if(valorHumedadTierra<riego_umbral && luz_prendida == false){
    regar();  
  }

  // Imprimo el registro al puerto serial y a data.log
  doLog();
  
  delay(60000); //Delay 30 min.
}
