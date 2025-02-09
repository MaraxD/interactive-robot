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

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

#define LDR_PIN 34  // light sensor

TFT_eSPI tft = TFT_eSPI();
int pupilX, pupilY, tick, startAngle, endAngle, xOffset;

// Define variables to store incoming readings
float incomingX, incomingY, incomingZ, currentY = 0;

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
  ANGRY,
  POKER,
  DISTRESS,
  HURT,
  EEPY
};
faceExpressions state;

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
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

void drawSpiral() {
  int centerX = SCREEN_WIDTH / 2 + 110;
  int centerY = SCREEN_HEIGHT / 2 - 75;
  float angle = 0;
  float radius = 1;

  while (radius < 35) {  // Stop when reaching screen limit
    int x = centerX + radius * cos(angle);
    int y = centerY + radius * sin(angle);

    tft.drawPixel(x, y, TFT_WHITE);  // Draw pixel at computed position

    angle += 0.1;   // Increase angle for next point
    radius += 0.2;  // Expand radius gradually
  }
}

void drawAnimeArc(int x, int y, int outerR, int innerR, int startAngle, int endAngle, uint16_t color) {
  int x1, y1, x2, y2, x3, y3, x4, y4;
  for (int i = startAngle; i < endAngle; i++) {
    // Outer arc points
    x1 = x + outerR * cos(radians(i));
    y1 = y + outerR * sin(radians(i));
    x2 = x + outerR * cos(radians(i + 1));
    y2 = y + outerR * sin(radians(i + 1));

    // Inner arc points
    x3 = x + innerR * cos(radians(i));
    y3 = y + innerR * sin(radians(i));
    x4 = x + innerR * cos(radians(i + 1));
    y4 = y + innerR * sin(radians(i + 1));

    // Draw filled quad between the inner and outer arc points
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
    tft.fillTriangle(x3, y3, x4, y4, x2, y2, color);
  }
}

