#include <SoftwareSerial.h>
#include <avr/wdt.h>

#include <OneWire.h> // для термометра
#include <DallasTemperature.h> //для термометра

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 12

#define NetLightPin9 9
#define NetLightPin2 2
 
#define GSM_TXPIN 5
#define GSM_RXPIN 4  
#define GSM_POW_PIN 6

SoftwareSerial gprsSerial(GSM_TXPIN, GSM_RXPIN); // RX, TX

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// arrays to hold device addresses
DeviceAddress insideThermometer;

volatile unsigned long time_imp = 0;     //Время между морганиями
volatile unsigned long time_old = 0;     //Предыдущее время моргания

const char* host = "www.api.thingspeak.com";
const char* privateKey = "MUEB68DVHDGF5CMV";
 
int Rele = 10;

boolean Led_on= true;
boolean webasto_on= false;
int error_count = 0;

void(* resetFunc) (void) = 0; // Reset (идем по адресу 0. состояния регистров сохраняются)

void interruptFunction() {
  digitalWrite(13, HIGH); // вкл
  time_imp = micros()-time_old;
  time_old = micros();  
  wdt_reset();
  digitalWrite(13, LOW); // отключаем
}

void setup()
{
  wdt_disable();
  Serial.begin(9200);             // the Serial port of Arduino baud rate. 
  Serial.println("--------------");
  Serial.println("|START Module|");
  Serial.println("--------------");
  //Пропишем опрос моргания диода
  pinMode(NetLightPin2, INPUT);  // Configure the pin as an input
  pinMode(NetLightPin9, INPUT);  // Configure the pin as an input
  attachInterrupt(0, interruptFunction, RISING); //Пропишем прерывания от NetLight
  
  pinMode(GSM_POW_PIN, OUTPUT); // Котакт питания
  powerUpOrDown();
  wdt_enable(WDTO_8S);  
  digitalWrite(Rele, LOW); // отключаем
  pinMode(Rele, OUTPUT);
  pinMode(13, OUTPUT);   // светодиод ардуины
  
  gprsSerial.begin(9200);         // the GPRS baud rate      

  sensors.begin();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();
  // set the resolution to 12 bit
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();

  

  
     
  delay(10000); //Время на поиск сети      
  Serial.println("");
  Serial.println("Init");
  InitHttp(); //Инициализация APN
}
 

void loop()
{
  SubmitHttpRequest();
  if (gprsSerial.available()) Serial.write(gprsSerial.read());
  if (error_count>10)
  {
    powerUpOrDown();
    resetFunc(); //вызов ресет        
  }

  Serial.print("NetLight period:");
  Serial.println(time_imp);
  sendTemperature();
}




void SubmitHttpRequest()
{
  int command=-1;
  gprsSerial.println("AT+HTTPINIT"); //init the HTTP request 
  delay(2000); 
  ShowSerialData();
  
  if (webasto_on) gprsSerial.println("AT+HTTPPARA=\"URL\",\"www.zaboy.somee.com/?pas=dron&status=on\"");// Сам запрос если вебаста работает
  else gprsSerial.println("AT+HTTPPARA=\"URL\",\"www.zaboy.somee.com/?pas=dron&status=off\"");// Сам запрос если вебаста отключена
  
  delay(1000);
  ShowSerialData();
 
  gprsSerial.println("AT+HTTPACTION=0");//Отправляем запрос
  while(!gprsSerial.available()); //ждем ответ
  if (WaitOkData()==1)
  {
    delay(1000);
    ShowSerialData();
    gprsSerial.println("AT+HTTPREAD");// читаем данные
    command=Wite_On_Off();

    switch(command)
    {
      case 1:
        Serial.println("ON");
        digitalWrite(Rele, HIGH); //включаем
        webasto_on=true;
        break;
      case 0:
        Serial.println("OFF");
        digitalWrite(Rele, LOW); //отключаем
        webasto_on=false;
        break;
      case -1:
        Serial.println("TimeOut");
        break;        
    }    
    gprsSerial.println("AT+HTTPTERM"); //закрываем http
    delay(2000); 
    ShowSerialData();
    gprsSerial.println("");
    delay(100);
  }
  else
  {
    error_count+=1;
  }
  
  
}


void InitHttp()
{
  gprsSerial.println("AT+CSQ");
  delay(100); 
  ShowSerialData();// this code is to show the data from gprs shield, in order to easily see the process of how the gprs shield submit a http request, and the following is for this purpose too. 
  gprsSerial.println("AT+CGATT?");
  delay(100); 
  ShowSerialData(); 
  gprsSerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR, the connection type is using gprs
  delay(1000); 
  ShowSerialData(); 
  gprsSerial.println("AT+SAPBR=3,1,\"APN\",\"mts\"");//setting the APN, the second need you fill in your local apn server
  gprsSerial.println("AT+SAPBR=3,1,\"USER\",\"mts\"");//setting the APN, the second need you fill in your local apn server
  gprsSerial.println("AT+SAPBR=3,1,\"PWD\",\"mts\"");//setting the APN, the second need you fill in your local apn server
  delay(4000); 
  ShowSerialData(); 
  gprsSerial.println("AT+SAPBR=1,1");//setting the SAPBR, for detail you can refer to the AT command mamual
  delay(2000); 
  ShowSerialData();
}

void ShowSerialData()
{
  while(gprsSerial.available()!=0)
    Serial.write(gprsSerial.read());
}


int WaitOkData()
{
  String str = "";
  char myChar = 'A';
  int timeout=60;
  
  do
  {
    while(gprsSerial.available()!=0)
    {
      myChar=gprsSerial.read();
      str = str + myChar;
    }
    delay(1000); 
    timeout-=1;
    if (timeout==0) return -1;
  } while (str.indexOf("HTTPACTION:0,200")==-1);
  return 1; 
    
}

int Wite_On_Off()
{
  String str2 = "";
  char myChar = 'A';
  int timeout=60;
  
  do
  {
    while(gprsSerial.available()!=0)
    {
      myChar=gprsSerial.read();
      str2 = str2 + myChar;
    }
    delay(1000); 
    timeout-=1;
    if (timeout==0) return -1;
  } while (str2.indexOf("webasto_on")==-1 && str2.indexOf("webasto_off")==-1);
  if (str2.indexOf("webasto_on")!=-1)  return 1; 
  if (str2.indexOf("webasto_off")!=-1)  return 0; 
  return -1;    
}


void powerUpOrDown()
{   
  digitalWrite(GSM_POW_PIN,LOW);
  delay(1000);
  digitalWrite(GSM_POW_PIN,HIGH);
  delay(2000);
  digitalWrite(GSM_POW_PIN,LOW);
  delay(3000);
}

// Функции термометра
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();    
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

void sendTemperature() {

  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");
  // print the device information
  float tempC = sensors.getTempC(insideThermometer);

  Serial.print("connecting to ");
  Serial.println(host);

  // We now create a URI for the request
  String url = host;
  url += "/update";
  url += "?key=";
  url += privateKey;
  url += "&field1=";
  url += tempC;  


  gprsSerial.println("AT+HTTPINIT"); //init the HTTP request 
  delay(2000); 
  ShowSerialData();
  gprsSerial.print("AT+HTTPPARA=\"URL\",\"");// начало запроса
  gprsSerial.print(url);// тело запроса
  gprsSerial.println("\"");// конец
  
  delay(1000);
  ShowSerialData();
  gprsSerial.println("AT+HTTPACTION=0");//Отправляем запрос
  while(!gprsSerial.available()); //ждем ответ
  ShowSerialData();

  delay(10); 
  
}


