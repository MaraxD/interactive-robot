
// Basic demo for accelerometer readings from Adafruit LIS3DH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// getting the MAC address
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <Adafruit_NeoPixel.h>

// light pins
#define LED_PIN 5    // chosen gpio
#define NUM_LEDS 12  // number of leds in the strip

// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
// Used for hardware & software SPI
#define LIS3DH_CS 10

// mac addres of the esp display
uint8_t broadcastAddress[] = { 0x24, 0xDC, 0xC3, 0x45, 0xF2, 0xBC };

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  float x;
  float y;
  float z;
} struct_message;

// Create a struct_message called AccReadings to hold sensor readings
struct_message AccReadings;

esp_now_peer_info_t peerInfo;

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
int initialBrightness=255, flip;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0) {
    success = "Delivery Success :)";
  } else {
    success = "Delivery Fail :(";
  }
}

void readMacAddress() {
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);


  } else {
    Serial.println("Failed to read MAC address");
  }
}


void setup(void) {
  Serial.begin(115200);

  // initialize acc
  while (!Serial) delay(10);  // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("LIS3DH test!");

  Wire.begin(21, 22);      //declaring the sda and scl pins
  if (!lis.begin(0x19)) {  // i2c address
    Serial.println("Couldn't start. Retrying...");
    delay(100);
    if (!lis.begin(0x19)) {
      Serial.println("Failed to initialize LIS3DH. Check wiring.");
      while (1) yield();
    }
  }
  Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_4_G);  // 2, 4, 8 or 16 G!

  Serial.print("Range = ");
  Serial.print(2 << lis.getRange());
  Serial.println("G");

  // lis.setPerformanceMode(LIS3DH_MODE_LOW_POWER);
  Serial.print("Performance mode set to: ");
  switch (lis.getPerformanceMode()) {
    case LIS3DH_MODE_NORMAL: Serial.println("Normal 10bit"); break;
    case LIS3DH_MODE_LOW_POWER: Serial.println("Low Power 8bit"); break;
    case LIS3DH_MODE_HIGH_RESOLUTION: Serial.println("High Resolution 12bit"); break;
  }

  lis.setDataRate(LIS3DH_DATARATE_10_HZ);
  Serial.print("Data rate set to: ");
  switch (lis.getDataRate()) {
    case LIS3DH_DATARATE_1_HZ: Serial.println("1 Hz"); break;
    case LIS3DH_DATARATE_10_HZ: Serial.println("10 Hz"); break;
    case LIS3DH_DATARATE_25_HZ: Serial.println("25 Hz"); break;
    case LIS3DH_DATARATE_50_HZ: Serial.println("50 Hz"); break;
    case LIS3DH_DATARATE_100_HZ: Serial.println("100 Hz"); break;
    case LIS3DH_DATARATE_200_HZ: Serial.println("200 Hz"); break;
    case LIS3DH_DATARATE_400_HZ: Serial.println("400 Hz"); break;

    case LIS3DH_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
    case LIS3DH_DATARATE_LOWPOWER_5KHZ: Serial.println("5 Khz Low Power"); break;
    case LIS3DH_DATARATE_LOWPOWER_1K6HZ: Serial.println("1.6 Khz Low Power"); break;
  }

  // Set device as a Wi-Fi Station

  // Init ESP-NOW
  WiFi.mode(WIFI_STA);  // Set ESP32 to Station mode
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else {
    Serial.print("it s ok");
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // lights
  strip.begin();
  strip.setBrightness(initialBrightness);
  strip.show();  // initialize all pixels to 'off'
}

void loop() {
  /* get a new sensor event, normalized */
  lis.read();
  sensors_event_t event;
  lis.getEvent(&event);

  Serial.print("\t\tX: ");
  Serial.print(event.acceleration.x);
  Serial.print(" \tY: ");
  Serial.print(event.acceleration.y);
  Serial.print(" \tZ: ");
  Serial.print(event.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.println();

  if (event.acceleration.x == 0.0 && event.acceleration.y == 0.0 && event.acceleration.z == 0.0) {
    Serial.println("Invalid data. Reinitializing sensor...");
    lis.begin(0x19);
  }

  //Set values to send
  AccReadings.x = event.acceleration.x;
  AccReadings.y = event.acceleration.y;
  AccReadings.z = event.acceleration.z;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&AccReadings, sizeof(AccReadings));

  // simulate "beating heart"
  strip.setPixelColor(6, strip.Color(255, 0, 0));  // red

  initialBrightness-=10*flip;
  strip.setBrightness(initialBrightness);
  strip.show();

  if(initialBrightness==100){
    flip=-1;
  }else if(initialBrightness==255){
    flip=1;
  }

  delay(100);
}
