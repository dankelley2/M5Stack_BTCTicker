#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <string>
#include <math.h>
#include <utility\Fonts\FreeMonoBold24pt7b.h>
#include <utility\Fonts\FreeSans9pt7b.h>
#include <vector>
#include <string>
using namespace std;
WiFiMulti WiFiMulti;

int status = WL_IDLE_STATUS;
int lastPrice = 0;
int currentPrice;
int minPrice = 9999999; 
int maxPrice = 0;
char servername[] = "api.coindesk.com"; // Google
String answer;
WiFiClient client;

vector<string> split(const char *str, char c = '|')
{
  vector<string> result;
    do
    {
        const char *begin = str;
        while(*str != c && *str)
            str++;
        result.push_back(string(begin, str));
    } while (0 != *str++);
    return result;
}


void setup() {
  WiFiMulti.addAP("SSID", "PASSWORD");
  Serial.begin(115200);
  m5.begin();
  m5.lcd.setBrightness(25);
  m5.update();

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
  }

  vector<string> LowHigh = split(readFile(SD, "/LowHigh.txt").c_str());
  String low = String(LowHigh.at(0).c_str());
  String high = String(LowHigh.at(1).c_str());
  minPrice = low.toInt();
  maxPrice = high.toInt();
  
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
    String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 19);
    Serial.println(jsonAnswer);
    Serial.println("");
    Serial.println(priceString);
    priceString.trim();
    int decimalplace = priceString.indexOf(".");
    String Dollars = priceString.substring(0, decimalplace);
    String Cents = priceString.substring(decimalplace+1);
    while (Cents.length() < 2) {
      Cents += "0";
    }
    String Amount = "$" + Dollars + "." + Cents;

    currentPrice = (Dollars + Cents).toInt();
    if (currentPrice < minPrice || currentPrice > maxPrice ){
      writeFile(SD, "/LowHigh.txt", (String(minPrice) + "|" + String(maxPrice)).c_str());
    }
    minPrice = std::min(minPrice,currentPrice);
    maxPrice = std::max(maxPrice,currentPrice);
    
    m5.Lcd.fillScreen(0x0000);
    m5.Lcd.setFont(&FreeSans9pt7b);
    
    m5.Lcd.setTextColor(RED);
    m5.Lcd.setCursor(20, 20);
    m5.Lcd.printf(("Min: " + String(minPrice).substring(0,Dollars.length()) + "." + String(minPrice).substring(Dollars.length())).c_str());
    
    m5.Lcd.setTextColor(GREEN);
    m5.Lcd.setCursor(205, 20);
    m5.Lcd.printf(("Max: " + String(maxPrice).substring(0,Dollars.length()) + "." + String(maxPrice).substring(Dollars.length())).c_str());

    m5.Lcd.setTextColor(WHITE);
    m5.Lcd.setCursor(30, 80);
    m5.Lcd.setFont(&FreeMonoBold24pt7b);
    m5.Lcd.printf("BTC Price");
    m5.Lcd.printf("\r\n");
    
    m5.Lcd.setCursor(50, 140);
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
    
    // delay 10 seconds
    for (int i = 0; i < 30; i++){
      if(M5.BtnA.wasPressed()) {
        m5.lcd.setBrightness(0);
      }
      if(M5.BtnB.wasPressed()) {
        m5.lcd.setBrightness(25);
      }
      if(M5.BtnC.wasPressed()) {
        m5.lcd.setBrightness(150);
      }
      m5.update();
      delay(1000);
    }
    answer = "";
    Amount = "";
    currentPrice = 0;
    ConnectToClient();
  }
}


String readFile(fs::FS &fs, const char * path) {

    File file = fs.open(path);
    if(!file){
        return "";
    }
    
    String stringbuilder = "";
    while(file.available()){
        char ch = file.read();
        stringbuilder += String(ch);
        Serial.println(stringbuilder);
    }
    return stringbuilder;
}

void writeFile(fs::FS &fs, const char * path, const char * message){

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        return;
    }
    if(file.print(message)){
      Serial.println("new record logged");
    } else {
      Serial.println("Error writing record");
    }
}
