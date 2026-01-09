# Interactive Robot
![IMG_2646](https://github.com/user-attachments/assets/a9731166-6473-4f65-b40a-b8d4cbbfbcc2)

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
4. Run the readMacAddress function (found in both scipts), to get the MAC address of each ESP, then assign the opposite values to <b> broadcastAddress </b> variable.

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

## Send messages to the robot
Open the textToESP file, and run the following command:
```bash
pip install pyserial
```
and,
```python
SERIAL_PORT = "/dev/<PORT>" # should be the same as the one in arduino (go to Tools -> Port)
BAUD_RATE = 115200  # must match the ESP baud rate
```
Now run the script and follow the prompt.
<br>

## [Optional] Setting up the local server
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
Now that everything has been configured correctly, plug in both the ESP32s into the laptop and flash the code into both of them. <b>!!! When flashing the code on the main ESP32 board, keep pressing on the BOOT button right when the uploading starts !!!</b>
