#include <Adafruit_PWMServoDriver.h>
#include <DigiPotX9Cxxx.h>
#include <Pixy2.h>
#include <PIDLoop.h>

#define SERVO_FREQ 50   //  Analog servos run at ~50 Hz updates

//Variables needed to store values
const int USPin = 7;                    //Pin for Ultrasonic Sensor
long pulse, inches, cm;
int shieldPin=analogRead(A0);
// Initialize Servo Shield Methods
Adafruit_PWMServoDriver servoShield = Adafruit_PWMServoDriver();
// Potentiometer Setup
//DigiPot digiPot(2,3,4);                 //*****incomplete setup as of now*******
Pixy2 pixy;
PIDLoop panLoop(1200, 0, 1200, true);   // changes servo track speed
PIDLoop tiltLoop(800, 0, 800, true);    // changes servo track speed




void setup()
{
  Serial.begin(57600);
  Serial.print("Starting...\n");
  // Initialize Servo Shield
  servoShield.begin();
  servoShield.setPWMFreq(SERVO_FREQ);
  // We need to initialize the pixy object
  pixy.init();
  // Use color connected components program for the pan tilt to track
  pixy.changeProg("color_connected_components");
}

void loop()
{  
  static int i = 0;
  int j;
  char buf[64];
  int32_t panOffset, tiltOffset;
  
  //UltraSonic SENSOR CODE
  pinMode(USPin, INPUT);
  //Used to read in the pulse that is being sent by the MaxSonar device.
  //Pulse Width representation with a scale factor of 147 uS per Inch.
  pulse = pulseIn(USPin, HIGH);
  //147uS per inch
  inches = pulse / 147;
  //change inches to centimetres
  cm = inches * 2.54;
 /* Serial.print(inches);                      //Serial Print for verification of sensor data
  Serial.print("in, ");                      //Serial Print for verification of sensor data
  Serial.print(cm);                          //Serial Print for verification of sensor data
  Serial.print("cm");                        //Serial Print for verification of sensor data
  Serial.println();                          //Serial Print for verification of sensor data
 */
  Serial.print(shieldPin);
  if ((inches > 12) && (inches < 42)){
    //PIXY Pan-Tilt CODE
    // get active blocks from Pixy
    pixy.ccc.getBlocks();
 
    if (pixy.ccc.numBlocks)
    {        
      i++;
    
      if (i%60==0)
        Serial.println(i);  
   
      // calculate pan and tilt "errors" with respect to first object (blocks[0]),
      // which is the biggest object (they are sorted by size).  
      panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)pixy.ccc.blocks[0].m_x;
      tiltOffset = (int32_t)pixy.ccc.blocks[0].m_y - (int32_t)pixy.frameHeight/2;  
 
      // update loops
      panLoop.update(panOffset);
      tiltLoop.update(tiltOffset);
 
      // set pan and tilt servos  
      shieldPin = pixy.setServos(panLoop.m_command, tiltLoop.m_command);
      servoShield.setPWM(0, 0, shieldPin);
   
#if 0 // for debugging
    sprintf(buf, "%ld %ld %ld %ld", rotateLoop.m_command, translateLoop.m_command, left, right);
    Serial.println(buf);  
#endif  
    } 
  } 
  else // no object detected, go into reset state
  {
    panLoop.reset();
    tiltLoop.reset();
    shieldPin = pixy.setServos(panLoop.m_command, tiltLoop.m_command);
    servoShield.setPWM(0, 0, shieldPin);
  }
}
