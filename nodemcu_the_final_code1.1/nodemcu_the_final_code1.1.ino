#include <SPI.h>
#include <MFRC522.h> 
#define SS_PIN D4
#define RST_PIN D3 
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
  PulseOximeter pox;
  uint32_t tsLastReport = 0;
  #define REPORTING_PERIOD_MS 1000
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 

 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
 
double temp_amb;
double temp_obj;
int covtemp=34;

#include <Servo.h>

Servo myservo;  
 int pos;
int x;
#include <ESP8266WiFi.h>
#include "NodeMcu_ESP8266_Gmail_Sender_by_FMT.h"  

#pragma region Globals
//replace the below two lines with your information
const char* ssid = "A71";                      // WIFI network name
const char* password = "mahdi91214";              // WIFI network password
/*--------------------------------------------------*/
uint8_t connection_state = 0;                    
uint16_t reconnect_interval = 10000;             
#pragma endregion Globals

String recipientID;
String subject;
String body;



void CustomInfo(){ // function to set custom settings
  //replace the below three lines with your information
  //recipientID = (String)"recipient@gmail.com"; // enter the email ID of the recipient
  subject = (String)"covid alert "; // enter the subject of the email
  body = (String)"you have covid syndrome.please visit to your nearest hospital and checkup.use mask.stay home and take rest"; // enter the body of the email
  /*--------------------------------------------------------------*/
}

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);  
        Serial.println(nSSID);
    } else {
        WiFi.begin(ssid, password);
        Serial.println(ssid);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.print("Connected to ");
    Serial.print(ssid);
    Serial.println(" successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}


void setup()
{
    Serial.begin(115200);
    
    
    connection_state = WiFiConnect();
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect

 
  pinMode(D0, INPUT);

   myservo.attach(D8);
   myservo.write(0); 
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
    
 mlx.begin();         //Initialize MLX90614
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
 
 // Serial.println("Temperature Sensor
    
     if (!pox.begin()) {
//Serial.println("FAILED");
display.clearDisplay(); display.setTextColor(WHITE); 
display.setTextSize(1);

display.setCursor(0, 0);
display.println("FAILED");
display.display();
for(;;);
} else {
display.clearDisplay(); display.setTextColor(WHITE);
display.setTextSize(1);

display.setCursor(0, 0);
display.println("SUCCESS");
display.display();
//Serial.println("SUCCESS");
}
}
    


void loop()
{
  rfid();
   pox.update();
 
  //Reading room temperature and object temp
  //for reading Fahrenheit values, use
  //mlx.readAmbientTempF() , mlx.readObjectTempF() )
  //temp_amb = mlx.readAmbientTempC();
  
  temp_obj = mlx.readObjectTempC();
 
 if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
  //Serial Monitor
 
 
  display.clearDisplay(); display.setTextColor(WHITE);
  display.setCursor(80,10);  
  display.setTextSize(1);
  display.println("Temp");
  display.setCursor(80,40);
  display.setTextSize(1);
  display.print(temp_obj);
  display.print((char)247);
  display.print("C");


display.setTextSize(1);
display.setCursor(0, 0);
display.println("BPM");
display.setTextSize(1);
display.setCursor(0,16);
display.println(pox.getHeartRate());
 


display.setCursor(0, 30);
display.println("SPO2");
display.setTextSize(1);
display.setCursor(0,45);
display.println(pox.getSpO2());
display.display();


 tsLastReport = millis();
}


}

void rfid() 
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;  
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "5B 0B 0F 1C" ) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("access");
    Serial.println("Orko");
    delay(5000);
     x= digitalRead(D0);
     temp_obj = mlx.readObjectTempC();
     if(temp_obj<=covtemp && x==0)myServo();
     if(temp_obj>covtemp && x==0)sendmail();
   
    
 
  }


  else if (content.substring(1) == "4A 2B 9D 15" ) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println("Indro");
    delay(5000);
     x= digitalRead(D0);
     temp_obj = mlx.readObjectTempC();
      if(temp_obj<=covtemp && x==0)myServo();
      if(temp_obj>covtemp && x==0){
       recipientID = (String)"iproy2111@gmail.com"; 
        sendmail();
      }
   
  }


  else if (content.substring(1) == "54 B8 48 73" ) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println("Mostafizur");
    delay(5000);
    x= digitalRead(D0);
    temp_obj = mlx.readObjectTempC();
     if(temp_obj<=covtemp && x==0)myServo();
     if(temp_obj>covtemp && x==0){
       recipientID = (String)"imostafizurs@gmail.com";
      sendmail();
     }
  
  }


  else if (content.substring(1) == "93 94 0D 2E" ) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println("Mahdi");
    delay(5000);
     x= digitalRead(D0);
     temp_obj = mlx.readObjectTempC();
      if(temp_obj<=covtemp && x==0)myServo();
      if(temp_obj>covtemp && x==0){
        recipientID = (String)"mahdihasanacce51@gmail.com";
        sendmail();
      }
   
  }
 else   {
    Serial.println(" Access denied");
   delay(1000);
  }
 
}


 

void myServo(){
      for (pos = 0; pos <= 100; pos += 1) { 
   
    myservo.write(pos);              
    delay(20);                       
  }
  for (pos = 100; pos >= 0; pos -= 1) { 
    myservo.write(pos);             
    delay(20);                      
  }
}

void sendmail(){
  Gsender *gsender = Gsender::Instance();
    
   //InputInfo(); //inputs receiver's gmail id, subject and body of the email
    CustomInfo(); // uncomment this line if you want to send the email using the custom information which are already set in the code; and comment the above line.
    
    if(gsender->Subject(subject)->Send(recipientID, body)) { // sends the email using a single line function
        Serial.println("Email sent"); // message confirmation
      
          

    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError()); // prints out the exact error if the email wasn't successfully sent
    }
}
