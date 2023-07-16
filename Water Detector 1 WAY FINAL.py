from machine import Pin, UART
import time

# Configure GPIO pins for each float switch
switch_1_pin = 25
switch_2_pin = 26
switch_3_pin = 27

# Configure GPIO pins for each LED
led_1_pin = 18
led_2_pin = 19
led_3_pin = 21
led_Network_pin = 22

# Set up the GPIO pins for the float switches
switch_1 = Pin(switch_1_pin, Pin.IN, Pin.PULL_UP)
switch_2 = Pin(switch_2_pin, Pin.IN, Pin.PULL_UP)
switch_3 = Pin(switch_3_pin, Pin.IN, Pin.PULL_UP)

# Set up the GPIO pins for the LEDs
led_1 = Pin(led_1_pin, Pin.OUT)
led_2 = Pin(led_2_pin, Pin.OUT)
led_3 = Pin(led_3_pin, Pin.OUT)
led_Network_pin = Pin(led_Network_pin, Pin.OUT)

# Set initial state of the water levels
level_1_reached = False
level_2_reached = False
level_3_reached = False

#GSM Object & Initialize the gsm connection with the SIM800L module
gsm = UART(2, 9600)

#Buzzer Alarm
buzzer_pin = 23
buzzer = Pin(buzzer_pin, Pin.OUT)



#SMS Message
def send_sms(message):
    gsm.write("AT+CMGF=1\r\n")
    time.sleep(1)
    gsm.write('AT+CMGS="+09204074049"\r\n')  # Replace with your phone number
    time.sleep(1)
    gsm.write(message + "\r\n")
    time.sleep(1)
    gsm.write(bytes([26]))
    time.sleep(1)

def beep():
    for i in range(30):
        buzzer.on()
        time.sleep(0.25)
        buzzer.off()
        time.sleep(0.25)


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

# Loop to continuously check the state of the float switches
while True:
    
    # Read the state of each float switch
    switch_1_state = switch_1.value()
    switch_2_state = switch_2.value()
    switch_3_state = switch_3.value()

    # Check if level 1 has been reached and send message if not already sent
    if switch_1_state == 0 and not level_1_reached:
        print("Water level 1 has been reached")
        level_1_reached = True
        
        #Send SMS
        send_sms("Water level 1 has been reached")

    # Check if level 2 has been reached and send message if not already sent
    if switch_2_state == 0 and not level_2_reached:
        print("Water level 2 has been reached")
        level_2_reached = True
        
        #Send SMS
        send_sms("Water level 2 has been reached")

    # Check if level 3 has been reached and send message if not already sent
    if switch_3_state == 0 and not level_3_reached:
        print("Water level 3 has been reached")
        level_3_reached = True

        #Send SMS
        send_sms("Water level 3 has been reached")
        
        #Buzzer Alarm
        beep()
        
    #If Flood has Dropped
    if switch_3_state == 1 and level_3_reached:
        print("Water level has dropped to 2")
        send_sms("Water level has dropped to 2")
        level_3_reached = False
        
    if switch_2_state == 1 and level_2_reached:
        print("Water level has dropped to 1")
        send_sms("Water level has dropped to 1")
        level_2_reached = False
        
    if switch_1_state == 1 and switch_2_state == 1 and switch_3_state == 1:
        if level_1_reached or level_2_reached or level_3_reached:
            print("Flood has subsided")
            send_sms("Flood has subsided.")
        level_1_reached = False
        level_2_reached = False
        level_3_reached = False

    # Reset the state of the water levels if all switches are open
    if switch_1_state == 1 and switch_2_state == 1 and switch_3_state == 1:
        level_1_reached = False
        level_2_reached = False
        level_3_reached = False

    # Check the state of each float switch and turn on/off the corresponding LED
    if switch_1_state == 0:
        led_1.on()
    else:
        led_1.off()

    if switch_2_state == 0:
        led_2.on()
    else:
        led_2.off()

    if switch_3_state == 0:
        led_3.on()
    else:
        led_3.off()

    # Wait a short time to avoid repeating the message multiple times
    time.sleep(0.5)



