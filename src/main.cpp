/*
   Object Oriented CAN based tachometer controller for WMU FSAE
   Designed by Zain Ahmed

   Made for Teensy 3.2/3.1
 */

#include <FlexCAN.h>
#include <Adafruit_NeoPixel.h>

int wakeUp = 1500;
int shiftRpm = 9000;
int redline = 12000;
int brightness = 255; // 0 to 255
int delayVal = 35;    // set wakeup sequence speed

bool EngRunning = false;
bool showingTPS = false;

int pixelPin = 14;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, pixelPin, NEO_GRB + NEO_KHZ800);

void displayTPS(double tp) { //display throttle position if engine is not running
        strip.clear();
        int ledsToLight = ceil(map(tp, 0, 100, 0, 16));

        for (int i = 0; i < ledsToLight; i++) {
                strip.setPixelColor(i, 0, 255, 255);
        }
        strip.show();
}

void setLights(int rpm) {

        if (rpm < shiftRpm) { // ----- NORMAL REVS -----

                strip.clear();

                int numLEDs = strip.numPixels();
                float rpmPerLED =
                        ((redline - wakeUp) / numLEDs); // calculates how many rpm per led
                int ledsToLight = ceil(rpm / rpmPerLED);

                for (int i = 0; i <= ledsToLight; i++) {

                        strip.setPixelColor(i, 0, 255, 0);
                }

                strip.show();
        }

        if ((rpm > shiftRpm) && (rpm < redline)) { // ----- SHIFT POINT-----
                strip.clear();

                int numLEDs = strip.numPixels();
                float rpmPerLED =
                        ((redline - wakeUp) / numLEDs); // calculates how many rpm per led
                int ledsToLight = ceil(rpm / rpmPerLED);

                for (int i = 0; i <= ledsToLight; i++) {

                        strip.setPixelColor(i, 255, 255, 0); // yellow
                }

                strip.show();
        }

        if (rpm > redline) { //----- REDLINE -----
                for (unsigned int i = 0; i < strip.numPixels(); i++) {
                        strip.setPixelColor(i, 255, 0, 0);
                }

                strip.show();
                delay(20);
                strip.clear();
                strip.show();
                delay(20);
        }
}

class canClass : public CANListener
{
public:
void printFrame(CAN_message_t &frame, int mailbox);
void gotFrame(CAN_message_t &frame, int mailbox);     //overrides the parent version so we can actually do something
};

void canClass::printFrame(CAN_message_t &frame, int mailbox)
{
        Serial.print("ID: ");
        Serial.print(frame.id, HEX);
        Serial.print(" Data: ");
        for (int c = 0; c < frame.len; c++)
        {
                Serial.print(frame.buf[c], HEX);
                Serial.write(' ');
        }
        Serial.write('\r');
        Serial.write('\n');
}

void canClass::gotFrame(CAN_message_t &frame, int mailbox) //runs every time a frame is recieved
{
        printFrame(frame, mailbox);
        digitalWrite(13, !digitalRead(13));

        if (frame.id == 218099784) { //frame has rpm and tps percentage
                int lowByte = frame.buf[0];
                int highByte = frame.buf[1];
                int newRPM = ((highByte * 256) + lowByte);

                if (newRPM != 0) {
                        EngRunning = true;
                        showingTPS = false;
                        setLights(newRPM);
                } else {
                        EngRunning = false;
                        double lowByte = frame.buf[2];
                        double highByte = frame.buf[3];
                        double tps = ((highByte * 256) + lowByte) / 10;
                        if (tps > 20) {
                                showingTPS = true;
                                displayTPS(tps);
                        } else {
                          showingTPS = false;
                        }
                }

        }
}

canClass canClass;

// -------------------------------------------------------------

void lightShow() {

        strip.clear(); // green bits
        for (int i = 0; i < 10; i++) {
                for (int j = 0; j < i; j++) {
                        strip.setPixelColor(j, 0, 255, 0);
                }

                strip.show();
                delay(delayVal);
        }

        delay(50);
        strip.clear(); // green bits
        for (int i = 0; i < 10; i++) {
                for (int j = 0; j < i; j++) {
                        strip.setPixelColor(j, 0, 255, 0);
                }

                strip.show();
                delay(delayVal);
        }

        strip.clear();

        for (int i = 10; i < strip.numPixels(); i++) { // yellow bits
                for (int j = 0; j < 10; j++) {
                        strip.setPixelColor(j, 255, 255, 0);
                }
                strip.show();
                strip.setPixelColor(i, 255, 255, 0);
                if (i > 10) {
                        delay(delayVal);
                }
        }

        delay(50);
        strip.clear(); // green bits
        for (int i = 0; i < 10; i++) {
                for (int j = 0; j < i; j++) {
                        strip.setPixelColor(j, 0, 255, 0);
                }
                strip.show();
                delay(delayVal);
        }

        strip.clear();
        for (int i = 10; i < strip.numPixels(); i++) { // yellow bits
                for (int j = 0; j < 10; j++) {
                        strip.setPixelColor(j, 255, 255, 0);
                }
                strip.show();
                strip.setPixelColor(i, 255, 255, 0);
                if (i > 10) {
                        delay(delayVal);
                }
        }

        for (int i = 0; i < 25; i++) {
                strip.clear();

                for (int j = 0; j < strip.numPixels(); j++)
                        strip.setPixelColor(j, 255, 0, 0);

                strip.show();
                delay(20);
                strip.clear();

                for (int j = 0; j < strip.numPixels(); j++) {
                        strip.setPixelColor(j, 0, 0, 0);
                }

                strip.show();
                delay(20);
        }
}

void setup(void)
{
        Serial.println("online");

        Can0.begin(250000); //PE3 ECU SPEED

        pinMode(13, OUTPUT);
        digitalWrite(13, HIGH);
        Can0.attachObj(&canClass);
        canClass.attachGeneralHandler();

        strip.begin();
        strip.setBrightness(brightness);
        strip.show();
        lightShow();
}


// -------------------------------------------------------------
void loop(void)
{
        if (EngRunning == false && showingTPS == false) { // heartbeat
                for (int i = 0; i <= 15; i++) {
                        strip.setPixelColor(i, 255, 0, 0);
                        strip.show();
                }

                delay(70);

                for (int i = 0; i < strip.numPixels(); i++) {
                        strip.setPixelColor(i, 40, 0, 0);
                        strip.show();
                }

                delay(80);

                for (int i = 0; i < strip.numPixels(); i++) {
                        strip.setPixelColor(i, 255, 0, 0);
                        strip.show();
                }

                delay(70);

                for (int i = 0; i < strip.numPixels(); i++) {
                        strip.setPixelColor(i, 40, 0, 0);
                        strip.show();
                }
                strip.clear();
                delay(1500);
        }
}
