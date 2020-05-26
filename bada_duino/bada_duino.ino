#include <DynamixelMotor.h>
#include <RGBmatrixPanel.h>
#include <ros.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Int16.h>

#define CLK 11
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3
#define D_ID 144  ///Dynmixel ID

unsigned long          previousTime = millis();
unsigned long          currentTime;
const long             timeInterval = 100; //ms
bool                   t_flag       = false;
char                   t_index      = 0;
float                  t_val        = 0;
int16_t speed = 125;                          // speed, between 0 and 1023
const long unsigned int DX_baudrate = 1000000;// communication baudrate

enum Events {
  NOTHING = 0,
  A_UP, A_DOWN, A_LEFT, A_RIGHT,
  FIRE_EVENT, WATER_EVENT, DOOR_EVENT, BELL_EVENT, BOILING_EVENT, CRYING_EVENT
};

const unsigned char PROGMEM ARROW_UP[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xc0, 0x00,
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x3f, 0xfc, 0x00,
  0x00, 0x7f, 0xfe, 0x00, 0x00, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80,
  0x01, 0xff, 0xff, 0x80, 0x01, 0xf7, 0xef, 0x80, 0x01, 0xe7, 0xe7, 0x80, 0x01, 0xc7, 0xe3, 0x80,
  0x01, 0x87, 0xe1, 0x80, 0x01, 0x07, 0xe0, 0x80, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00,
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00,
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00,
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM ARROW_DOWN[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00,
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00,
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00,
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x01, 0x07, 0xe0, 0x80, 0x01, 0x87, 0xe1, 0x80,
  0x01, 0xc7, 0xe3, 0x80, 0x01, 0xe7, 0xe7, 0x80, 0x01, 0xf7, 0xef, 0x80, 0x01, 0xff, 0xff, 0x80,
  0x01, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xfe, 0x00,
  0x00, 0x3f, 0xfc, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x07, 0xe0, 0x00,
  0x00, 0x03, 0xc0, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM ARROW_RIGHT[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xfc, 0x0, 0x0, 0x1, 0xfe, 0x0,
  0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x7f, 0x80, 0x0, 0x0, 0x3f, 0xc0,
  0x0, 0x0, 0x1f, 0xe0, 0x3f, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xf8,
  0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xf8,
  0x3f, 0xff, 0xff, 0xf0, 0x0, 0x0, 0x1f, 0xe0, 0x0, 0x0, 0x3f, 0xc0,
  0x0, 0x0, 0x7f, 0x80, 0x0, 0x0, 0xff, 0x0, 0x0, 0x1, 0xfe, 0x0,
  0x0, 0x3, 0xfc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

const unsigned char PROGMEM ARROW_LEFT[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x3f, 0xc0, 0x0, 0x0, 0x7f, 0x80, 0x0,
  0x0, 0xff, 0x0, 0x0, 0x1, 0xfe, 0x0, 0x0, 0x3, 0xfc, 0x0, 0x0,
  0x7, 0xf8, 0x0, 0x0, 0xf, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xfc,
  0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xfc,
  0xf, 0xff, 0xff, 0xfc, 0x7, 0xf8, 0x0, 0x0, 0x3, 0xfc, 0x0, 0x0,
  0x1, 0xfe, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0, 0x0, 0x7f, 0x80, 0x0,
  0x0, 0x3f, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

const unsigned char PROGMEM FIRE1[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00,
  0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x39, 0xfc, 0x00,
  0x00, 0x79, 0xfc, 0x00, 0x00, 0x73, 0xfc, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff, 0xfd, 0x00,
  0x00, 0xff, 0xfd, 0x80, 0x00, 0xff, 0xff, 0x80, 0x00, 0xfc, 0xff, 0xc0, 0x00, 0xfc, 0xff, 0xc0,
  0x02, 0x7c, 0x1f, 0xe0, 0x06, 0x7c, 0x3f, 0xe0, 0x06, 0x7c, 0x3f, 0xf0, 0x07, 0xf8, 0x3f, 0xf0,
  0x07, 0xf8, 0x1f, 0xf0, 0x07, 0xf0, 0x17, 0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0xf0,
  0x07, 0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x07, 0xf0, 0x03, 0xf8, 0x0f, 0xe0,
  0x01, 0xfc, 0x1f, 0xc0, 0x01, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x1f, 0xfc, 0x00,

};
const unsigned char PROGMEM FIRE2[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00,
  0x00, 0x1e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x3f, 0x10, 0x00, 0x00, 0x3f, 0x9c, 0x00,
  0x00, 0x3f, 0x9e, 0x00, 0x00, 0x3f, 0xce, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00, 0xbf, 0xff, 0x00,
  0x01, 0xbf, 0xff, 0x00, 0x01, 0xff, 0xff, 0x00, 0x03, 0xff, 0x3f, 0x00, 0x03, 0xff, 0x3f, 0x00,
  0x07, 0xf8, 0x3e, 0x40, 0x07, 0xfc, 0x3e, 0x60, 0x0f, 0xfc, 0x3e, 0x60, 0x0f, 0xfc, 0x1f, 0xe0,
  0x0f, 0xf8, 0x1f, 0xe0, 0x0f, 0xe8, 0x0f, 0xe0, 0x0f, 0xe0, 0x0f, 0xe0, 0x0f, 0xe0, 0x0f, 0xe0,
  0x0f, 0xe0, 0x0f, 0xe0, 0x0f, 0xe0, 0x0f, 0xe0, 0x0f, 0xe0, 0x0f, 0xe0, 0x07, 0xf0, 0x1f, 0xc0,
  0x03, 0xf8, 0x3f, 0x80, 0x01, 0xff, 0xff, 0x80, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xf8, 0x00
};

std_msgs::Bool CameraState_msg;
std_msgs::Bool DisplayState_msg;
std_msgs::Bool ButtonState_msg;

bool Camera_cmd = false, Button_cmd = false;
int Display_cmd = 0; // need convert function

void Dynamixel_startUP();
void LEDMatrix_startUP();
void LEDMatrix_Control(Events event);
void MotorCtrl(bool cmd);
float VelocityCtrl(int pos1, int pos2, float t);

//callback
void CameraCommand(const std_msgs::Bool& cmd);
void DisplayCommand(const std_msgs::Int16& cmd);

ros::Publisher CameraState ("Cameara_State", &CameraState_msg);
ros::Publisher DisplayState("Display_State", &DisplayState_msg);
ros::Publisher ButtonState ("Button_State", &ButtonState_msg);
ros::Subscriber<std_msgs::Bool> CameraSUB("bada/duino/camera_cmd", &CameraCommand);
ros::Subscriber<std_msgs::Int16> DisplaySUB("bada/duino/display_cmd", &DisplayCommand );



HardwareDynamixelInterface interface(Serial2);// Serial1 -- RX2 TX2, Serial -- RX1 TX1

DynamixelMotor motor(interface, D_ID);
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

ros::NodeHandle  nh;

void setup() {
  nh.initNode();
  
  nh.advertise(CameraState);
  nh.advertise(DisplayState);
  nh.advertise(ButtonState);
  
  nh.subscribe(CameraSUB);
  nh.subscribe(DisplaySUB);
  
  Dynamixel_startUP();
  LEDMatrix_startUP();
  motor.goalPosition(205);
  delay(1000);
  
  nh.loginfo("BADA_DUINO ON");
}

void loop() {
  T_ISR();
  currentTime = millis();

  if (t_flag) {
    t_flag = false;
    switch (t_index) {
      case 0:
        nh.spinOnce();
        t_index = 1;
        break;
      case 1:
        t_index = 2;
      case 2:
        ButtonState.publish(&ButtonState_msg);
        t_index = 3;
        break;
      case 3:
        t_index = 4;
        break;
      case 4:
        t_index = 5;
        DisplayState.publish(&DisplayState_msg);
        if (Display_cmd >= 0) LEDMatrix_Control(static_cast<Events>(Display_cmd));
        break;
      case 5:
        t_index = 6;
        break;
      case 6:
        CameraState.publish(&CameraState_msg);
        t_index = 7;
        break;
      case 7:
        t_index = 8;
        break;
      case 8:
        t_index = 9;
        //LEDMatrix_Control(FIRE_EVENT);
        break;
      case 9:
        t_index = 0;
        break;
      default:
        t_index = 0;
        break;
    }//close switch
  }
  if (Camera_cmd == true) MotorCtrl(Camera_cmd);
  else MotorCtrl(Camera_cmd);
  if (Button_cmd == true); //motion that go backward, turn 180 and wait. or following situation.
}

void CameraCommand (const std_msgs::Bool& cmd) {
  Camera_cmd = cmd.data;
}

void DisplayCommand(const std_msgs::Int16& cmd) {
  Display_cmd = cmd.data;
}

void Dynamixel_startUP() {
  interface.begin(DX_baudrate);
  delay(200);
  uint8_t status = motor.init();
  while (status != DYN_STATUS_OK) { //Failure check
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    nh.loginfo("Dynamixel is not conected ");
    delay(1000);
  }

  motor.enableTorque();  // joint mode 180° angle range
  motor.jointMode(204, 820);  //Change this code after Design.
  motor.speed(speed);
}

void T_ISR() {
  if (currentTime - previousTime > timeInterval) {
    previousTime = currentTime;
    t_flag = true;
  }
}

void LEDMatrix_startUP() {
  matrix.begin();
  delay(100);
}

void LEDMatrix_Control(Events event) {
  static Events current = -1;

  if (current != event){
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    current = event;
  }
  // A_UP,A_DOWN, A_LEFT, A_RIGHT,
  // FIRE_EVENT, WATER_EVENT, DOOR_EVENT, BELL_EVENT, BOILING_EVENT, CRYING_EVENT

  switch (event) {
    case NOTHING : 
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      break;
    case A_UP : 
      matrix.drawBitmap(0, 0,  ARROW_UP, 32, 32, matrix.Color333(3, 7, 1));
      break;
    case A_DOWN : 
      matrix.drawBitmap(0, 0,  ARROW_DOWN, 32, 32, matrix.Color333(3, 7, 1));
      break;
    case A_LEFT:
      matrix.drawBitmap(0, 0,  ARROW_LEFT, 32, 32, matrix.Color333(3, 7, 1));
      break;
    case A_RIGHT:
      matrix.drawBitmap(0, 0,  ARROW_RIGHT, 32, 32, matrix.Color333(3, 7, 1));
      break;
    case FIRE_EVENT:
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      matrix.drawBitmap(0, 0,  FIRE1, 32, 32, matrix.Color333(7, 0, 0));
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      matrix.drawBitmap(0, 0,  FIRE2, 32, 32, matrix.Color333(7, 0, 0));
      break;
  }
}

void LEDMatrix_Erasing() {
  matrix.fillScreen(matrix.Color333(0, 0, 0));
}

void MotorCtrl(bool cmd) {
  if (cmd) motor.goalPosition(500); // Changed!
  else  motor.goalPosition(210);    // Defalut
}
