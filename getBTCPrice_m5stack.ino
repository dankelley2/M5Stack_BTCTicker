#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <string>
#include <math.h>
#include <utility\Fonts\FreeMonoBold24pt7b.h>
#include <utility\Fonts\FreeSans9pt7b.h>
WiFiMulti WiFiMulti;


int status = WL_IDLE_STATUS;
int lastPrice = 0;
int currentPrice;
int minPrice = 99999; 
int maxPrice = 0;
char servername[] = "api.coindesk.com"; // Google
String answer;

WiFiClient client;

void setup() {
  WiFiMulti.addAP("WIFI SSID", "wifi_password");

  m5.begin();
  m5.lcd.setBrightness(25);
  m5.update();

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
  }
  ConnectToClient();
}

void ConnectToClient() {
  if (client.connect(servername, 80)) {
    // Make a HTTP request:
    client.print(String("GET ") + "/v1/bpi/currentprice.json HTTP/1.1\r\n" +
                 "Host: api.coindesk.com\r\n" +
                 "Connection: close\r\n\r\n");
  }
}


void loop() {

  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    answer += c;
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    m5.update();
    client.stop();

    String jsonAnswer;
    int jsonIndex;

    for (int i = 0; i < answer.length(); i++) {
      if (answer[i] == '{') {
        jsonIndex = i;
        break;
      }
    }

    jsonAnswer = answer.substring(jsonIndex);
    jsonAnswer.trim();

    int rateIndex = jsonAnswer.indexOf("rate_float");
    String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 18);
    priceString.trim();
    int decimalplace = priceString.indexOf(".");
    String Dollars = priceString.substring(0, decimalplace);
    String Cents = priceString.substring(decimalplace+1);
    String Amount = "$" + Dollars + "." + Cents;

    currentPrice = Dollars.toInt();
    minPrice = std::min(minPrice,currentPrice);
    maxPrice = std::max(maxPrice,currentPrice);
    
    m5.Lcd.fillScreen(0x0000);
    m5.Lcd.setFont(&FreeSans9pt7b);
    
    m5.Lcd.setTextColor(RED);
    m5.Lcd.setCursor(20, 20);
    m5.Lcd.printf(("Min: " + String(minPrice)).c_str());
    
    m5.Lcd.setTextColor(GREEN);
    m5.Lcd.setCursor(210, 20);
    m5.Lcd.printf(("Max: " + String(maxPrice)).c_str());

    m5.Lcd.setTextColor(WHITE);
    m5.Lcd.setCursor(30, 80);
    m5.Lcd.setFont(&FreeMonoBold24pt7b);
    m5.Lcd.printf("BTC Price");
    m5.Lcd.printf("\r\n");
    
    m5.Lcd.setCursor(60, 140);
    m5.Lcd.printf(Amount.c_str());

    if (currentPrice >= lastPrice) //UP
    {
      m5.Lcd.fillTriangle(140, 205, 180, 205, 160, 180, GREEN);
    }
    else if (currentPrice < lastPrice) //Down
    {
      m5.Lcd.fillTriangle(140, 205, 180, 205, 160, 230, RED);
    }
    
    lastPrice = currentPrice;
    
    // delay 60 seconds
    for (int i = 0; i < 60; i++){
      if(M5.BtnA.wasPressed()) {
        m5.lcd.setBrightness(0);
      }
      if(M5.BtnB.wasPressed()) {
        m5.lcd.setBrightness(50);
      }
      if(M5.BtnC.wasPressed()) {
        m5.lcd.setBrightness(255);
      }
      m5.update();
      delay(1000);
    }
    ConnectToClient();
  }
}