void drawFace(faceExpressions expression) {

  // if it s neutral, animate the eyes
  if (expression == NEUTRAL) {
    // TODO refactor
    pupilX = 60;
    pupilY = 20;
    // pupilX += 10;
    // pupilY += 10;
    //eyes
    tft.drawEllipse(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 - pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    // pupilX -= 20;
    tft.drawEllipse(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 + pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    //mouth
    tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 20, 270, 90, TFT_WHITE, TFT_WHITE);
    delay(800);

    //looking to one side
    tft.fillScreen(TFT_BLACK);
    pupilX += 10;
    pupilY += 10;
    //eyes
    tft.drawEllipse(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 - pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    pupilX -= 20;
    tft.drawEllipse(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 + pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    //mouth
    tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 20, 270, 90, TFT_WHITE, TFT_WHITE);
    delay(300);

    // looking to the other side
    pupilX = 60;
    pupilY = 20;
    tft.fillScreen(TFT_BLACK);
    pupilX -= 10;
    pupilY += 10;
    //eyes
    tft.drawEllipse(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 - pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    pupilX += 20;
    tft.drawEllipse(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2 + pupilX, SCREEN_HEIGHT / 2 - pupilY, 25, 25, TFT_WHITE);

    //mouth
    tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 20, 270, 90, TFT_WHITE, TFT_WHITE);
    delay(300);


    if (tick == 10) {
      // blinking
      tft.fillScreen(TFT_BLACK);
      tft.fillRect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);  // left
      tft.fillRect(SCREEN_WIDTH / 2 + 20, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);   // right

      //mouth
      tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 20, 270, 90, TFT_WHITE, TFT_WHITE);
      delay(100);
      tick = 0;
      pupilX = 60;
      pupilY = 20;
    }

  } else if (expression == ANGRY) {
    // anime angry marks 💢
    for (int i = 0; i < 4; i++) {
      int endAngle = i * 90 + 20;      // Start angle shifted for inward arcs
      int startAngle = endAngle + 70;  // Ending angle

      // Draw the arc with thickness
      drawAnimeArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 80, 60, startAngle, endAngle, TFT_WHITE);
    }

    // left eye
    tft.drawLine(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 - 45, SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 300, 130, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 300, 130, TFT_WHITE, TFT_WHITE);  // pupil

    // right eye
    tft.drawLine(SCREEN_WIDTH / 2 + 90, SCREEN_HEIGHT / 2 - 45, SCREEN_WIDTH / 2 + 30, SCREEN_HEIGHT / 2, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 230, 60, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 230, 60, TFT_WHITE, TFT_WHITE);

    // draw the mouth upside down
    tft.drawArc(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 80, 15, 20, 90, 270, TFT_WHITE, TFT_WHITE);
    delay(900);
  } else if (expression == POKER) {
    // left eye
    tft.fillRect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 90, TFT_WHITE, TFT_WHITE);  // pupil

    // right eye
    tft.fillRect(SCREEN_WIDTH / 2 + 20, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 90, TFT_WHITE, TFT_WHITE);

    // draw the mouth in a straight line
    tft.fillRect(SCREEN_WIDTH / 2 - 15, SCREEN_HEIGHT / 2 + 80, 30, 2, TFT_WHITE);  // draw a freaking rectangle in order to have a mouth thicker 😐
  } else if (expression == EEPY) {
    // draw the two eyes closed (maybe this can be reused when adding the blinking animation)
    int font_size = FONT_SIZE;
    for (int i = 0; i < 2; i++) {
      tft.drawString("Z", SCREEN_WIDTH / 2 + 60 + (i * 20), SCREEN_HEIGHT / 2 - 50 - (i * 10), font_size);
      font_size += 2;
    }

    tft.fillRect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);  // left
    tft.fillRect(SCREEN_WIDTH / 2 + 20, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);   // right

    //mouth
    tft.fillEllipse(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 15, 15, TFT_WHITE);
    tft.fillEllipse(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 40, 10, 10, TFT_BLACK);
  } else if (expression == HURT) {
    // spiral
    drawSpiral();

    // left
    delay(150);
    tft.fillRect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 270, 90, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 270, 90, TFT_WHITE, TFT_WHITE);  // pupil

    tft.fillRect(SCREEN_WIDTH / 2 + 20, SCREEN_HEIGHT / 2 - 20, 80, 2, TFT_WHITE);  // right

    // semi squigly mouth
    tft.fillRect(SCREEN_WIDTH / 2 - 15, SCREEN_HEIGHT / 2 + 80, 15, 2, TFT_WHITE);
    for (int i = 0; i < 4; i++) {
      //create 4 arcs, alternating in placement (first is the lower arc, then the upper arc and so on..)
      startAngle = (i % 2 == 0) ? 270 : 90;
      endAngle = (i % 2 == 0) ? 90 : 270;

      xOffset = 5 * (2 * i + 1);

      tft.drawArc(SCREEN_WIDTH / 2 + xOffset, SCREEN_HEIGHT / 2 + 80, 5, 5, startAngle, endAngle, TFT_WHITE, TFT_WHITE);
    }
    delay(500);
  } else if (expression == DISTRESS) {
    int distanceToCenter;
    // same as angry face but opposite (TODO combine them)
    // left eye
    tft.drawLine(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2 - 45, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 300, 130, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 + 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 300, 130, TFT_WHITE, TFT_WHITE);  // pupil

    // right eye
    tft.drawLine(SCREEN_WIDTH / 2 + 90, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 + 30, SCREEN_HEIGHT / 2 - 45, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, 40, 230, 60, TFT_WHITE, TFT_WHITE);
    tft.drawArc(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 25, 10, 230, 60, TFT_WHITE, TFT_WHITE);

    // squigly mouth
    for (int i = 0; i < 4; i++) {
      //create 4 arcs, alternating in placement (first is the lower arc, then the upper arc and so on..)
      startAngle = (i % 2 == 0) ? 270 : 90;
      endAngle = (i % 2 == 0) ? 90 : 270;

      // keep decresing until you reach the middle of the canvas
      xOffset = 5 * (2 * i);

      tft.drawArc(SCREEN_WIDTH / 2 - xOffset, SCREEN_HEIGHT / 2 + 80, 5, 5, startAngle, endAngle, TFT_WHITE, TFT_WHITE);
      tft.drawArc(SCREEN_WIDTH / 2 + xOffset, SCREEN_HEIGHT / 2 + 80, 5, 5, startAngle, endAngle, TFT_WHITE, TFT_WHITE);
    }
    delay(800);
  }
}

void setup() {
  Serial.begin(115200);

  // Start the tft display
  tft.init();
  // Set the TFT display rotation in landscape mode
  tft.setRotation(1);

  tft.invertDisplay(1);  //invert the display at code level
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Set X and Y coordinates for center of display
  // int centerX = SCREEN_WIDTH / 2;
  // int centerY = SCREEN_HEIGHT / 2;

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
  Serial.print("x values:");
  Serial.println(incomingReadings.x);

  Serial.print("y values:");
  Serial.println(incomingReadings.y);


  tft.fillScreen(TFT_BLACK);


  if (incomingReadings.x < 8.50) {
    state = EEPY;
  } else {
    state = NEUTRAL;
    tick++;  // for the animation
  }

  // if (abs(incomingReadings.z) > 10) {
  //   state = HURT;
  // } else
  if (int(analogRead(LDR_PIN) > 300)) {
    state = DISTRESS;
  }

  // 12 si -6 when laying flat
  // 1 si 5 when up (sa nu se confunde cu awake)

  //pokerface -> when saying a dad joke
  //angry ->

  drawFace(DISTRESS);

  delay(100);
}