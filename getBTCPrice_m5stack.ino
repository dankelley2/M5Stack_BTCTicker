
using namespace std;
#include "Free_Fonts.h"
#include <WiFi.h>
#include <M5Stack.h>
#include <string>
#include <math.h>
#include <vector>
#include <string>

int lastPrice = 0;
int currentPrice;
int minPrice = 999999999; 
int maxPrice = 0;
char servername[] = "api.coindesk.com"; // Google
String answer;

WiFiClient client;
int status = WL_IDLE_STATUS;            // the Wifi radio's status

char ssid[]     = "SSID";
char password[] = "***********";

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
  Serial.begin(115200);
  delay(100);
  Serial.println(readFile(SD, "/LowHigh.txt").c_str());
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  
  M5.begin();
  M5.lcd.setBrightness(25);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.update();
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
    M5.update();
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
      minPrice = min(minPrice,currentPrice);
      maxPrice = max(maxPrice,currentPrice);
      writeFile(SD, "/LowHigh.txt", ( String(minPrice) + "|" + String(maxPrice)).c_str());
    }
    else{
      minPrice = min(minPrice,currentPrice);
      maxPrice = max(maxPrice,currentPrice);
    }

    /*DRAW
     * 
     */
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setFreeFont(FSSBO9);

    int ypos = 200;
    int fHeight = M5.Lcd.fontHeight(GFXFF);
    
    M5.Lcd.setTextColor(RED);
    String str_minPrice = String(minPrice);
    String str_minPriceDollars = str_minPrice.substring(0,str_minPrice.length()-2);
    String str_minPriceCents = str_minPrice.substring(str_minPrice.length()-2);
    M5.Lcd.drawString("Min:",260,ypos,GFXFF);
    M5.Lcd.drawString("$" + str_minPriceDollars + "." + str_minPriceCents, 260, ypos + fHeight, GFXFF);
        
    M5.Lcd.setTextColor(GREEN);
    String str_maxPrice = String(maxPrice);
    String str_maxPriceDollars = str_maxPrice.substring(0,str_maxPrice.length()-2);
    String str_maxPriceCents   = str_maxPrice.substring(str_maxPrice.length()-2);
    M5.Lcd.drawString("Max:",60,ypos,GFXFF);
    M5.Lcd.drawString("$" + str_maxPriceDollars + "." + str_maxPriceCents, 60, ypos + fHeight, GFXFF);
    
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setFreeFont(FSSBO18);
    M5.Lcd.drawString("BTC Price", 160, 60, GFXFF);
    M5.Lcd.setFreeFont(FSSBO24);
    M5.Lcd.drawString(Amount, 160, 120, GFXFF);

    if (currentPrice >= lastPrice) //UP
    {
      M5.Lcd.fillTriangle(140, 205, 180, 205, 160, 180, GREEN);
    }
    else if (currentPrice < lastPrice) //Down
    {
      M5.Lcd.fillTriangle(140, 205, 180, 205, 160, 230, RED);
    }
    
    lastPrice = currentPrice;
    
    // delay 10 seconds
    for (int i = 0; i < 30; i++){
      if(M5.BtnA.wasPressed()) {
        M5.lcd.setBrightness(0);
      }
      if(M5.BtnB.wasPressed()) {
        M5.lcd.setBrightness(25);
      }
      if(M5.BtnC.wasPressed()) {
        M5.lcd.setBrightness(150);
      }
      M5.update();
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
        writeFile(SD, "/LowHigh.txt", ( String(minPrice) + "|" + String(maxPrice)).c_str());
        return "";
    }
    
    String stringbuilder = "";
    while(file.available()){
        char ch = file.read();
        stringbuilder += String(ch);
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
