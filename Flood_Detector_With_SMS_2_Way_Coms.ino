#include <SoftwareSerial.h>

// Define pin numbers for float switches
const int switch_1_pin = 25;
const int switch_2_pin = 26;
const int switch_3_pin = 27;

// Define pin numbers for LEDs
const int led_1_pin = 18;
const int led_2_pin = 19;
const int led_3_pin = 21;
const int led_Network_pin = 22;

// Set initial state of the water levels
bool level_1_reached = false;
bool level_2_reached = false;
bool level_3_reached = false;

// Ensures Flood Subsides
bool flood_subsided = false;

// Removes pin fluctuations
bool fluctuation_1 = false;
bool fluctuation_2 = false;
bool fluctuation_3 = false;

// GSM Object
SoftwareSerial gsm(16, 17);  // RX, TX

// Buzzer Alarm
const int buzzer_pin = 23;

// Current water level
int current_water_level = 0;

// Last time network status was checked
unsigned long lastNetworkCheckTime = 0;
const unsigned long networkCheckInterval = 5000; // Check every 5 seconds

// Last time SMS was processed
unsigned long lastSMSProcessTime = 0;
const unsigned long smsProcessInterval = 1000; // Process SMS every 1 second

// Setup function
void setup() {
  // Set up pins for float switches
  pinMode(switch_1_pin, INPUT_PULLUP);
  pinMode(switch_2_pin, INPUT_PULLUP);
  pinMode(switch_3_pin, INPUT_PULLUP);

  // Set up pins for LEDs
  pinMode(led_1_pin, OUTPUT);
  pinMode(led_2_pin, OUTPUT);
  pinMode(led_3_pin, OUTPUT);
  pinMode(led_Network_pin, OUTPUT);

  // Set up buzzer pin
  pinMode(buzzer_pin, OUTPUT);

  // Initialize GSM module
  Serial.begin(115200); // Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  gsm.begin(9600);      // Begin serial communication with Arduino and SIM800L

  gsm.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  gsm.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
}

// Main loop
void loop() {
  unsigned long currentMillis = millis();

  // Check network registration status periodically
  if (currentMillis - lastNetworkCheckTime >= networkCheckInterval) {
    checkNetworkLED();
    lastNetworkCheckTime = currentMillis;
  }

  // Process incoming SMS periodically
  if (currentMillis - lastSMSProcessTime >= smsProcessInterval) {
    updateSerial();
    lastSMSProcessTime = currentMillis;
  }

  // Read the state of each float switch
  int switch_1_state = digitalRead(switch_1_pin);
  int switch_2_state = digitalRead(switch_2_pin);
  int switch_3_state = digitalRead(switch_3_pin);

  // Check water levels and send SMS if necessary
  if (switch_1_state == LOW && !level_1_reached && !fluctuation_1) {
    Serial.println("Water level 1 has been reached");
    level_1_reached = true;
    fluctuation_1 = false;
    current_water_level = 1;
    send_sms("Water level 1 has been reached");
  }

  if (switch_2_state == LOW && !level_2_reached && !fluctuation_2) {
    Serial.println("Water level 2 has been reached");
    level_2_reached = true;
    fluctuation_2 = false;
    fluctuation_1 = true;
    current_water_level = 2;
    send_sms("Water level 2 has been reached");
  }

  if (switch_3_state == LOW && !level_3_reached) {
    Serial.println("Water level 3 has been reached");
    level_3_reached = true;
    flood_subsided = true;
    fluctuation_3 = false;
    fluctuation_2 = true;
    fluctuation_1 = true;
    current_water_level = 3;
    send_sms("Water level 3 has been reached");
    beep();
  }

  // Check if flood has subsided
  if (switch_3_state == HIGH && level_3_reached) {
    Serial.println("Water level has dropped to 2");
    send_sms("Water level has dropped to 2");
    level_3_reached = false;
    current_water_level = 2;
  }

  if (switch_2_state == HIGH && level_2_reached && switch_3_state == HIGH) {
    Serial.println("Water level has dropped to 1");
    send_sms("Water level has dropped to 1");
    level_2_reached = false;
    flood_subsided = true;
    current_water_level = 1;
  }

  if (switch_1_state == HIGH && switch_2_state == HIGH && switch_3_state == HIGH) {
    if (level_1_reached || level_2_reached || level_3_reached || flood_subsided) {
      delay(4000);
      Serial.println("Flood has subsided");
      send_sms("Flood has subsided");
    }

    // Reset water level states if all switches are open
    level_1_reached = false;
    level_2_reached = false;
    level_3_reached = false;
    flood_subsided = false;

    // Reset pin fluctuations
    fluctuation_1 = false;
    fluctuation_2 = false;
    fluctuation_3 = false;

    current_water_level = 0;
  }

  // Update LED states based on float switch states
  digitalWrite(led_1_pin, switch_1_state == LOW ? HIGH : LOW);
  digitalWrite(led_2_pin, switch_2_state == LOW ? HIGH : LOW);
  digitalWrite(led_3_pin, switch_3_state == LOW ? HIGH : LOW);
}

void checkNetworkLED() {
  gsm.print("AT+CREG?\r\n");
  delay(100);
  String response = "";
  while (gsm.available()) {
    response += char(gsm.read());
  }
  if (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1) {
    // Network registration successful or already registered
    digitalWrite(led_Network_pin, HIGH);
  } else {
    // Network registration failed or not registered
    digitalWrite(led_Network_pin, LOW);
  }
}

// Function for buzzer alarm
void beep() {
  for (int i = 0; i < 30; i++) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay(250);
  }
}

// Function to send SMS
void send_sms(String message) {
  gsm.print("AT+CMGF=1\r\n");
  delay(100);
  gsm.print("AT+CMGS=\"+09204074049\"\r\n"); // Replace with your phone number
  delay(100);
  gsm.print(message + "\r\n");
  delay(100);
  gsm.write(26);
  delay(100);
}

void updateSerial() {
  while (gsm.available()) {
    String receivedData = gsm.readStringUntil('\n'); // Read the received data

    if (receivedData.indexOf("level") != -1) // Check if received SMS contains the keyword "level"
    {
      Serial.println("Received SMS contains the keyword 'level'.");
      Serial.println(" Current Water Level: " + String(current_water_level));
      send_sms(" Current Water Level: " + String(current_water_level));
      // Additional actions can be performed here
    }
  }
}
