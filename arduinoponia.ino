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

// Servo
#include <Servo.h>

//Constants
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


//Variables
int chk;
float hum;
float temp;

int valorHumedadTierra;
int valorHumedadTierraDigital;

int valorLuz;
int valorLuzDigital;

byte byteRead;
long previousTime = 0;
long interval = 60000;

String current_mode = "default";

// Servo
Servo myservo;


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
int temperatura_umbral_min;
int temperatura_umbral_max;
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
    //rtc.setDOW(SUNDAY);     // Set Day-of-Week to SUNDAY
    //rtc.setTime(20, 58, 50);     // Set the time to 12:00:00 (24hr format)
    //rtc.setDate(17, 7, 2016);   // Set the date to January 1st, 2014

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

   Serial.println("Arduinoponio te escucha, ingresa un comando....");
 }

void applySetting(String settingName, int settingValue) {
  
   if(settingName == "luz_horario_encendido") {
       //luz_horario_encendido=settingValue;
       luz_horario_encendido=0;
   }
   
   if(settingName == "luz_horario_apagado") {
       //luz_horario_apagado=settingValue;
       luz_horario_apagado=8;
   }

   if(settingName == "riego_umbral") {
       riego_umbral=settingValue;
   }

   if(settingName == "temperatura_umbral_min") {
       //temperatura_umbral=settingValue;
       temperatura_umbral_min=22;
   }

   if(settingName == "temperatura_umbral_max") {
       //temperatura_umbral=settingValue;
       temperatura_umbral_max=23;
   }

   if(settingName == "current_mode") {
       //current_mode=settingValue;
       current_mode="temperatura";
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
  int pos = 0;
  logDataln("[Activo riego]");
  myservo.attach(12);
  for(pos = 0; pos < 50; pos += 1)  
  {                                  
    myservo.write(pos);              
    delay(15);                       
  }
  logDataln("[---------------------------]");
  myservo.detach();
  delay(5000); //Delay 5 seg.
  myservo.attach(12);
  logDataln("[Desactivo riego]");
  for(pos = 70; pos>=1; pos-=1)  
  {                                
    myservo.write(pos);              
    delay(15);                       
  }
  myservo.detach();
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

void revisarEstadoLuz(){
  // Get hora actual
  String horaActual = currentTime.substring(0, currentTime.indexOf(':'));
  
  if(horaActual.toInt()>=luz_horario_encendido && horaActual.toInt()<luz_horario_apagado){
    prenderLuz();
  }
  else{
    apagarLuz();
  }
}

void loop()
{

  unsigned long currentTime = millis();

  if(currentTime - previousTime > interval){
    previousTime = currentTime;
    
    // Si es hora de prender la luz, la prendo
    revisarEstadoLuz();
  
    // Mido la humedad de la tierra, si es  menor a riego_umbral y la luz esta apagada, riego.
    if(valorHumedadTierra<riego_umbral && luz_prendida == false){
      regar();  
    }
  
    logDataln("[temp<temperatura_umbral_min]");
    logDataln(String(temp<temperatura_umbral_min));
  
    logDataln("[temp]");
    logDataln(String(temp));
  
    logDataln("[temperatura_umbral_min]");
    logDataln(String(temperatura_umbral_min));
    
    // Mido la temperatura del ambiente, si es  menor a temperatura_umbral_min y la luz esta apagada, la prendo.
    if(temp<=21 && luz_prendida == false){
      prenderLuz();
      logDataln("[La temperatura es muy baja, prendo la luz]");
    }
  
    logDataln("[temp>temperatura_umbral_max]");
    logDataln(String(temp>temperatura_umbral_max));
    // Mido la temperatura del ambiente, si es  mayor a temperatura_umbral_max y la luz esta prendida, la apagago
    if(temp>23 && luz_prendida == true){
      apagarLuz();
      logDataln("[La temperatura es correcta, apago la luz]");
    }
  
    // Imprimo el registro al puerto serial y a data.log
    doLog();
  }
    
    if (Serial.available()) {
      /* read the most recent byte */
      byteRead = Serial.read();
      /*ECHO the value that was read, back to the serial port. */
      switch(byteRead){
        case 'h':
          {
             Serial.println("Ayuda");
             break;
          }
          
        
        case '1':
          {
            Serial.println("[Current Mode is "+current_mode+"]");
            Serial.println("Modos");
            Serial.println("1) Automatico");
            Serial.println("2) Manual");
            break;
          }
          

        case '2':
          {
            Serial.println("Estadoa actual");
            Serial.println("");
            Serial.println("");
            break;
          }
          

        case '3':
          {
            Serial.println("Acerca de...");
            Serial.println("Arduinoponia 0.1");
            Serial.println("https://github.com/smarbos/arduinoponia");
             break;
          }
         
        case '4':
          {
            Serial.print("Leyendo settings.ini:");
   break;
  
          }
    
      }
      //Serial.write(byteRead);
    }
    
  
}
