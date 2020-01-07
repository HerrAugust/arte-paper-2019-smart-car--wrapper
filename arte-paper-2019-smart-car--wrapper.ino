/**
   The car receives commands via bluetooth and via the Arduino IDE serial monitor.
   It is user choice how to give those commands.
   The car has 3 modes: STOPPED, AUTO and MANUAL. It starts in STOPPED mode.
   In this state (STOPPED mode), the accepted command are:
   1. "follow line" ("fwln" for short), in which case the car enters AUTO mode
   2. "go ahead", "go back", "turn left", "turn right", in which case
      the car enters MANUAL mode

   In AUTO mode, the car follows a line and the accepted commands are:
   1. "go ahead", "go back", "turn left", "turn right", in which case
      the car enters MANUAL mode

   In MANUAL mode, the car performs the required command pnly. Accepted commands are:
   1. "go ahead"
   2. "go back" and "go backwards"
   3. "turn left"
   4. "turn right"
   5. "stop", in which case the car enters STOPPED mode

   The car behaviour is drived by some periodic tasks. Sensing and driver tasks have the
   highest priorities since have control of the car.
   1. Communication task      - receives cmds from bluetooth
   2. Control task            - stores info about sensors and,
                                based on the current modes and user commands,
                                controls the motors and their available speeds,
                                making the car move.

   Credits: A.Mascitti, M.Marinoni
   This code is based on the one by Stefano Maugeri,
   https://www.hackster.io/stefano_maugeri/autonomous-line-follower-with-seeed-shield-bot-1-2-and-arte-4faf68
   More info available at the paper [paper under revision].
*/

#include "DriverMotors.h"
#include <arte.h>
#include <SoftwareSerial.h>


#define COMMUNICATION     loop1
#define CONTROL           loop2


#define sprint            Serial.print
#define sprintln          Serial.println


//modes:
#define AUTO              'a' /* line follower */
#define MANUAL            'm' /* bluetooth and serial */
#define STOPPED           's' /* stopped */


#define   CMD_GO_AHEAD    "goa"
#define   CMD_GO_BACK     "gob"
#define   CMD_TURN_LEFT   "tlf"
#define   CMD_TURN_RIGHT  "trg"
#define   CMD_STOP        "stp"
#define   CMD_FOLLOW_LINE "flwln"
#define   CMD_FASTER      "fast"
#define   CMD_SLOWER      "slow"


#define   resetCommands() commands.isGoAhead   = false;    \
                          commands.isGoBack    = false;    \
                          commands.isTurnRight = false;    \
                          commands.isTurnLeft  = false;    \
                                                           \
                          commands.isStop      = false;

// bluetooth (BT) module configuration
#define   BT_RX_PIN 0 /* please, connect BT RXD to digital pin 0 */
#define   BT_TX_PIN 1 /* please, connect BT TXD to digital pin 1 */



// --------------------------- variables -------------------------------------------------------

SoftwareSerial serialInput (BT_RX_PIN, BT_TX_PIN);

int S1;                                   // proximity sensors values
int S2;
int S3;
int S4;
int S5;                      

enum speedModes { SLOW, FAST } speedMode; // motors speeds
int  speeds[] = { 80,  255 };             // motors speeds corresponding values

static char mode;                         // current mode, one of AUTO, STOPPED, MANUAL

struct commands_struct {
  bool isGoAhead;
  bool isGoBack;
  bool isTurnRight;
  bool isTurnLeft;
  
  bool isStop;
  
  bool isSpeedModeChanged;
} commands;


// --------------------------- functions -------------------------------------------------------

void   setNextAction(String cmd);
String getCommand();
void   follow_line();

void setup() {
  long baudrate = 115200;
  Serial.begin (baudrate); // serial output
  serialInput.begin(baudrate);  // serial input

  sprintln ("setup()");
  speedMode = SLOW;
  initialize(speeds[speedMode], speeds[speedMode]);
  mode = STOPPED;

  sprintln("--------------------------------------------------------------------");
  sprintln("Waiting for bluetooth commands.");
  sprintln("--------------------------------------------------------------------");

}

/*
  Read sensors and, based on user requests, activate motors.
  It must be able to move the rover only if we are in AUTO mode or MANUAL mode, else keep still the rover.
*/
void CONTROL(100) {  
  if (mode == AUTO) {
    S1 = readS5();
    S2 = readS4();
    S3 = readS3();
    S4 = readS2();
    S5 = readS1();

    follow_line();
  }
  else { // MANUAL, STOPPED
    if (commands.isStop) {
      stop();
    }
    else if (commands.isGoAhead) {
      goForward();
    }
    else if (commands.isGoBack) {
      goBackwards();
    }
    else if (commands.isTurnLeft) {
      goLeft();
    }
    else if (commands.isTurnRight) {
      goRight();
    }
  }

  if (commands.isSpeedModeChanged) {
    setSpeed(speeds[speedMode], speeds[speedMode]);
    commands.isSpeedModeChanged = false;
  }

}

