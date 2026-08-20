/***************  BLYNK SETTINGS  ****************/
#define BLYNK_TEMPLATE_ID "TMPL3f2ivQe6J"
#define BLYNK_TEMPLATE_NAME "RADAR ALERT"
#define BLYNK_AUTH_TOKEN "Js9HLcNGiIRkctB_34AZ-QODO7JVk4py"

/***************  LIBRARIES  ********************/
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>

/***************  WIFI LOGIN  *******************/
char ssid[] = "RADAR";
char pass[] = "234567890";

/***************  SERIAL SETTINGS  **************/
HardwareSerial UNO(2);   // UART2 -> RX=16, TX=17

unsigned long lastNotifyTime = 0;
const unsigned long notifyDelay = 2000; // 2 seconds

/***************  TIME SETTINGS  ****************/
const char* ntpServer = "pool.ntp.org";
long gmtOffset_sec     = 19800;  // IST (UTC+5:30)
int  daylightOffset_sec = 0;

/*************************************************/

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "TIME_ERR";
  }

  char timeStr[25];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStr);
}

void setup() {
  Serial.begin(115200);
  UNO.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("\nConnecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("ESP32 Connected!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time Sync Done");
}

void loop() {
  Blynk.run();

  if (UNO.available()) {

    String data = UNO.readStringUntil('\n');
    data.trim();
    if (data.length() == 0) return;

    Serial.println("Received: " + data);

    int aIndex = data.indexOf("A:");
    int dIndex = data.indexOf(",D:");

    if (aIndex < 0 || dIndex < 0) {
      Serial.println("❌ Invalid Data");
      return;
    }

    String angle = data.substring(aIndex + 2, dIndex);
    String distance = data.substring(dIndex + 3);

    int distValue = distance.toInt();

    // 🚨 Only notify on object detection (0–20 cm)
    if (distValue > 0 && distValue <= 20) {

      unsigned long now = millis();

      if (now - lastNotifyTime >= notifyDelay) {

        String timestamp = getTimeString();

        String message =
          "🚨 RADAR ALERT\n"
          "Angle: " + angle + "°\n"
          "Distance: " + distance + " cm\n"
          "Time: " + timestamp;

        Blynk.logEvent("radar_alert", message);
        Serial.println("✔ Notification Sent:\n" + message);

        lastNotifyTime = now;
      }
    }
  }
}  
