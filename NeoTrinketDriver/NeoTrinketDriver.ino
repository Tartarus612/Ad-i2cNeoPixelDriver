// Functions are:
// a = set number of pixels, a [X] where X is number of pixels function cmd a parameter a,x
// b = set brightness, values are 0 to 100, but need to internally modify since 0 = max brightness
// (color values are interpreted literally; no scaling), 1 = min
// brightness (off), 255 = just below max brightness. function cmd b,x
//c = update a pixel at index to the RGB colors function cmd c parameters c,index,r, g, b
//d = turn off all pixels
#include <Wire.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <string.h>

#define SLAVE_ADDRESS 0x04
#define PIN 4
int m_numPixels = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel m_strip = Adafruit_NeoPixel(144, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  m_strip.begin();
  m_strip.setBrightness(0);
  m_strip.show(); // Initialize all pixels to 'off'
  // put your setup code here, to run once:
  Serial.begin(9600); // start serial for output
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);

  // define callbacks for i2c communication
  Wire.onReceive(receiveEvent);

  Serial.println("Ready!");
}

void loop() {
  delay(10000);
}

// callback for received data
void receiveEvent(int howMany) {
  String recdData = "";
  int numOfBytes = Wire.available();
  //display number of bytes and cmd received, as bytes

  char cmd = Wire.read();  //cmd
  //display message received, as char
  for (int i = 0; i < numOfBytes - 1; i++) {
    recdData += char(Wire.read());
  }

    char recdDataChar[recdData.length() + 1];
    recdData.toCharArray(recdDataChar, sizeof(recdDataChar));

  switch (cmd) {
    case 'a':
      m_numPixels = parseSingleInt16FromParameter(recdDataChar);
      m_strip.updateLength(m_numPixels);
      m_strip.show(); // Initialize all pixels to 'off'
      break;
    case 'b':
      m_strip.setBrightness(convertFromPercentToStripBrightness(parseSingleInt8FromParameter(recdDataChar)));
      m_strip.show();
      break;
    case 'c':
    //Serial.println("recdData = " + (String)recdData + " recdDataChar = "+ (String)recdDataChar + " parsePixelIndexFromParameter = " + (String)parsePixelIndexFromParameter(recdDataChar) + " parseColorFromParameter = " + (String)parseColorFromParameter(recdDataChar));
      m_strip.setPixelColor(parsePixelIndexFromParameter(recdDataChar), parseColorFromParameter(recdDataChar));
      m_strip.show();
      break;
    case 'd':
      for(int i = 0; i < m_numPixels; i++)
      {
        m_strip.setPixelColor(i, m_strip.Color(0, 0, 0));
      }
      m_strip.show();
      break;
    default:
      Serial.println("switch default");
      break;
  }
}

uint16_t parsePixelIndexFromParameter(char* parameter)
{
  char* ptr = strtok(parameter, ",");
  return atol(ptr);
}

uint32_t parseColorFromParameter(char* parameter)
{
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  int firstComma = ((String)parameter).indexOf(',');
  r = atol((((String)parameter).substring(firstComma + 1, firstComma + 4)).c_str());
  g = atol((((String)parameter).substring(firstComma + 5, firstComma + 8)).c_str());
  b = atol((((String)parameter).substring(firstComma + 9, firstComma + 12)).c_str());
  
  return m_strip.Color(r, g, b);
}

uint16_t parseSingleInt16FromParameter(char* parameter)
{
  return (uint16_t)atol(parameter);
}

uint8_t parseSingleInt8FromParameter(char* parameter)
{
  return (uint8_t)atol(parameter);
}

uint8_t convertFromPercentToStripBrightness(uint8_t percent)
{
//0 = max brightness
// (color values are interpreted literally; no scaling), 1 = min
// brightness (off), 255 = just below max brightness.
  if(percent == 100)
  {
    return 255;
  }
  else
  {
    return (uint8_t)(((float)255)*(((float)percent)/100));
  }
}