/*
   Communication task.
   This task receives command from the serial and will call the corresponding management routine.
   It works also with bluetooth commands, since both are serial inputs.

   Before to use this task, please pair smartphone/PC with BT module.
   Also, bt   module led (HC-06) must be stable blue.
   The Android Pie app I used for bluetooth control is "Arduino bluetooth controller" by "Giumig Apps".

   Alternatively, debug can be performed via Arduino IDE Serial Monitor.
*/
void COMMUNICATION(250) {  
  static bool isDoneCom = false;

  if (!isDoneCom) {
    isDoneCom = true;
    sprintln ("Task waits for a command from bluetooth: ");
  }

  String actualCommand = getCommand();
  // sprintln(actualCommand);
  if (actualCommand != "" && actualCommand != "\n") {
    isDoneCom = false;
    actualCommand.trim();
    sprint ("Com. task got: "); sprintln (actualCommand);

    setNextAction(actualCommand);
    actualCommand = "";
  }

}

void setNextAction(String actualCommand) {
  bool repeatWhile = true;

  while (repeatWhile) {
    repeatWhile = false;
    if (mode == AUTO) {
      if (actualCommand == CMD_GO_AHEAD   ||
          actualCommand == CMD_GO_BACK    ||
          actualCommand == CMD_TURN_LEFT  ||
          actualCommand == CMD_TURN_RIGHT ) {
        mode = MANUAL;
        repeatWhile = true;
      }
      else if (actualCommand == CMD_STOP) {
        mode = STOPPED;
        resetCommands();
        commands.isStop = true;
      }
      else if (actualCommand == CMD_FASTER) {
        speedMode = FAST;
        commands.isSpeedModeChanged= true;
      }
      else if (actualCommand == CMD_SLOWER) {
        speedMode = SLOW;
        commands.isSpeedModeChanged= true;
      }
      else {
        resetCommands();
      }
    } // mode == AUTO
    else if (mode == MANUAL) {
      if (actualCommand == CMD_STOP) {
        mode = STOPPED;
        resetCommands();
        commands.isStop = true;
      }
      else if (actualCommand == CMD_GO_AHEAD) {
        resetCommands();
        commands.isGoAhead = true;
      }
      else if (actualCommand == CMD_GO_BACK) {
        resetCommands();
        commands.isGoBack = true;
      }
      else if (actualCommand == CMD_TURN_LEFT) {
        resetCommands();
        commands.isTurnLeft = true;
      }
      else if (actualCommand == CMD_TURN_RIGHT) {
        resetCommands();
        commands.isTurnRight = true;
      }
      else if (actualCommand == CMD_FASTER) {
        speedMode = FAST;
        commands.isSpeedModeChanged= true;
      }
      else if (actualCommand == CMD_SLOWER) {
        speedMode = SLOW;
        commands.isSpeedModeChanged= true;
      }
    } // mode == MANUAL
    else if (mode == STOPPED) {
      if (actualCommand == CMD_FOLLOW_LINE) {
        mode = AUTO;
        repeatWhile = true;
      }
      else if (actualCommand == CMD_GO_AHEAD   ||
               actualCommand == CMD_GO_BACK    ||
               actualCommand == CMD_TURN_LEFT  ||
               actualCommand == CMD_TURN_RIGHT ) { // go ahead, go back, turn left, turn right
        mode = MANUAL;
        repeatWhile = true;
      }
    } // mode == STOPPED
    else {
      sprintln("Warning, setNextAction() else branch. Unexpected command");
    } // else == mode

  } // while (repeatWhile)
}

/*
   Read the serial, and stands still until we receive a command
*/
String getCommand() {
  String msg  = "";

  while (serialInput.available()) {
    msg += char(serialInput.read());
  }

  return msg;
}

/*
   This function moves the rover along the path (the path is a black line) and sends to the serial the direction in which it's going.
   It tries to move in such a way the S3 sensor detects LOW, and the other sensors detect HIGH.

              SENSORS                                             Eg. Open path                                     Eg. Closed path
                ||                                                  . (END)                                        _______(START)/(END)
                S3                                                 /                                                 |       |
             S2 || S4                                              \__                                               |       |
           S1   ||   S5                                               \__.  (START)                                  |_______|

  The rover scheme is from above - you don't see your Arduino.
*/
void follow_line() {

  if (S5 == LOW && S1 == HIGH) {
    sprintln (CMD_TURN_RIGHT);
    goRight();
  }
  else if (S1 == LOW && S5 == HIGH) {
    sprintln (CMD_TURN_LEFT);
    goLeft();
  }
  else if (S1 == HIGH && S2 == LOW && S3 == LOW && S4 == LOW && S5 == HIGH) {
    sprintln (CMD_GO_AHEAD);
    goForward();
  }
  else if (S1 == LOW && S2 == LOW && S4 == LOW && S5 == LOW) {
    sprintln (CMD_STOP);
    stop();
  }
  
}

void loop() {
  
  if (arteEnabled() == 0 || arteSetupDone() == 0) {
    sprintln("Error, ARTe not enabled. Code not safe!");
  }
  delay(200);
  
}