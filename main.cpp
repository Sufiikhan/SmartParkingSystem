#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClient.h>

// Wi-Fi credentials
char ssid[] = "DCS-WLAN-2";   // Replace with your Wi-Fi network name
char pass[] = "987654321";      // Replace with your Wi-Fi password

// Blynk Auth Token and Template
#define BLYNK_AUTH_TOKEN "gAPCbhWMxbeabqsF9fpCxQblXVmqpicu"
#define BLYNK_TEMPLATE_ID    "TMPL62GAbY64b"  // Replace with your template ID
#define BLYNK_TEMPLATE_NAME  "Smart Parking System" // Replace with your template name

// Pin definitions
const int IR1 = 2;  // GPIO for IR sensor 1
const int IR2 = 3;  // GPIO for IR sensor 2
int Slot = 5;       // Total number of parking slots

unsigned long lastActionTime = 0; // Variable to track the last action time
const unsigned long debounceDelay = 200; // Minimum time between sensor readings (debouncing)

BlynkTimer timer;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize LCD with I2C address 0x27
Servo myservo;                      // Declare servo

void updateLCD();  // Declare the updateLCD() function

// Define the myTimerEvent function
void myTimerEvent()
{
    long uptime = millis() / 1000;  // Get the uptime in seconds
    Blynk.virtualWrite(V2, uptime);  // Send the uptime to Virtual Pin 2
}

void setup()
{
    Serial.begin(115200); // Initialize serial communication at 115200 baud

    // Connect to Wi-Fi and Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Initialize LCD
    lcd.init();      // Initialize the LCD
    lcd.backlight(); // Turn on the LCD backlight
    lcd.setCursor(0, 0);
    lcd.print("   ESP32 SYSTEM  ");
    lcd.setCursor(0, 1);
    lcd.print(" PARKING SYSTEM ");
    delay(2000);
    lcd.clear();

    // Set IR sensor pins as input
    pinMode(IR1, INPUT);
    pinMode(IR2, INPUT);

    // Attach servo motor to pin 21
    myservo.attach(21);
    myservo.write(100); // Set servo to initial position (gate closed)

    // Set the timer to send data to Blynk every second
    timer.setInterval(1000L, myTimerEvent);
}

void loop()
{
    unsigned long currentMillis = millis(); // Get the current time

    // Check for car entry
    if (digitalRead(IR1) == LOW && (currentMillis - lastActionTime) > debounceDelay)
    {
        lastActionTime = currentMillis; // Update last action time
        if (Slot > 0)
        {
            myservo.write(0); // Open gate
            delay(500);        // Wait for servo to open
            Slot--;           // Decrement available slots
            updateLCD();
            Blynk.virtualWrite(V1, Slot); // Send updated slot data to Blynk app (V1 virtual pin)
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("    SORRY :(    ");
            lcd.setCursor(0, 1);
            lcd.print("  Parking Full  ");
            delay(3000);
            lcd.clear();
        }
    }

    // Check for car exit
    if (digitalRead(IR2) == LOW && (currentMillis - lastActionTime) > debounceDelay)
    {
        lastActionTime = currentMillis; // Update last action time
        myservo.write(0); // Open gate
        delay(500);        // Wait for servo to open
        Slot++;            // Increment available slots
        updateLCD();
        Blynk.virtualWrite(V1, Slot); // Send updated slot data to Blynk app (V1 virtual pin)
    }

    // Close gate if both flags are triggered and slots are not full
    if (Slot != 5) // Close gate if slot is not full
    {
        delay(1000);
        myservo.write(100); // Close gate
        delay(500);          // Wait for servo to close
    }

    Blynk.run(); // Run Blynk's event loop
    timer.run(); // Run the timer to send events
}

// Update the LCD based on the current slot count
void updateLCD()
{
    lcd.setCursor(0, 0);
    lcd.print("    WELCOME!    ");
    lcd.setCursor(0, 1);
    lcd.print("Slot Left: ");
    lcd.print(Slot);
}
