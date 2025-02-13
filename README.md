# Interactive Robot

## Materials used
<ul>
  <li>
    "CYD" ESP32 board
  </li>
  <li>
    ESP32 board
  </li>
  <li>
    LIS3DH Accelerometer
  </li>
  <li>
    CJMCU-2812B-12 LED strip
  </li>
</ul>

## Prerequisites
1. Clone this repository and make sure you have Arduino installed
2. Install Adafruit LIS3DH and Adafruit NeoPixel (for the sensors), TFT_eSPI and XPT2046_Touchscreen (for drawing on the display) from arduino's library manager
3. In order for the TST_eSPI library to work, you need to prepare User_Setup.h config file:
<br>    -> preparing the config file for Windows PC (https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/#config-file-windows-pc)
<br>    -> preparing the config file for Mac OS (https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/#config-file-mac-os)

## Wiring up LIS3DH to the main ESP32 board
LIS3DH | ESP32 | Info | 
--- | --- | --- 
VCC | 3.3V | Power - you can also use 5V 
GND | GND | Ground 
SCL | 22 | Serial Clock 
SDA | 21 | Serial Data 

## Wiring up CJMCU-2812B-12 to the main ESP32 board
LIS3DH | ESP32 | Info 
--- | --- | --- 
VCC | 5V | Power - always use 5V!!! | 
GND | GND | Ground |
DO | 5 | Data Out - any digital output pin can be used |

## Setting up the local server
Run the following command,
```bash
 flask --app main run --host=0.0.0.0
```
now the server is externaly visible.

Go to esp32_script file, add the server's ip address and your wifi credentials:
```cpp
String localServerName = <YOUR_SERVER_IP_ADRESS>;
const char* ssid = <YOUR_WIFI_NAME>;
const char* password = <YOUR_WIFI_PASSWORD>;  //
```
<br>
Now that everything has been configured correctly, plug in both the ESP32s into the laptop and flash the code into both of them. **!!! When flashing the code on the main ESP32 board, keep pressing on the BOOT button right when the uploading starts !!!**
