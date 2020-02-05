#include "Image.c" //calls opennode logo

#include <M5Stack.h> 
#include <WiFiClientSecure.h>
#include <ArduinoJson.h> 

//enter your wifi details
char wifiSSID[] = "YOUR-SSID";
char wifiPASS[] = "YOUR-PASS";

//LNBITS DETAILS
int httpsPort = 443;
const char* lnbitshost = "lnbits.com"; //change if using a different LNbits instance
String invoicekey = "YOUR-API-KEY"; 

String amount = "100";

String description = "Arcade";
String data_lightning_invoice_payreq;
String data_id;
String data_status = "unpaid";
int counta = 0;
String hints = "false"; 
String payreq;

void setup() {
  M5.begin();
  M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)lnbits_map);
  
  WiFi.begin(wifiSSID, wifiPASS);   
  while (WiFi.status() != WL_CONNECTED) {
  }
  
  pinMode(21, OUTPUT);
}

void loop() {

  fetchpayment();
   page_qrdisplay(payreq);
  
  checkpayment();
  while (counta < 1000){
    if (data_status == "unpaid"){
      delay(300);
      checkpayment();
      counta++;
    }
    else{     

      digitalWrite(21, HIGH);
      delay(300);
      digitalWrite(21, LOW);
      delay(300);
      counta = 1000;
    }  
  }
  counta = 0;

}

////////////////////////// GET/POST REQUESTS///////////////////////////////

void fetchpayment(){

 WiFiClientSecure client;
  
  if (!client.connect(lnbitshost, httpsPort)) {
    return;
  }
  
  String topost = "{  \"value\" : \"" + amount +"\", \"memo\" :\""+ description + String(random(1,1000)) + "\"}";
  String url = "/api/v1/invoices";
  client.print(String("POST ") + url +" HTTP/1.1\r\n" +
                "Host: " + lnbitshost + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "Grpc-Metadata-macaroon:"+ invoicekey +"\r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n" +
                "Content-Length: " + topost.length() + "\r\n" +
                "\r\n" + 
                topost + "\n");



  while (client.connected()) {
    String line = client.readStringUntil('\n');
   Serial.println(line);
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readString();


  Serial.println(line);
  const size_t capacity = JSON_OBJECT_SIZE(2) + 430;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, line);
  const char* pay_req = doc["pay_req"]; 
  const char* payment_hash = doc["payment_hash"]; 
  payreq = pay_req;
  Serial.println(data_lightning_invoice_payreq);
  data_id = payment_hash;

}




void checkpayment(){

   WiFiClientSecure client;
  
  if (!client.connect(lnbitshost, httpsPort)) {
    return;
  }
  String url = "/api/v1/invoice/";
  client.print(String("GET ") + url + data_id +" HTTP/1.1\r\n" +
                "Host: " + lnbitshost + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "Grpc-Metadata-macaroon:"+ invoicekey +"\r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n\r\n");


   while (client.connected()) {
    String line = client.readStringUntil('\n');
   Serial.println(line);
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readString();
  Serial.println(line);

  const size_t capacity = JSON_OBJECT_SIZE(1) + 100;
  DynamicJsonDocument doc(capacity);
  
  deserializeJson(doc, line);
  
  const char* PAID = doc["PAID"];

  String paidd = PAID;

  if (paidd== "FALSE"){
    data_status = "unpaid";
  }
  else{
    data_status = "paid";
  }


}

void page_qrdisplay(String xxx)
{  

  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.qrcode(payreq,45,0,240,14);
  delay(100);

}
