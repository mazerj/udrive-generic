#
# targets: upload, monitor, build/all (default)
#
# must be one .ino file in dir!
#

ARDUINO_QUIET = 0
ARDUINO_DIR   = /usr/share/arduino
ARDMK_DIR     = /usr
AVR_TOOLS_DIR = /usr

BOARD_TAG     = uno
ARDUINO_PORT  = /dev/ttyACM0
ARDUINO_LIBS  = AdafruitRGB Encoder MotorShield Wire 

include /usr/share/arduino/Arduino.mk
