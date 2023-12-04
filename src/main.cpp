#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include "time.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

const char *ssid = "GocChienLuoc";
const char *password = "leuleuleu";
const char *serverIP = "192.168.1.4";
const int serverPort = 8080;
const char *ntpServer = "pool.ntp.org";
unsigned long epochTime;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
#define LED_PIN 2
#define Pir 34
#define NUM 100   // số lưọng mẫu 1 lần gửi
#define TIMER 10  // chu kì lấy mẫu
#define SPLIT "_" // phân cách giữa 2 mẫu

String formattedDate;
String dayStamp;
long timeStamp;
uint8_t Counter = 0;
int Volt = 0;
//     QueueHandle_t WifiQueue,ReadQueue;
SemaphoreHandle_t WifiSemaphore;
HTTPClient http;
String Write="";


unsigned long getTime()
{
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}
void Wifi(void *Para) // send data to sever
{

  uint32_t httpResponseCode;
  if (xSemaphoreTake(WifiSemaphore, portMAX_DELAY) == pdPASS && Counter == NUM)
  {
    int EspID = 1;

    digitalWrite(LED_PIN, HIGH);
   
     WiFiClient client;
    if (client.connect(serverIP, serverPort))
    {
      Serial.println("Connected to server !");

      // add time      
      epochTime = getTime();
     
      Write = Write + "&time=" + String(epochTime);

      // init postMsg
      String postMsg = "POST / HTTP/1.1 \n\n" + Write;
      client.print(postMsg);

      // Wait for the response
      while (client.connected())
      {
        if (client.available())
        {
          String line = client.readStringUntil('\r');
          Serial.println(line);
        }
      }

      // Close the connection
      client.stop();
    }

    Counter = 0;
    
  }

  vTaskDelay(1);
}

void Read(void *Para) //   read data from PIR, build strig " Write" send to  task wifi
{
  TickType_t GetTick = xTaskGetTickCount();
  
  while (1)
  {
    if (Counter == 0)
    {
      Write = "esp-id=0";
      Write = Write + "&vol=";
      Volt = analogRead(Pir);
      Write = Write + String(Volt);
      Counter++;
    }
    Volt = analogRead(Pir);
    Write = Write + SPLIT + String(Volt);
    Counter++;
    if (Counter == NUM)
    {
      
      BaseType_t Interrupt = pdFALSE;
      xSemaphoreGiveFromISR(WifiSemaphore, &Interrupt);
      
    }
    vTaskDelayUntil(&GetTick, TIMER / portTICK_PERIOD_MS);
  }
}
void setup()
{
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..."); // connecting wf
  pinMode(LED_PIN, OUTPUT);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  };
  Serial.println("Connected to WiFi");
  configTime(0, 0, ntpServer);
  pinMode(Pir, INPUT);
  
 
  WifiSemaphore = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(Wifi, "Wifi", 10000, NULL, 2, NULL, 1); // tạo tác vụ wifi gắn vào core 1
  xTaskCreatePinnedToCore(Read, "Read", 10000, NULL, 2, NULL, 0); // tạo tác vụ đọc dữ liệu gắn vào core 0
}
void loop()
{
  
}