from time import sleep
import serial

# Connect to Arduino
# On MacOS, find the port by "ls /dev/*" in terminal. Then, copy in /dev/tty.usbmodem or /dev/tty.usbserial.
dev = serial.Serial(port="/dev/tty.usbmodem142401", baudrate=9600)

print("Starting\n")
sleep(1)
running = True

while True:
    print("1: Turn LEDs On/Off\n"
          "2: Reactive\n"
          "3: Sparkle\n"
          "4: Juggle\n"
          "5: Rainbow")
    command = input("Choose a mode: ")
    dev.write(command.encode("ascii"))
    print()