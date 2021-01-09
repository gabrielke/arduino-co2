
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>

#include <Wire.h>
#include <SoftwareSerial.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "WifiSecrets.h"
#define MH_Z19_RX D7
#define MH_Z19_TX D6

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

SoftwareSerial co2Serial(MH_Z19_RX, MH_Z19_TX); // define MH-Z19

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "hu.pool.ntp.org", 3600, 60000);

boolean connected;

int readCO2()
{

  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  // command to ask for data
  byte response[9]; // for answer

  co2Serial.write(cmd, 9); //request PPM CO2

  // The serial stream can get out of sync. The response starts with 0xff, try to resync.
  while (co2Serial.available() > 0 && (unsigned char)co2Serial.peek() != 0xFF) {
    co2Serial.read();
  }

  memset(response, 0, 9);
  co2Serial.readBytes(response, 9);

  if (response[1] != 0x86)
  {
    Serial.println("Invalid response from co2 sensor!");
    return -1;
  }

  byte crc = 0;
  for (int i = 1; i < 8; i++) {
    crc += response[i];
  }
  crc = 255 - crc + 1;

  if (response[8] == crc) {
    int responseHigh = (int) response[2];
    int responseLow = (int) response[3];
    int ppm = (256 * responseHigh) + responseLow;
    return ppm;
  } else {
    Serial.println("CRC error!");
    return -1;
  }
}


void setup() {
  co2Serial.begin(9600); //Init sensor MH-Z19(14)

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display();

  connected = false;

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("GabrielNet", "access248");
  wifiMulti.addAP("Ixenit", "Petnehazi52");

  // Connect to WiFi network
  display.println();
  display.print("Connecting to ");
  display.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }
  display.println("WiFi connected");
  timeClient.begin();

  display.println(WiFi.localIP());
  display.display();

}

void loop() {

  int ppm = readCO2();
  timeClient.update();

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("ppm: ");
  display.println(ppm);
  display.print(timeClient.getFormattedTime());
  display.display();
  delay(1000);
}
