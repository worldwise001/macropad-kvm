#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>
#include <RotaryEncoder.h>
#include <Wire.h>

// Create the neopixel strip with the built in definitions NUM_NEOPIXEL and PIN_NEOPIXEL
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_NEOPIXEL, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Create the OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &SPI1, OLED_DC, OLED_RST, OLED_CS);

// Create the rotary encoder
RotaryEncoder encoder(PIN_ROTA, PIN_ROTB, RotaryEncoder::LatchMode::FOUR3);
void checkPosition() {
  encoder.tick();  // just call tick() to check the state.
}
// our encoder position state
int encoder_pos = 0;

void setup() {
  Serial.begin(115200);
  //while (!Serial) { delay(10); }     // wait till serial port is opened
  delay(100);  // RP2040 delay is not a bad idea

  Serial.println("Adafruit Macropad with RP2040");

  // start pixels!
  pixels.begin();
  pixels.setBrightness(255);
  pixels.show(); // Initialize all pixels to 'off'

  // Start OLED
  display.begin(0, true); // we dont use the i2c address but we will reset!
  display.display();

  // set all mechanical keys to inputs
  for (uint8_t i = 0; i <= 12; i++) {
    pinMode(i, INPUT_PULLUP);
  }

  // set rotary encoder inputs and interrupts
  pinMode(PIN_ROTA, INPUT_PULLUP);
  pinMode(PIN_ROTB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ROTA), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ROTB), checkPosition, CHANGE);

  // text display tests
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(SH110X_WHITE, SH110X_BLACK); // white text, black background

  pinMode(PIN_SPEAKER, OUTPUT);
  digitalWrite(PIN_SPEAKER, LOW);
  // tone(PIN_SPEAKER, 988, 100);  // tone1 - B5
  // delay(100);
  // tone(PIN_SPEAKER, 1319, 200); // tone2 - E6
  // delay(200);
}

uint8_t selected = 0;

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("* Adafruit Macropad *");

  encoder.tick(); // check the encoder
  int newPos = encoder.getPosition();
  if (encoder_pos != newPos) {
    Serial.print("Encoder:");
    Serial.print(newPos);
    Serial.print(" Direction:");
    Serial.println((int)(encoder.getDirection()));
    encoder_pos = newPos;
  }
  display.setCursor(0, 8);
  display.print("Rotary encoder: ");
  display.print(encoder_pos);

  // check encoder press
  display.setCursor(0, 24);
  if (!digitalRead(PIN_SWITCH)) {
    Serial.println("Encoder button");
    display.print("Encoder pressed ");
  }

  for (int i = 1; i <= 12; i++) {
    if (!digitalRead(i)) { // switch pressed!
      Serial.print("Switch "); Serial.println(i);
      if (i % 3 == 0) {
        if (selected != 0) {
          pixels.setPixelColor((selected*3) - 1, 0x000000);
        }
        selected = i/3;
      }
      pixels.setPixelColor(i - 1, 0xFFFFFF); // make white
      // move the text into a 3x4 grid
      display.setCursor(((i - 1) % 3) * 48, 32 + ((i - 1) / 3) * 8);
      display.print("KEY");
      display.print(i);
    } else {
      if (i % 3 != 0) {
        pixels.setPixelColor(i - 1, 0x000000);
      }
    }
  }

  // show neopixels
  pixels.show();

  // display oled
  display.display();
}
