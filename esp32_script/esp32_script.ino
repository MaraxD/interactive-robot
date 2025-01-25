/*  Rui Santos & Sara Santos - Random Nerd Tutorials
    THIS EXAMPLE WAS TESTED WITH THE FOLLOWING HARDWARE:
    1) ESP32-2432S028R 2.8 inch 240×320 also known as the Cheap Yellow Display (CYD): https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/cyd/
    2) REGULAR ESP32 Dev Board + 2.8 inch 240x320 TFT Display: https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/ and https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/esp32-tft/
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <SPI.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd/ or https://RandomNerdTutorials.com/esp32-tft/   */
#include <TFT_eSPI.h>

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
// Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z, eyeMoves, pupilX, pupilY, tick;
int startAngle, endAngle;

// Define variables to store incoming readings
float incomingX;
float incomingY;
float incomingZ;

// mac addres of the esp w/acc
uint8_t broadcastAddress[] = { 0xD8, 0x3B, 0xDA, 0x99, 0x61, 0xAC };

typedef struct struct_message {
  float x;
  float y;
  float z;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message AccReadings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

enum faceExpressions {
  NEUTRAL,
  HAPPY,
  ANGRY,
  POKER,
  DISTRESS,
  HURT,
  EEPY
};

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingX = incomingReadings.x;
  incomingY = incomingReadings.y;
  incomingZ = incomingReadings.z;
}

void readMacAddress() {
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

// Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
void printTouchToSerial(int touchX, int touchY, int touchZ) {
  Serial.print("X = ");
  Serial.print(touchX);
  Serial.print(" | Y = ");
  Serial.print(touchY);
  Serial.print(" | Pressure = ");
  Serial.print(touchZ);
  Serial.println();
}

// Print Touchscreen info about X, Y and Pressure (Z) on the TFT Display
void printTouchToDisplay(int touchX, int touchY, int touchZ) {
  // Clear TFT screen
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  int centerX = SCREEN_WIDTH / 2;
  int textY = 80;

  String tempText = "X = " + String(touchX);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  textY += 20;
  tempText = "Y = " + String(touchY);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  textY += 20;
  tempText = "Pressure = " + String(touchZ);
  tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);
}

void drawFace(faceExpressions expression) {

  // if it s neutral, animate the eyes
  if (expression == NEUTRAL && tick == 10) {
    // TODO refactor
    pupilX += 10;
    pupilY += 10;
    //eyes
    tft.drawEllipse(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 - pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    pupilX -= 20;
    tft.drawEllipse(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 + pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    tick = 0;

    //mouth
    tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 20, 270, 90, TFT_WHITE, TFT_WHITE);
  } else if (expression == ANGRY) {
    // left eye
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 130, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 130, TFT_WHITE, TFT_WHITE);  // pupil

    // right eye
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 230, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 230, 90, TFT_WHITE, TFT_WHITE);

    // draw the mouth upside down
    tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 80, 15, 20, 90, 270, TFT_WHITE, TFT_WHITE);
  } else if (expression == POKER) {
    // left eye
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 90, TFT_WHITE, TFT_WHITE);  // pupil

    // right eye
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 90, TFT_WHITE, TFT_WHITE);

    // draw the mouth in a straight line
    tft.fillRect(SCREEN_WIDTH / 2 - 15, SCREEN_HEIGHT / 2 + 80, 30, 2, TFT_WHITE);  // draw a freaking rectangle in order to have a mouth thicker 😐
  } else if (expression == EEPY) {
    // draw the two eyes closed (maybe this can be reused when adding the blinking animation)
    tft.fillRect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);  // left
    tft.fillRect(SCREEN_WIDTH / 2 + 20, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);   // right

    //mouth
    tft.fillEllipse(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 15, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 10, 10, TFT_BLACK);
  } else if (expression == HURT) {
    // left
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 90, TFT_WHITE, TFT_WHITE);  // pupil

    tft.fillRect(SCREEN_WIDTH / 2 + 20, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);  // right

    // semi squigly mouth
    tft.fillRect(SCREEN_WIDTH / 2 - 15, SCREEN_HEIGHT / 2 + 80, 15, 2, TFT_WHITE);
    for (int i = 0; i < 4; i++) {
      //create 4 arcs, alternating in placement (first is the lower arc, then the upper arc and so on..)
      startAngle = 270;
      endAngle = 90;

      if (i % 2 != 0) {
        startAngle = 90;
        endAngle = 270;
      }

      tft.drawArc(SCREEN_WIDTH / 2 + 5 * (2 * i + 1), SCREEN_HEIGHT / 2 + 80, 5, 5, startAngle, endAngle, TFT_WHITE, TFT_WHITE);
    }
  } else if (expression == DISTRESS) {
    // same as angry face but opposite (TODO combine them)
    // left eye
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 130, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 130, TFT_WHITE, TFT_WHITE);  // pupil

    // right eye
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 230, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 230, 90, TFT_WHITE, TFT_WHITE);

    // squigly mouth
    for (int i = 0; i < 6; i++) {
      //create 4 arcs, alternating in placement (first is the lower arc, then the upper arc and so on..)
      startAngle = 270;
      endAngle = 90;

      if (i % 2 != 0) {
        startAngle = 90;
        endAngle = 270;
      }

      // keep decresing until you reach the middle of the canvas
      if(SCREEN_WIDTH / 2 - 15 < SCREEN_WIDTH){
        tft.drawArc(SCREEN_WIDTH / 2 - 15 * (2 * i + 1), SCREEN_HEIGHT / 2 + 80, 5, 5, startAngle, endAngle, TFT_WHITE, TFT_WHITE);
      }
      tft.drawArc(SCREEN_WIDTH / 2 - 15 * (2 * i + 1), SCREEN_HEIGHT / 2 + 80, 5, 5, startAngle, endAngle, TFT_WHITE, TFT_WHITE);
    }
  } else {
    // reset the eyes
    pupilX = 60;
    pupilY = 20;
    //eyes
    tft.drawEllipse(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 - pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    tft.drawEllipse(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 + pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    //mouth
    tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 20, 270, 90, TFT_WHITE, TFT_WHITE);
  }
}

void setup() {
  Serial.begin(115200);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 3: touchscreen.setRotation(3);
  touchscreen.setRotation(1);

  // Start the tft display
  tft.init();
  // Set the TFT display rotation in landscape mode
  tft.setRotation(1);

  tft.invertDisplay(1);  //invert the display at code level
  // Clear the screen before writing to it
  //tft.fillScreen(TFT_BLACK);
  eyeMoves = 0;
  // tft.setTextColor(TFT_BLACK, TFT_WHITE);

  // Set X and Y coordinates for center of display
  // int centerX = SCREEN_WIDTH / 2;
  // int centerY = SCREEN_HEIGHT / 2;

  // tft.drawCentreString("Touch screen to test", centerX, centerY, FONT_SIZE);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // init variables
  tick = 0;
}


void loop() {
  Serial.println("incoming readings:");
  Serial.print(incomingReadings.x);
  tft.fillScreen(TFT_BLACK);
  int i = 0;

  drawFace(DISTRESS);
  tick++;

  delay(100);
}