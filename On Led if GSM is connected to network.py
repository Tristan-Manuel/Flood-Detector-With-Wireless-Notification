from  machine import Pin, UART
import time

# Initialize the UART connection with the SIM800L module
gsm = UART(2, 9600)
gsm.init(9600, bits=8, parity=None, stop=1, tx=17, rx=16)

# Initialize the LED
led_Network_pin = 22 # Change this to the GPIO pin where your LED is connected
led_Network_pin = Pin(led_Network_pin, Pin.OUT)

# Check the network registration status of the SIM800L module
gsm.write("AT+CREG?\r\n")
time.sleep(1)
response = gsm.read()
if b'+CREG: 0,1' in response or b'+CREG: 0,5' in response:
    # Network registration successful or already registered
    led_Network_pin.on()
else:
    # Network registration failed or not registered
    led_Network_pin.off()

# Continuously check the network registration status and update the LED
while True:
    gsm.write("AT+CREG?\r\n")
    time.sleep(1)
    response = gsm.read()
    if b'+CREG: 0,1' in response or b'+CREG: 0,5' in response:
        # Network registration successful or already registered
        led_Network_pin.on()
    else:
        # Network registration failed or not registered
        led_Network_pin.off()
    time.sleep(1)
