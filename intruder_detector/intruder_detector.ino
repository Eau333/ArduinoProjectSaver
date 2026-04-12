#include <Arduino.h>

#include "PinDefinitionsAndMore.h" // Set IR_RECEIVE_PIN for different CPU's

#include "pitches.h"

#include "TinyIRReceiver.hpp" // include the code

#include "SR04.h"
#define TRIG_PIN 5
#define ECHO_PIN 7

#define BLUE 11
#define GREEN 10
#define RED 9

long a;
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);

int melody[] = {
  NOTE_C4, NOTE_C7};
int duration = 50;

/*

0 = 0x16
1 = 0xC
2 = 0x18
3 = 0x5E
4 = 0x8
5 = 0x1C
6 = 0x5A
7 = 0x42
8 = 0x52
9 = 0x4A

*/

int passcode[] = {
    0x16, 0xC, 0x18, 0x5E
};

bool armed = true;

bool detected = false;

bool coding = false;

int buzzer = 8;

void setup() {
    Serial.begin(115200);

    pinMode(buzzer,OUTPUT);
    setled(2);

    #if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
        || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
        // Wait until Serial Monitor is attached.
        // Required for boards using USB code for Serial like Leonardo.
        // Is void for USB Serial implementations using external chips e.g. a CH340.
        while (!Serial)
            ;
        // !!! Program will not proceed if no Serial Monitor is attached !!!
    #endif

        // Just to know which program is running on my Arduino
    #if defined(ESP8266) || defined(ESP32)
        Serial.println();
    #endif
        Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_TINYIR));

        // Enables the interrupt generation on change of IR input signal
        if (!initPCIInterruptForTinyReceiver()) {
            Serial.println(F("No interrupt available for pin " STR(IR_RECEIVE_PIN))); // optimized out by the compiler, if not required :-)
        }
    #if defined(USE_FAST_PROTOCOL)
        Serial.println(F("Ready to receive Fast IR signals at pin " STR(IR_RECEIVE_PIN)));
    #else
        Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_RECEIVE_PIN)));
    #endif
}

void setled(int colour){
    digitalWrite(RED, 0);
    digitalWrite(GREEN, 0);
    digitalWrite(BLUE, 0);
    switch (colour){
        case 0:
            digitalWrite(RED, 255);
            break;
        case 1:
            digitalWrite(GREEN, 255);
            break;
        case 2:
            digitalWrite(BLUE, 255);
    }
}

void loop() {
    if (TinyReceiverDecode()) {
        if (TinyIRReceiverData.Command == 0x45){
            armed = true;
            setled(1);
            digitalWrite(13,HIGH);
            Serial.println("power button received");        
        }
        else if (TinyIRReceiverData.Command == 0x47){
            Serial.println("stop button recieved");
            if (detected){
                coding = true;
                setled(2);
                int digit = 0;
                while (coding){
                    if (TinyReceiverDecode() && !(TinyIRReceiverData.Flags == IRDATA_FLAGS_IS_REPEAT)){
                        if (TinyIRReceiverData.Command == passcode[digit]){
                            digit += 1;
                            setled(1);
                        }
                        else {
                            digit = 0;
                            setled(0);
                        }
                        if (digit == 4){
                            detected = false;
                            coding = false;
                            setled(1);
                            armed = true;
                            digitalWrite(buzzer,LOW);
                            digitalWrite(13,LOW);
                        }
                        else {
                            delay(500);
                            setled(2);
                        }    
                    }
                }
            }
            else {
                armed = false;
                setled(0);
            }
        }
        Serial.println(TinyIRReceiverData.Command, HEX);
    }
    if (armed){
        if (!detected && sr04.Distance() < 50){
            detected = true;
        }
        if (detected){
            for (int thisNote = 0; thisNote < 2; thisNote++) {
                // pin8 output the voice, every scale is 0.5 sencond
                tone(4, melody[thisNote], duration);
                
                // Output the voice after several minutes
                delay(duration);
            }
            digitalWrite(buzzer,HIGH);
        }
    }
    // Serial.print(sr04.Distance());
    // Serial.println("cm");
}

