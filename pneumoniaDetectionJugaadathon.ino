#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <MPU6050.h>

//Temperature Sensor definitions
const int B=4275;                 // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
unsigned long lastRecordTime;

//Respiration Sensor definitions
MPU6050 mpu;
int breathCount=0;
int breathStatus=0;

//OLED definitions
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup() {

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  Serial.begin(115200);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
  
  breathCount=0;

  Serial.println("Initialize MPU6050");
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  
  mpu.calibrateGyro();

  mpu.setThreshold(3);

  lastRecordTime=millis();
}

float checkTemperature()
{
  int a = analogRead(A3);
  float R = 1023.0/((float)a)-1.0;
  R = 100000.0*R;
  float temperatureC=1.0/(log(R/100000.0)/B+1/298.15)-273.15;//convert to temperature via datasheet ;
  float temperatureF = (temperatureC*1.8)+32;

  return temperatureF;
}

void checkBreathCount()
{
  Vector rawGyro = mpu.readRawGyro();  
  if (rawGyro.XAxis>130)
  {
    breathCount++;
    digitalWrite(5, HIGH);
    delay(250);
    digitalWrite(5, LOW);
  }
}

void checkBreathStatus()
{
  if ((breathCount/2)>=30)
    {
      Serial.println("Warning!");
      lastRecordTime=millis();
      breathCount=0;
      breathStatus=2;
    }

    else if ((breathCount/2)<30)
    {
      Serial.println("No Worries!");
      lastRecordTime=millis();
      breathCount=0;
      breathStatus=1;
    }
}

void printOnSerial()
{
  Serial.print(breathCount);
  Serial.print("\t");
  Serial.print(checkTemperature());
  Serial.print("\t");
}

void displayOnOLED()
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(checkTemperature());
  display.println(breathCount);
  switch(breathStatus)
  {
    case 1: display.println("Good"); digitalWrite(4, HIGH); digitalWrite(6, LOW); break;
    case 2: display.println("Warning!"); digitalWrite(4, LOW); digitalWrite(6, HIGH); break;
        
  }
  display.display();
  display.clearDisplay();
}


void loop() {

  checkBreathCount();
  if ((millis()-lastRecordTime)>30000)
  {
    checkBreathStatus();
  }

  printOnSerial();
  displayOnOLED();

  Serial.println();

}
