#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Use 0x3C as the common I2C address for SH1106
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println(F("GPS Initializing..."));
    display.display();
}

void loop() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("GPS Data:"));
    
    if (gps.location.isValid()) {
        display.print(F("Lat: "));
        display.println(gps.location.lat(), 6);
        display.print(F("Lng: "));
        display.println(gps.location.lng(), 6);
    } else {
        display.println(F("No Signal"));
    }
    
    display.display();
    delay(1000);
}
