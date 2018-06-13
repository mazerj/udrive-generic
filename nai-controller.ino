/* Thu Feb 22 15:42:22 2018 mazer 
** Updated and hardware fixed up to work again..
**
** Fri Feb 23 12:19:35 2018 mazer 
**   making work with both Adafruit V2 controller 9AFV2 and original
**   Arduino.cc R3.
*/


// R3 for old-style original Arduino Motor Shield R3 (otherwise, Adafruit V2)
//#define R3	1

#include <Wire.h>			// built-in library
#include <Adafruit_RGBLCDShield.h>	// adafruit lcd shield
#include <Encoder.h>			// quadrature encoder library
#include <EEPROMex.h>			// read/write EPROM vals
#ifdef R3
# include <MotorShield.h>		// works with Arduino-brand R3 shield
#else
# include <Adafruit_MotorShield.h>	// Adafruit V2
#endif

#define WHITE		0x07		// FOR LCD BACKLIGHT
#define JOGPIN		2		// ANALOG input pin for jog pot
#define VELPIN		3		// ANALOG input pin for velocity pot
#define ENC_A		2		// AB shaft encoder line A (DIG in)
#define ENC_B		4		// AB shaft encoder line B (DIG in)
#define COUNTS_PER_UM	2.0157		// MM-3M-F 16:1: 51,200 counts/inch

//#define SLEEPAFTER	0		// never sleep backlight
#define SLEEPAFTER	(1000*60*5)    // sleep after 5 mins

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#ifdef R3
MS_DCMotor motor(MOTOR_A);
#else
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *motor = AFMS.getMotor(1);
#endif

Encoder myEnc(ENC_A, ENC_B);

int update = 1;			// update lcd display?
int lcdon = 0;			// lcd backlight on?
int last_msjog = -1;		// last jog size setting in um
uint8_t last_speed = -1;	// last speed setting in 8bit PWM units
int lastum = 0;			// last encoder position in um
unsigned long last_update = 0;	// last activity time (for "sleep")
char buf[17];			// scratch buffer for lcd display

void setup(void)
{
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcdon = 1;

  lcd.setCursor(0,0);
  lcd.print("NAI-Controller");
  lcd.setCursor(0,1);
  
#ifdef R3
  motor.run(BRAKE);
  motor.setSpeed(0);
  lcd.print("old R3 mtr shld");
#else
  AFMS.begin();
  motor->run(RELEASE);
  motor->setSpeed(0);
  lcd.print("adafr mtr shld");
#endif
  // retrieve saved encoder position
  myEnc.write(EEPROM.readLong(0));
  delay(1000);
  lcd.clear();
}

// speed to normalized velocity (0-9)
// for display only
#define NORMVEL(s) (int(0.5 + 9.0 * s / 255.0))

uint8_t getspeed()
{
  int s = (1024-analogRead(VELPIN)) / 4;
  return(min(255, max(0, s)));
}

int getmsjog()
{
  // gives range from 0-500ms
  return(20*(int((1024.0-analogRead(JOGPIN))/40.0)));
}

void motor_stop()
{
#ifdef R3
  motor.run(BRAKE);
#else
  motor->run(RELEASE);
#endif
}

void motor_run(uint8_t speed, int flags, int dur)
{
#ifdef R3
  motor.setSpeed(speed);
  motor.run(flags);
#else
  motor->setSpeed(speed);
  motor->run(flags);
#endif
  if (dur) {
    delay(dur);
    motor_stop();
  }
}

#ifdef R3
# define UP FORWARD|RELEASE
# define DOWN BACKWARD|RELEASE
#else
# define UP BACKWARD
# define DOWN FORWARD
#endif

void loop()
{
  long um;
  int msjog = -1;
  uint8_t speed = 0;
  unsigned long ms = millis();
  uint8_t buttons = lcd.readButtons();

  msjog = getmsjog();
  speed = getspeed();
  if (msjog != last_msjog || speed != last_speed) {
    update = 1;
    last_msjog = msjog;
    last_speed = speed;
  }

  if (buttons) {
    update = 1;
    if ((buttons & BUTTON_SELECT) && (buttons & BUTTON_LEFT)) {
      // s-left: zero counter
      myEnc.write(0);
      update = 1;
    } else if ((buttons & BUTTON_SELECT) && (buttons & BUTTON_RIGHT)) {
      // s-right: save position
      // save current position to EEPROM
      EEPROM.writeLong(0, myEnc.read());
    } else if ((buttons & BUTTON_DOWN) && msjog > 0) {
      // down && jog > 0: jog down
      motor_run(speed, DOWN, msjog);
      update = 1;
    } else if ((buttons & BUTTON_UP) && msjog > 0) {
      // up && jog > 0: jog up
      motor_run(speed, UP, msjog);
      update = 1;
    } else if ((buttons & BUTTON_UP) && msjog == 0) {
      // up & jog == 0: slew up
      do {
	speed = getspeed();
	motor_run(speed, UP, 0);
	um = (long) ((double)myEnc.read() / COUNTS_PER_UM);
	lcd.setCursor(0,0);
	sprintf(buf, "pos:%06dum", (int)um);
	lcd.print(buf);
	lcd.setCursor(0,1);
	sprintf(buf, "  SLEWING  UP   ");
	lcd.print(buf);
	buttons = lcd.readButtons();
      } while ((buttons & BUTTON_UP) && msjog == 0);
      motor_stop();
      update = 1;
    } else if (buttons & BUTTON_DOWN && msjog == 0) {
      // down & jog == 0: slew down
      do {
	speed = getspeed();
	motor_run(speed, DOWN, 0);
	um = (long) ((double)myEnc.read() / COUNTS_PER_UM);
	lcd.setCursor(0,0);
	sprintf(buf, "pos:%06dum", (int)um);
	lcd.print(buf);
	lcd.setCursor(0,1);
	sprintf(buf, "1234567890123456");
	sprintf(buf, "  SLEWING DOWN  ");
	lcd.print(buf);
	buttons = lcd.readButtons();
      } while ((buttons & BUTTON_DOWN) && msjog==0);
      motor_stop();
      update = 1;
    }
  }

  um = (long) ((double)myEnc.read() / COUNTS_PER_UM);

  if (um != lastum) {
    update = 1;
    lastum = um;
  }
  if (update) {
    if (! lcdon) {
      lcd.setBacklight(WHITE);
      lcdon = 1;
    }

    lcd.setCursor(0,0);
    sprintf(buf, "pos:%06dum", (int)um);
    lcd.print(buf);
    
    lcd.setCursor(0,1);
    sprintf(buf, "jog:%04dms vel:%01d", msjog, NORMVEL(speed));
    lcd.print(buf);

    last_update = ms;
  } else {
    if (SLEEPAFTER && lcdon && ((ms - last_update) > SLEEPAFTER)) {
	lcd.setBacklight(0x0);
	lcdon = 0;
    }
  }
  update = 0;
}
