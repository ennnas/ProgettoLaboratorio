#define BLYNK_PRINT Serial
#define OWN_ADDRESS  0x08
#define LED_PIN 13

// Leggere e scrivere sui file
#include <FileIO.h>
// Comunicare con il sistema operativo Linino
#include <Bridge.h>
// Blynk per YUN
#include <BlynkSimpleYun.h>
// Accedere alla EEPROM
#include <EEPROM.h>
// Eseguire processi sul sistema Linino
#include <Process.h>
// Comunicare con STM32 tramite protocollo I2C
#include <Wire.h>

// auth token dell'applicazione Blynk
char auth[] = "6c86234609c14ae588f238f39a4a7e85";
// indirizzo di lettura/scrittura sulla EEPROM
int addr = 0;
// Se gli hashtag coincidono 1
int match = 0;
int STMCommand = 0;
int start = 0;

// process used to get the date
Process date;                 

// Bottone di Update nell'app
BLYNK_WRITE(V1) {
  if(param.asInt()==1){
    Serial.println("Update dell'hashtag di twitter");
    getHashtag();
  }
}

// Bottone di Check nell'app
BLYNK_WRITE(V2){
  if(param.asInt()==1){
    Serial.println("Controllo se gli hashtag coincidono");
    checkHashtag();
  }
}

String readHashtag(char* filepath,int V)
{
  File hashtag = FileSystem.open(filepath, FILE_READ);
  String str = "";
  while(hashtag.available()>0){
    str = str+char(hashtag.read());
  }
  hashtag.close();
  // Send it to the server
  Blynk.virtualWrite(V, str);
  return str;
}

void checkHashtag(){
  //leggo le due stringhe
  String str1 = readHashtag("/www/example.txt",V5);
  String str2 = readHashtag("/www/trend.txt",V6);
  //controllo se c'è match 
  Serial.println("Ho letto i due file");
  if(str1==str2){
      // stato di Success
      match = 1;
      // nella EEPROM salvo il timestamp dell'ultimo match
      date.begin("/bin/date");
      date.run();
      String data = date.readString();
      EEPROM.put(addr,data);
      Serial.print(EEPROM.get(addr,data));
  }
  else{
    match = 0;
  }
}

void getHashtag(){
  Process python;
  python.runShellCommand("python /www/twy.py");
  while(python.running());
  Serial.println("#Hashtag aggiornato");
  readHashtag("/www/trend.txt",V6);
}

void receiveEvent(int bytes) 
{
    STMCommand = Wire.read();
    Serial.print("Ho ricevuto ");
    Serial.println(STMCommand);
    start = 1; 
    Serial.println("Ho posto start = 1");
}

void requestEvent()
{
    //Restituisce match=1 se c'è stato il match
    Serial.print("Invio match a stm: ");
    Serial.println(match);
    Wire.write(match);
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  // Debug console
  Serial.begin(9600);
  Bridge.begin();
  FileSystem.begin();
  Blynk.begin(auth);
  Wire.begin(OWN_ADDRESS); 

  // Attach a function to trigger when something is received
  Wire.onReceive(receiveEvent);
  // Attach a function to trigger when something is requested
  Wire.onRequest(requestEvent);
  // Setup a function to be called every second
  //timer.setInterval(1000L, sendTemperature);
  Serial.println("Setup Fatto");
}

void loop()
{
  if(start){
    digitalWrite(LED_PIN, HIGH);
  }
  Blynk.run();
}

