import serial
import time

SERIAL_PORT = "/dev/cu.usbserial-1413430"

BAUD_RATE = 115200  # must match the ESP baud rate
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # wait for ESP to initialize

try:
    while True:
        choice = input("Do you want to tell a dad joke or insult him? ")
        message=f"{choice}:{input()}"
        ser.write(message.encode())  # send as bytes
        print(f"Sent: {message}")
except KeyboardInterrupt:
    print("\nClosing connection.")
    ser.close()
