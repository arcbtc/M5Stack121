/**
 *  M5Stack121LNPAY
 */

#include "LNPAYSplash.c"
#include <M5Stack.h> 
#include <string.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <math.h>

#include <HTTPClient.h>


#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

//WIFI Setup
char wifiSSID[] = "<your_wifi_ssid>";
char wifiPASS[] = "<your_wifi_pass>";

//API Setup
String api_key = "<api_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/integrations
String wallet_key = "<wi_XXXXX_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/advanced-wallets

//Payment Setup
String memo = "M5 "; //memo suffix, followed by a random number
String nosats = "50";

//Endpoint setup
const char* api_endpoint = "https://lnpay.co/v1";
String invoice_create_endpoint = "/user/wallet/" + wallet_key + "/invoice";
String invoice_check_endpoint = "/user/lntx/"; //append LNTX ID to the end (e.g. /user/lntx/lntx_mfEKSse22)


String lntx_id;
String choice = "";
String on_sub_currency = on_currency.substring(3);

String key_val;
String cntr = "0";
String inputs = "";
int keysdec;
int keyssdec;
float temp;  
String fiat;
float satoshis;
float conversion;
bool settle = false;
String payreq = "";


void setup() {
  M5.begin();
  M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)PAYWSplash_map);
  Wire.begin();

  //connect to local wifi            
  WiFi.begin(wifiSSID, wifiPASS);   
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(i >= 5){
     M5.Lcd.fillScreen(BLACK);
     M5.Lcd.setCursor(55, 80);
     M5.Lcd.setTextSize(2);
     M5.Lcd.setTextColor(TFT_RED);
     M5.Lcd.println("WIFI NOT CONNECTED");
    }
    delay(1000);
    i++;
  }
 
  pinMode(KEYBOARD_INT, INPUT_PULLUP);
  on_rates();
 
}

void loop() {
  page_input();
  cntr = "1";
  while (cntr == "1"){
    M5.update();
      page_processing();
      reqinvoice(nosats);
      page_qrdisplay(payreq);
      checkpaid();
      key_val = "";
      inputs = "";
    } 
}

/**
 * Request an invoice
 */
void reqinvoice(String value){

  String payload;
  HTTPClient http;
  http.begin(api_endpoint + invoice_create_endpoint + "?fields=payment_request,id"); //Getting fancy to response size
  http.addHeader("Content-Type","application/json");
  http.addHeader("X-Api-Key",api_key);
  String toPost = "{  \"num_satoshis\" : " + nosats +", \"memo\" :\""+ memo + String(random(1,1000)) + "\"}";
  int httpCode = http.POST(toPost); //Make the request
  
  if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println(payload);
    }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources

  Serial.println(payload);

  
  const size_t capacity = JSON_OBJECT_SIZE(2) + 500;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload);
  
  const char* payment_request = doc["payment_request"]; 
  const char* id = doc["id"]; 
  payreq = (String) payment_request;
  lntx_id = (String) id;
  Serial.println(payreq);
  Serial.println(lntx_id);
}


void checkpaid(){

     int counta = 0;
     int tempi = 0;
     settle = false;

     while (tempi == 0){
       checkpayment();
       if (settle == false){
          counta ++;
          Serial.print(counta);
          if (counta == 100) {
           tempi = 1;
          }
       } 
       else{
         tempi = 1;
       }
         
     }
}


void checkpayment(){
  String payload;
  HTTPClient http;
  http.begin(api_endpoint + invoice_check_endpoint + lntx_id + "?fields=settled"); //Getting fancy to response size
  http.addHeader("Content-Type","application/json");
  http.addHeader("X-Api-Key",api_key);
  int httpCode = http.GET(); //Make the request
  
  if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println(payload);
    }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources
  
  const size_t capacity = JSON_OBJECT_SIZE(2) + 500;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload);
  
  int settled = doc["settled"]; 
  Serial.println(settled);
  if (settled == 1){
    settle = true;
  }
  else{
    settle = false;
  }
}

void page_qrdisplay(String xxx)
{  
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.qrcode(payreq,45,0,240,14);
  delay(100);
}


void page_processing()
{ 
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(40, 80);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.println("PROCESSING");
}

