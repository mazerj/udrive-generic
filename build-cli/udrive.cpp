#include <Arduino.h>
#define JOGPIN		2
#define SPEEDPIN	3
#define ENC_A		2
#define ENC_B		4
#define COUNTS_PER_UM 2.0157 // MM-3M-F 16:1: 51,200 counts/inch

#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <MotorShield.h>
#include <Encoder.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
MS_DCMotor motor(MOTOR_A);
Encoder myEnc(ENC_A, ENC_B);

void setup(void)
{
  //Serial.begin(9600);
  //Serial.println("hello");
  lcd.begin(16, 2);
  lcd.clear();
  motor.run(BRAKE);
  motor.setSpeed(0);}

int update = 1;
int last_msjog = -1;
uint8_t last_speed = -1;
int lastum = 0;
char buf[17];

int getmsjog()
{
  return(20*(int((1024.0-analogRead(JOGPIN))/10.0)));
}

#define VEL(x) (int(0.5 + 9.0 * x / 255.0))

uint8_t getspeed()
{
  return((1024-analogRead(SPEEDPIN)) / 8); // 0-255
}

void loop()
{
  long um;
  uint8_t buttons = lcd.readButtons();
  int msjog = -1;
  uint8_t speed = 0;

  msjog = getmsjog();
  speed = getspeed();
  if (msjog != last_msjog || speed != last_speed) {
    update = 1;
    last_msjog = msjog;
    last_speed = speed;
  }

  if (buttons) {
    if (buttons & BUTTON_SELECT) {
      myEnc.write(0);
      update = 1;
    } else if (buttons & BUTTON_DOWN) {
      //Serial.println(msjog);
      motor.setSpeed(speed);
      motor.run(FORWARD|RELEASE);
      delay(msjog);
      motor.run(BRAKE);
      update = 1;
    } else if (buttons & BUTTON_UP) {
      //Serial.println(msjog);
      motor.setSpeed(speed);
      motor.run(BACKWARD|RELEASE);
      delay(msjog);
      motor.run(BRAKE);
      update = 1;
    } else if (buttons & BUTTON_LEFT && msjog == 0) {
      delay(250);
      motor.setSpeed(speed);
      motor.run(BACKWARD|RELEASE);
      while (1) {
	speed = getspeed();
	motor.setSpeed(speed);
	um = (long) ((double)myEnc.read() / COUNTS_PER_UM);
	lcd.setCursor(0,0);
	sprintf(buf, "pos:%6dum ", um);
	lcd.print(buf);
	lcd.setCursor(1,0);
	sprintf(buf, "jog:slew ^ vel:%01d", VEL(speed));
	if (lcd.readButtons()) {
	  break;
	}
	delay(250);
      }
      motor.run(BRAKE);
      update = 1;
    } else if (buttons & BUTTON_RIGHT && msjog == 0) {
      delay(250);
      motor.setSpeed(speed);
      motor.run(FORWARD|RELEASE);
      while (1) {
	speed = getspeed();
	motor.setSpeed(speed);
	um = (long) ((double)myEnc.read() / COUNTS_PER_UM);
	lcd.setCursor(0,0);
	sprintf(buf, "pos:%6dum ", um);
	lcd.print(buf);
	lcd.setCursor(1,0);
	sprintf(buf, "jog:slew v  vel:%01d", VEL(speed));
	if (lcd.readButtons()) {
	  break;
	}
	delay(250);
      }
      motor.run(BRAKE);
      update = 1;
    }
  }

  um = (long) ((double)myEnc.read() / COUNTS_PER_UM);

  if (um != lastum) {
    update = 1;
    lastum = um;
  }
  if (update) {
    lcd.setCursor(0,0);
    sprintf(buf, "pos:%06dum", um);
    lcd.print(buf);
    
    lcd.setCursor(0,1);
    sprintf(buf, "jog:%04dms vel:%01d", msjog, VEL(speed));
    lcd.print(buf);
  }
  update = 0;
}
