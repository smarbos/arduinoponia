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
#include <DHT.h>
#include <DS3231.h>
#include <SPI.h>
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
File myFile2;
File myFile3;


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

    // The following lines can be uncommented to set the date and time
    //rtc.setDOW(SATURDAY);     // Set Day-of-Week to SUNDAY
    //rtc.setTime(00, 42, 20);     // Set the time to 12:00:00 (24hr format)
    //rtc.setDate(2, 7, 2016);   // Set the date to January 1st, 2014

    pinMode(10, OUTPUT);
    pinMode(53, OUTPUT);

    Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);

  myFile = SD.open("data.log", FILE_WRITE);
  if (myFile) {
    // close the file:
    //myFile.close();
    Serial.println("data.log opened.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data.log");
  }


  // re-open the file for reading:
  //myFile3 = SD.open("data.log", FILE_WRITE);
  /*if (myFile3) {
    Serial.println("data.log:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data.log");
  }*/



}

void loop()
{
    myFile = SD.open("data.log", FILE_WRITE);
    Serial.println("[-----------START-----------]");
    myFile.println("[-----------START-----------]");

     // Send Day-of-Week
      Serial.print(rtc.getDOWStr());
      myFile.println(rtc.getDOWStr());
      Serial.print(" ");
      myFile.println(" ");

      // Send date
      Serial.print(rtc.getDateStr());
      myFile.println(rtc.getDateStr());
      Serial.print(" -- ");
      myFile.println(" -- ");

      // Send time
      Serial.print("[");
      myFile.println("[");
      Serial.print(rtc.getTimeStr());
      myFile.println(rtc.getTimeStr());
      Serial.println("]");
      myFile.println("]");

    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    //Print temp and humidity values to serial monitor
    Serial.print("Humidity: ");
    myFile.println("Humidity: ");
    Serial.print(hum);
    myFile.println(hum);
    Serial.println(" %");
    myFile.println(" %");
    Serial.print("Temp: ");
    myFile.println("Temp: ");
    Serial.print(temp);
    myFile.println(temp);
    Serial.println(" Celsius");
    myFile.println(" Celsius");




    Serial.print("Humedad sustrato:");
    myFile.println("Humedad sustrato:");
    valorHumedadTierra = analogRead(0);
    valorHumedadTierraDigital = digitalRead(3);
    Serial.print(valorHumedadTierra);
    myFile.println(valorHumedadTierra);
    Serial.print(" | ");
    myFile.println(" | ");
    Serial.print(valorHumedadTierraDigital);
    myFile.println(valorHumedadTierraDigital);
    Serial.println(" ");
    myFile.println(" ");

    if (valorHumedadTierra <= 300)
       Serial.println(" Encharcado");
       myFile.println(" Encharcado");
    if ((valorHumedadTierra > 300) and (valorHumedadTierra <= 700))
        Serial.println("Humedo, no regar");
        myFile.println("Humedo, no regar");
    if (valorHumedadTierra > 700)
        Serial.println(" Seco, necesitas regar");
        myFile.println(" Seco, necesitas regar");





    valorLuzDigital = digitalRead(4);
    if(valorLuzDigital == 1){
      Serial.print("Luz Apagada");
      myFile.println("Luz Apagada");
      digitalWrite(10, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else{
      Serial.print("Luz Prendida");
      myFile.println("Luz Prendida");
      digitalWrite(10, LOW);   // turn the LED on (HIGH is the voltage level)
    }
    Serial.println(" ");
    myFile.println(" ");

    /*if (valorLuz <= 300)
       Serial.println(" Encharcado");
    if ((valorLuz > 300) and (valorLuz <= 700))
        Serial.println(" Humedo, no regar");
    if (valorLuz > 700)
        Serial.println(" Seco, necesitas regar");
        */
    Serial.println("[------------END------------]");
    myFile.println("[------------END------------]");
    Serial.println("[===========================]");
    myFile.println("[===========================]");
    myFile.close();
    delay(60000); //Delay 1 min.
}
