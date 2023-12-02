const char* ssid = "ESP";
const char* password = "12345678";
const char* serverAddress = "192.168.1.71";
const int serverPort = 1602;

#define LED_PIN 2
#define Pir 34
void connectToWiFi();

void setup() {
  Serial.begin(115200);
  connectToWiFi();
}

void loop() {
  int espId = 0; 
  int vol = analogRead(Pir); 
  time_t currentTime = time(nullptr);

  
  HTTPClient http;
  http.begin("http://" + String(serverAddress) + ":" + String(serverPort));

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "esp-id=" + String(espId) + "&vol=" + String(vol) + "&time=" + String(currentTime);
  uint32_t httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    
    String response = http.getString();
    Serial.println(response);
    
    
    if (response.indexOf("HTTP/1.1 200 OK") != -1) {
      
       digitalWrite(LED_PIN, HIGH);
    }
  } else {
    Serial.print("HTTP Request failed, error: ");
    Serial.println(httpResponseCode);
    
  }

  http.end();
//   }

  delay(10);
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}