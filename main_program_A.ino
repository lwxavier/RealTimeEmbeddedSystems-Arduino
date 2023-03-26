
// Define a constant value ALPHA for use in pixel collision detection.
const uint16_t ALPHA = 0x1111;

// Include the required libraries and header file.
#include "flappyBirdSprites.h"

#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi101.h>
#include "secrets.h"
#include <ctype.h>
#include <Servo.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
// WiFi parameters
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

#define WIFI_DELAY 2000 // attempt connection every 2 seconds
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
// Instantiate a TinyScreen object.
TinyScreen display = TinyScreen(TinyScreenPlus);
#define BRIGHTNESS 10 // set brightness at level 10
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// (1/3) Start of code excerpt from http://www.geekmomprojects.com/tinyscreen-case-with-buttons/
#define SCREEN_WIDTH    96
#define SCREEN_HEIGHT   64
#define DEGREES_TO_RADIANS 3.14159/180

// RGB colors 
#define N_LIGHT_COLORS  5
uint8_t light_colors[][3] = { {255, 224, 219}, {255, 238, 219}, {255, 233, 219}, {244, 255, 219}, {219, 255, 238}, {100, 219, 241}, {219, 223, 255}, {255, 255, 255} };
   
#define N_DARK_COLORS   8                    
uint8_t dark_colors[][3] =  {{0, 0, 0}, {82, 51, 21}, {45, 82, 21}, {21, 82, 74}, {21, 57, 82}, {67, 21, 82}, {82, 21, 42}, {255, 255, 255}};                    

#define BLACK           0x00
#define BLUE            0xE0
#define RED             0x03
#define GREEN           0x1C
#define WHITE           0xFF
#define GREY            0x6D
#define YELLOW          0x1F
#define BROWN           0x32

int screen_center[2] = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
int delay_time = 100;

class Polygon {
  private: 
    int num_nodes;
    int num_vertices;
    int (*nodes)[3];
    int (*vertices)[2];
    
  public:
  
    Polygon(int n_nodes, int node_ptr[][3], int n_vertices, int vertex_ptr[][2]) {
      num_nodes = n_nodes;
      nodes = node_ptr;
      num_vertices = n_vertices;
      vertices = vertex_ptr;
    }
    
   
  void copy_nodes(int new_nodes[][3]) {
    for (int i = 0; i < num_nodes; i++) {
      new_nodes[i][0] = nodes[i][0];
      new_nodes[i][1] = nodes[i][1];
      new_nodes[i][2] = nodes[i][2];
    }
  }
      
  void print_nodes() {
    SerialUSB.print("Node example: ");
    for (int i = 0; i < num_nodes; i ++) {
      SerialUSB.print(nodes[i][0]);
      SerialUSB.print(", ");
      SerialUSB.print(nodes[i][1]);
      SerialUSB.print(", ");
      SerialUSB.println(nodes[i][2]);
    }
  }
  // Yaw, pitch, roll in degrees.  Scale can run from 0 to 1
  void rotate3D(float yaw, float pitch, float roll, int new_nodes[][3]) {
    double sin_y = sin(yaw*DEGREES_TO_RADIANS);
    double cos_y = cos(yaw*DEGREES_TO_RADIANS);
    double sin_p = sin(pitch*DEGREES_TO_RADIANS);
    double cos_p = cos(pitch*DEGREES_TO_RADIANS);
    double sin_r = sin(roll*DEGREES_TO_RADIANS);
    double cos_r = cos(roll*DEGREES_TO_RADIANS);
        
    // Yaw first, then pitch, then roll
    for (int i = 0; i < num_nodes; i++) {
      float x, y, z;
      float xp, yp, zp;
      
      x = new_nodes[i][0];
      y = new_nodes[i][1];
      z = new_nodes[i][2];
 
      // Rotate yaw
      xp = x*cos_y - y*sin_y;
      yp = y*cos_y + x*sin_y;
      x = xp;
      y = yp;
      
      // Rotate pitch
      xp = x*cos_p - z*sin_p;
      zp = z*cos_p + x*sin_p;
      x = xp;
      z = zp;
      
      // Rotate roll
      yp = y*cos_r - z*sin_r;
      zp = z*cos_r + y*sin_r;
      y = yp;
      z = zp;

      new_nodes[i][0] = (int) (x + 0.5);
      new_nodes[i][1] = (int) (y + 0.5);
      new_nodes[i][2] = (int) (z + 0.5);        
    }
  } 

  void draw(TinyScreen disp, int new_nodes[][3]) {
    uint8_t x0, y0, x1, y1;
    for (int i = 0; i < num_vertices; i++) {
      x0 = (uint8_t) new_nodes[vertices[i][0]][0] + screen_center[0];
      x1 = (uint8_t) new_nodes[vertices[i][1]][0] + screen_center[0];
      y0 = (uint8_t) new_nodes[vertices[i][0]][1] + screen_center[1];
      y1 = (uint8_t) new_nodes[vertices[i][1]][1] + screen_center[1];

      disp.drawLine(x0, y0, x1, y1, (uint16_t) i*5+1); 
    }
  };
    
};

// Cube
#define CUBE_SCALE  18
int cube_nodes[8][3] = {
  {-CUBE_SCALE, -CUBE_SCALE, -CUBE_SCALE},
  {-CUBE_SCALE, -CUBE_SCALE,  CUBE_SCALE},
  {-CUBE_SCALE,  CUBE_SCALE, -CUBE_SCALE},
  {-CUBE_SCALE,  CUBE_SCALE,  CUBE_SCALE},
  {CUBE_SCALE, -CUBE_SCALE, -CUBE_SCALE},
  {CUBE_SCALE, -CUBE_SCALE,  CUBE_SCALE},
  {CUBE_SCALE,  CUBE_SCALE, -CUBE_SCALE},
  {CUBE_SCALE,  CUBE_SCALE,  CUBE_SCALE}
};

int cube_vertices[12][2] = {
  {0, 1}, {1,3}, {3,2}, {2,0}, {4,5}, {5,7}, {7,6}, {6,4}, {0,4}, {1,5}, {2,6}, {3,7}
};

// Pyramid
#define PYR_SCALE 16
int pyramid_nodes[5][3] = {
  {PYR_SCALE, PYR_SCALE, -PYR_SCALE},
  {PYR_SCALE, -PYR_SCALE, -PYR_SCALE},
  {-PYR_SCALE, -PYR_SCALE, -PYR_SCALE},
  {-PYR_SCALE, PYR_SCALE, -PYR_SCALE},
  {0, 0, PYR_SCALE}
};

int pyramid_vertices[8][2] = {
  {0,1}, {1,2}, {2,3}, {3,0}, {0,4}, {1,4}, {2,4}, {3,4}
};

// Octahedron
#define OCT_SCALE 16
int octohedron_nodes[6][3] = {
  {OCT_SCALE, OCT_SCALE, 0},
  {OCT_SCALE, -OCT_SCALE, 0},
  {-OCT_SCALE, -OCT_SCALE, 0},
  {-OCT_SCALE, OCT_SCALE, 0},
  {0, 0, OCT_SCALE},
  {0, 0, -OCT_SCALE}
};

int octohedron_vertices[12][2] = {
  {0,1}, {1,2}, {2,3}, {3,0}, {0,4}, {1,4}, {2,4}, {3,4}, {0,5}, {1,5}, {2,5}, {3,5}
};


Polygon cube(8, cube_nodes, 12, cube_vertices);
Polygon pyramid(5, pyramid_nodes, 8, pyramid_vertices);
Polygon octohedron(6, octohedron_nodes, 12, octohedron_vertices);

// Make sure this is at least as large as the largest number of nodes in a single shape
int new_nodes[8][3];
// (1/3) End of code excerpt from http://www.geekmomprojects.com/tinyscreen-case-with-buttons/
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// (2/3) Start of code excerpt from http://www.geekmomprojects.com/tinyscreen-case-with-buttons/
float angle_y = 0;
float angle_p = 0;
float angle_r = 0;
float inc_y = 2;
float inc_p = 3;
float inc_r = 4;

#define NPOLS 3
Polygon* pol_list[NPOLS] = {&cube, &pyramid, &octohedron};
int pol_index = 0;

int col_index = 0;

boolean buttonPressed = false;
// (2/3) End of code excerpt from http://www.geekmomprojects.com/tinyscreen-case-with-buttons/
//-------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------------------------------
// Flappy Bird codes
//
// Define the memory structure for a sprite.
typedef struct
{
  int x;                       // x-coordinate of sprite
  int y;                       // y-coordinate of sprite
  int width;                   // width of sprite in pixels
  int height;                  // height of sprite in pixels
  int bitmapNum;               // identifier for the sprite bitmap
  const unsigned int *bitmap;  // pointer to the sprite bitmap

} ts_sprite;

// Define the memory structure for a string.
typedef struct
{
  int x;                 // x-coordinate of string
  int y;                 // y-coordinate of string
  int height;            // height of string in pixels
  char stringChars[40];  // characters in the string

} ts_string;

// Declare function prototypes for sprite collision detection.
bool testBitmapCollision(ts_sprite *s1, ts_sprite *s2);
bool testPixelCollision(ts_sprite *s1, ts_sprite *s2);

// Define global variables for game state and mechanics.
int collisionDetected = 0;  // indicates if a collision has occurred
int wingChangeCount = 8;    // counter for bird wing animation
int wingPos = 1;            // position of bird wing in animation
int pipeOffsetX = 5;        // x-offset for pipe generation
int pipeSpacingX = 34;      // x-spacing between pipes
int movePipe = 4;           // speed at which pipes move across the screen
int movePipeMod = 1;        // modulator for pipe movement speed
int pipeSpacingY = 32;      // y-spacing between pipes
int frame = 0;              // counter for game frames
int cloudSpacingX = 50;     // x-spacing between clouds
int cloudOffsetX = 0;       // x-offset for cloud generation
int speedUpBoxActive = 0;   // indicates if the speed-up box is active
int speedUpBoxHit = 0;      // indicates if the speed-up box has been hit
int speedUp = 0;            // indicates if the speed-up effect is active
int slowBoxActive = 0;      // indicates if the slow box is active
int slowBoxHit = 0;         // indicates if the slow box has been hit
int slowUp = 0;             // indicates if the slow effect is active
int closeBoxActive = 0;     // indicates if the close box effect is active
int closeBoxHit = 0;        // indicates if the bird hit the close box
int doCloseBox = 0;         // indicates if the close box effect should be triggered
int darkBoxActive = 0;      // indicates if the dark box effect is active
int darkBoxHit = 0;         // indicates if the bird hit the dark box
int doDarkBox = 0;          // indicates if the dark box effect should be triggered
int heartBoxActive = 0;     // indicates if the heart box effect is active
int heartBoxHit = 0;        // indicates if the bird hit the heart box

// Declaration of integer constants
int defaultBrightness = 10;  // default screen brightness
int currentBrightness = 10;  // current screen brightness
int startScreen = 1;         // indicates if the game should start or not

// Declaration of sprite variables
ts_sprite flappyBird = { -25, 22, 17, 12, 0, flappyBirdBitmap };             // the main character
ts_sprite wing = { -25, 23, 7, 8, 0, wingBitmap };                           // the wings of the character
ts_sprite pipeUp0 = { 0, 40, 12, 40, 0, greenPipeUpBitmap };                 // the first upper pipe
ts_sprite pipeUp1 = { 0, 40, 12, 40, 0, greenPipeUpBitmap };                 // the second upper pipe
ts_sprite pipeUp2 = { 0, 40, 12, 40, 0, greenPipeUpBitmap };                 // the third upper pipe
ts_sprite pipeUp3 = { 0, 40, 12, 40, 0, greenPipeUpBitmap };                 // the fourth upper pipe
ts_sprite pipeDown0 = { 0, -pipeSpacingY, 12, 40, 0, greenPipeDownBitmap };  // the first lower pipe
ts_sprite pipeDown1 = { 0, -pipeSpacingY, 12, 40, 0, greenPipeDownBitmap };  // the second lower pipe
ts_sprite pipeDown2 = { 0, -pipeSpacingY, 12, 40, 0, greenPipeDownBitmap };  // the third lower pipe
ts_sprite pipeDown3 = { 0, -pipeSpacingY, 12, 40, 0, greenPipeDownBitmap };  // the fourth lower pipe
ts_sprite cloud0 = { 55, 0, 15, 10, 0, cloudBitmap };                        // the first cloud
ts_sprite cloud1 = { 55, 15, 15, 10, 0, cloudBitmap };                       // the second cloud
ts_sprite cloud2 = { 55, 30, 15, 10, 0, cloudBitmap };                       // the third cloud
ts_sprite cloud3 = { 55, 30, 15, 10, 0, cloudBitmap };                       // the fourth cloud
ts_sprite ground = { 0, 52, 105, 12, 0, groundBitmap };                      // the ground
ts_sprite speedBox = { -10, -10, 10, 10, 0, speedUpBoxBitmap };              // the speed up box
ts_sprite closeBox = { -10, -10, 10, 10, 0, closeBoxBitmap };
ts_sprite darkBox = { -10, -10, 10, 10, 0, darkBoxBitmap };
ts_sprite slowBox = { -10, -10, 10, 10, 0, slowBoxBitmap };
ts_sprite heartBox = { -10, -10, 10, 10, 0, heartBoxBitmap };
ts_sprite title = { 0, -60, 96, 56, 0, flappyTitle };
ts_sprite hearts = { -35, 2, 70, 5, 0, heartBitmap };

int lives = 0;
ts_sprite *spriteList[22] = { &cloud0, &cloud1, &cloud2, &cloud3, &pipeUp0, &pipeDown0, &pipeUp1, &pipeDown1, &pipeUp2, &pipeDown2, &pipeUp3, &pipeDown3, &ground, &hearts, &closeBox, &speedBox, &darkBox, &heartBox, &slowBox, &flappyBird, &wing, &title };

int amtSprites = 22;

int highScore = 0;
int currentScore = 0; // the score of the player
int showScore = 0;
ts_string score = { 0, -20, 0, "0" };
//End Flappy Bird codes
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Main Program

const int ldrPin = A1; // analog pin used to connect the LDR sensor 
int ldrVal = 0;
int room_ldrVal = 0;
Servo myServo;

int patientInputFieldScore = 0;
int patientInputFieldLOAD = 0;
int levelSelected = 0;
int difficultySelected = 0;



int x_pos = 10; // for cursor
int y_pos = 10; // for cursor
//--------------------------------------------------------------------------
//
// State Machine values:
//--------------------------------------------------------------------------
const byte Loading_Page        = 0;
const byte Ready               = 1;
const byte Difficulty_Selected = 2;
const byte Countdown_5s        = 3;
const byte Game_Start          = 4; 
const byte Finish_Results      = 5;
const byte Pause               = 6;
const byte Continue_15s        = 7;

byte current_state;
//--------------------------------------------------------------------------


String rawUserData = "";
String patientName = "";

int userStatus = 5; // 5 is a random value at the beginning

int final_score = 0; // initialise final score
int load_value = 0; // initialise load value


unsigned long previousMillis = 0; // initialise timer variable

unsigned long poll_millis = 0; // initialise pause poll variable

unsigned long exit_pause = 0; // initialise exit timer variable

unsigned long time_paused = 0; // to store total time paused 

unsigned long start_time_paused = 0; // to store time for beginning of pause

// debouncing variables
long unsigned int lastPress;
int debounceTime = 20;


// for Flappy Bird game
int counter = 0;
int fall_speed = 3; 


// for flapping with ldr
unsigned long interval_time = 1000; //1000ms before flapping can happen again 
unsigned long flap_millis = 0; 
unsigned long clum_millis = 0;

#define GAME_DURATION 60000 // time in ms. 60 seconds game time

float RPMmultiplier = RPMmultiplier = (60000 / GAME_DURATION); //60000ms in a minute



//RPM variables
// const int NUM_READINGS = 60;  // the number of readings to use for calculating RPM
// const int RPM_PERIOD_MS = 1000;  // the time period (in ms) for calculating RPM
// unsigned long lastReadings[NUM_READINGS] = {0};  // an array to store the last few readings
int numRevs = 0;  // the number of readings in the lastReadings array
int patientRPM = 0;   // the global variable for storing the patient's RPM



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void setup() {

  Wire.begin();

// Serial USB setup  
  SerialUSB.begin(57600);
  while (!SerialUSB) {
    ; // wait for serial port to connect
  }
  SerialUSB.println("\n________________________________________________________________");
  SerialUSB.println("Power ON"); 

  WiFi.setPins(8, 2, A3, -1); // VERY IMPORTANT FOR TINYDUINO
 
//-------------------------------------------------------------------------------------------------
//
// TinyScreen setup parameters
//-------------------------------------------------------------------------------------------------      
  display.begin();
  display.setBrightness(BRIGHTNESS);
  display.setFont(thinPixel7_10ptFontInfo);
  display.fontColor(TS_8b_White,TS_8b_Black); // (text_color,background_color)
  display.setFlip(false); // comment out if screen is flipped
  
  display.setBitDepth(TSBitDepth8); // Normal Bit Depth is 8 bit
  //display.setBitDepth(TSBitDepth16); // Bit Depth for Flappy Bird is 16 bit
  
  //display.setFlip(false);
//---------------------------------------------------------------------------  

  ThingSpeak.begin(client);  // Initialize ThingSpeak

  
  start_message();
	myServo.attach(9);  // attaches the servo on pin 9 to the servo object  
                      // update pin number for servo again
  
  analogWrite(8, 255); // For LDR, full duty cycle                                          

  current_state = Loading_Page; // State machine goes to Loading_Page

  rawUserData.reserve(200); // put in setup!! impt


// Interrupt Service Routine (ISR) setup parameters
//-------------------------------------------------------------------------------------------------      
  pinMode(19, INPUT_PULLUP);  // set the button pins as an input with internal pull-up resistor enabled
  //pinMode(25, INPUT_PULLUP);
  //pinMode(30, INPUT_PULLUP);
  //pinMode(31, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(19), button_isr, FALLING);  // attach interrupt to the falling edge of any button pin
  //attachInterrupt(digitalPinToInterrupt(25), button_isr, FALLING);
  //attachInterrupt(digitalPinToInterrupt(30), button_isr, FALLING);
  //attachInterrupt(digitalPinToInterrupt(31), button_isr, FALLING);

}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/*
  //this is for flapping with TinyScreen buttons
  int getInput() {
    if (display.getButtons(){
      return 1;
    }
  }
*/

// this is for flapping with ldr
int getInput(){ //interval_time, flap_millis, clum_millis
  //flapping with buttons
  if (display.getButtons()){
    return 1;
  }

  updateRPM();

  ldrVal = analogRead(ldrPin);
  if (ldrVal - room_ldrVal > 150){
    //prevent it from reading ldr too often
    //such that the bird will only flap one time per instance
    //also prevents 'debouncing'
    int clum_millis = millis();

    if (clum_millis - flap_millis > interval_time){ //interval_time is 1000ms
      flap_millis = millis();

      //for testing
      //SerialUSB.println(ldrVal);
      return 1;
    }
  }
  else{
    return 0;
  }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

// volatile flags
volatile int loading_flag        = 0; // Loading Screen
volatile int wifi_flag           = 0; // WiFi Connection
volatile int sensorChecks_flag   = 0; // Sensor Checks
volatile int TS_flag             = 0; // User Login
volatile int diffSelect_flag     = 0; // Difficulty Selection
volatile int countdown_flag      = 0; // Countdown flag
volatile int gameOver_flag       = 0; // To see if game is over
volatile int flappyBitDepth_flag = 0; // 16 bit depth for Flappy Bird
volatile int normalBitDepth_flag = 0; // 8 bit depth normally
volatile int screenReset_flag    = 0; // To reset the screen
volatile int servo_flag          = 0; // check servo 
volatile int ldr_flag            = 0; // check ldr
volatile int pause_flag          = 0; // check if should pause
volatile int resume_flag         = 0; // check if can resume
volatile int easyMode_flag       = 0; // check if in easy mode
volatile int hardMode_flag       = 0; // check if in hard mode

//-- timers --
volatile int x5s_flag             = 0; // timer of 5s
volatile int x10s_flag            = 0; // timer of 10s
volatile int x15s_flag            = 0; // timer of 15s
volatile int x60s_flag            = 0; // timer of 60s
volatile int x500ms_flag          = 0; // timer of 500ms
volatile int exitTimer_flag       = 1; // timer for exiting pause sequence

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


void button_isr(){
  pause_flag = 1;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void loop(){
  main_program();
}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// for testing 
/*
  void loop(){

    connect_to_wifi();
    //getUserData();
    //FlappyBird();
    //delay(10000);
    //screenFadeOut();
    //screenFadeIn();
    // for testing
    int gamestart = ThingSpeak.writeField(machineChannelNumber, 1, 0, machineWriteAPIKey);

    ReadyScreen();

    checkUserReady();
    getUserData();
    welcomeUser();
    
    delay(3000);
    showDiff();
    delay(3000);
    countdown_5s();



    previousMillis = millis(); // start timing total game time

    poll_millis = millis();

    SerialUSB.println("\nGame begin");

    time_paused = 0;
    
    while (gameOver_flag != 1){ // game over when elapsed time is 60s
      FlappyBird();

      if (x60s_flag != 1){
        x60s_elapsed();
      }
      gameOver_flag = x60s_flag; // currently 20s game time
      
      pause_poll();
                
    }

    resetScreen(); //resets the Bit Depth set for flappy bird
    normalMode(); // To show display.print properly

    mainLoopFlags_reset(); //resets relevant flags for main loop

    resultsScreen();
    
    delay(15000);

  }
*/
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


// Bit Depth Modes
//-------------------------------------------------------------------------------------------------      
void flappyMode(){
  if (flappyBitDepth_flag != 1){
    display.setBitDepth(TSBitDepth16); // Bit Depth for Flappy Bird is 16 bit
    flappyBitDepth_flag = 1;    
    normalBitDepth_flag = 0;
  }  
}

void normalMode(){
  if (normalBitDepth_flag != 1){
    display.setBitDepth(TSBitDepth8); // Bit Depth is normally 8 bit
    flappyBitDepth_flag = 0;    
    normalBitDepth_flag = 1;
  }  
}
//-------------------------------------------------------------------------------------------------      
//
//
//-------------------------------------------------------------------------------------------------      
void FlappyBird() { // screenReset_flag

  if (screenReset_flag != 1){
    resetScreen(); // required otherwise FlappyBird will not load properly
    flappyMode();
    screenReset_flag = 1;
  }


  slowBoxActive = 1;

  frame++;  //increment the frame count

  ground.x--;                       //move the ground to the left by 1 pixel
  if (ground.x < -4) ground.x = 0;  //if the ground has moved too far left, reset its position to the right

  //if the first pipe has moved off the screen to the left
  if (pipeUp0.x + pipeUp0.width < 0) {
    //set the position of the first pipe to the position of the second pipe
    pipeUp0.x = pipeUp1.x;
    pipeUp0.y = pipeUp1.y;
    pipeDown0.x = pipeDown1.x;
    pipeDown0.y = pipeDown1.y;
    //set the position of the second pipe to the position of the third pipe
    pipeUp1.x = pipeUp2.x;
    pipeUp1.y = pipeUp2.y;
    pipeDown1.x = pipeDown2.x;
    pipeDown1.y = pipeDown2.y;
    //set the position of the third pipe to the position of the fourth pipe
    pipeUp2.x = pipeUp3.x;
    pipeUp2.y = pipeUp3.y;
    pipeDown2.x = pipeDown3.x;
    pipeDown2.y = pipeDown3.y;

//level
    int level = levelSelected;
    int minPipeHeight = 46; // 46 is minimum allowable height, otherwise the user can just glide on ground
    int maxPipeHeight = minPipeHeight + 6; // variance of 6 is ideal

    //set the position of the fourth pipe to the right of the screen, with a random height within the range for the current level
    pipeUp3.x = pipeUp2.x + pipeSpacingX - doCloseBox;
    pipeDown3.x = pipeDown2.x + pipeSpacingX - doCloseBox;

    if (level == 1) {
      //set the height of the pipes for level 2

      fall_speed = 5; //slowest fall setting

      minPipeHeight = 46;
      maxPipeHeight = minPipeHeight + 4; // variance of 6 is ideal
            
      pipeUp3.y = minPipeHeight + micros() % (maxPipeHeight - minPipeHeight);
    } 
    
    else if (level == 2) {
      //set the height of the pipes for level 2

      fall_speed = 4; //medium fall setting


      minPipeHeight = 40;
      maxPipeHeight = minPipeHeight + 7; 

      pipeUp3.y = minPipeHeight + micros() % (maxPipeHeight - minPipeHeight);
    } 
    
    else if (level == 3) {
      //set the height of the pipes for level 3

      fall_speed = 4; //fastest fall setting

      minPipeHeight = 34;
      maxPipeHeight = minPipeHeight + 10; // variance of 6 is ideal
      pipeUp3.y = minPipeHeight + micros() % (maxPipeHeight - minPipeHeight);
    }

    pipeDown3.y = pipeUp3.y - pipeSpacingY - 40;
 
    //reset the close box flag
    doCloseBox = 0;
  }

  //move all four pipes to the left by 1 pixel
  pipeUp0.x--;
  pipeDown0.x--;
  pipeUp1.x--;
  pipeDown1.x--;
  pipeUp2.x--;
  pipeDown2.x--;
  pipeUp3.x--;
  pipeDown3.x--;

  //if the fourth pipe is at a specific height and the speed-up box is not currently active
  if (pipeUp3.y == 38 && !speedUpBoxActive) {
    //activate the speed-up box, setting its position and bitmap number
    speedUpBoxActive = 1;
    speedBox.x = pipeUp3.x + 2;
    speedBox.y = pipeUp3.y - (pipeSpacingY / 2) - (speedBox.height / 2);
    speedBox.bitmapNum = 0;
  }
  //if the fourth pipe is at a specific height and the close box is not currently active
  else if (pipeUp3.y == 20 && !closeBoxActive) {
    //activate the close box, setting its position and bitmap number
    closeBoxActive = 1;
    closeBox.x = pipeUp3.x + 2;
    closeBox.y = pipeUp3.y - (pipeSpacingY / 2) - (closeBox.height / 2);
    closeBox.bitmapNum = 0;
  }
  //Checking if the fourth pipe is at a specific height and the dark box is not currently active
  else if (pipeUp3.y == 45 && !darkBoxActive) {
    //Activating the dark box, setting its position and bitmap number
    darkBoxActive = 1;
    darkBox.x = pipeUp3.x + 2;
    darkBox.y = pipeUp3.y - (pipeSpacingY / 2) - (darkBox.height / 2);
    darkBox.bitmapNum = 0;
  }
  //If the third pipe is at a specific height and the slow box is not currently active
  else if (pipeUp3.y == 35 && !slowBoxActive) {
    //Activating the slow box, setting its position and bitmap number
    slowBoxActive = 1;
    slowBox.x = pipeUp3.x + 2;
    slowBox.y = pipeUp3.y - (pipeSpacingY / 2) - (slowBox.height / 2);
    slowBox.bitmapNum = 0;
  }
  //If the second pipe is at a specific height and the heart box is not currently active
  else if (pipeUp3.y == 34 && !heartBoxActive) {
    //Activating the heart box, setting its position and bitmap number
    heartBoxActive = 1;
    heartBox.x = pipeUp3.x + 2;
    heartBox.y = pipeUp3.y - (pipeSpacingY / 2) - (heartBox.height / 2);
    heartBox.bitmapNum = 0;
  }

  //Moving clouds across the screen
  if (frame & 1)
    cloudOffsetX--;
  if (cloudOffsetX <= -14) {
    cloudOffsetX += cloudSpacingX;
    cloud0.y = cloud1.y;
    cloud1.y = cloud2.y;
    cloud2.y = cloud3.y;
    //Setting a random Y coordinate for cloud3
    cloud3.y = -5 + micros() % 40;
  }
  //Setting the X position of each cloud based on the offset
  cloud0.x = cloudOffsetX + cloudSpacingX * 0;
  cloud1.x = cloudOffsetX + cloudSpacingX * 1;
  cloud2.x = cloudOffsetX + cloudSpacingX * 2;
  cloud3.x = cloudOffsetX + cloudSpacingX * 3;

  //If the speed up box is active
  if (speedUpBoxActive) {
    //Moving the speed box to the left
    speedBox.x--;
    //If the speed up box has been hit, move it upwards
    if (speedUpBoxHit) speedBox.y--;
    //If the speed box has moved off the screen, deactivate it
    if (speedBox.x < -speedBox.width || speedBox.y < -speedBox.height) {
      speedUpBoxHit = 0;
      speedUpBoxActive = 0;
    }
    //If 3 frames have passed, alternate the bitmap number of the speed box
    if (!(frame % 3)) {
      speedBox.bitmapNum ^= 1;
    }
    //If the flappy bird collides with the speed box, activate the speed up effect
    if (testPixelCollision(&flappyBird, &speedBox)) {
      speedUpBoxHit = 1;
      speedUp = 10000;
    }
  }

// Not needed
  //If the slow box is active
  if (slowBoxActive) {
    //Moving the slow box to the left
    slowBox.x--;
    //If the slow box has been hit, move it upwards
    if (slowBoxHit) slowBox.y--;
    if (slowBox.x < -slowBox.width || slowBox.y < -slowBox.height) {
      slowBoxHit = 0;
      slowBoxActive = 0;
    }
    if (!(frame % 3)) {
      slowBox.bitmapNum ^= 1;
    }
    if (testPixelCollision(&flappyBird, &slowBox)) {
      slowBoxHit = 1;
      speedUp = -10000;
    }
  }


  if (closeBoxActive) {
    closeBox.x--;
    if (closeBoxHit) closeBox.y--;
    if (closeBox.x < -closeBox.width || closeBox.y < -closeBox.height) {
      closeBoxHit = 0;
      closeBoxActive = 0;
    }
    if (frame & 1) {
      closeBox.bitmapNum ^= 1;
    }
    if (testPixelCollision(&flappyBird, &closeBox)) {
      closeBoxHit = 1;
      doCloseBox = 10;
    }
  }

// Not needed
  if (darkBoxActive) {
    darkBox.x--;
    if (darkBoxHit) darkBox.y--;
    if (darkBox.x < -darkBox.width || darkBox.y < -darkBox.height) {
      darkBoxHit = 0;
      darkBoxActive = 0;
    }
    if (frame & 1) {
      darkBox.bitmapNum ^= 1;
    }
    if (testPixelCollision(&flappyBird, &darkBox)) {
      darkBoxHit = 1;
      doDarkBox = 10;
    }
  }

// Not needed
  if (heartBoxActive) {
    heartBox.x--;
    if (heartBoxHit) heartBox.y--;
    if (heartBox.x < -heartBox.width || heartBox.y < -heartBox.height) {
      heartBoxHit = 0;
      heartBoxActive = 0;
    }
    if (frame & 1) {
      heartBox.bitmapNum ^= 1;
    }
    if (!heartBoxHit && testPixelCollision(&flappyBird, &heartBox)) {
      heartBoxHit = 1;
      lives++;
    }
  }

// button press
  if (getInput()) {
    wingChangeCount = 2;
    if (flappyBird.y > 0) {
      flappyBird.y -= 5; // determines amount of rise for the bird 
      wing.y = flappyBird.y + 1;
    }
  } else {
    wingChangeCount = 6;

// determines speed of bird falling
    if (flappyBird.y < ground.y - flappyBird.height) {
      if (counter % fall_speed == 0) {
        flappyBird.y += 1;
        wing.y = flappyBird.y + 1;
      }
      counter++;

    }
  }

  if (!(frame % wingChangeCount)) {
    wing.bitmapNum++;
    if (wing.bitmapNum > 2) wing.bitmapNum = 0;
  }

  pipeUp1.bitmap = greenPipeUpBitmap;
  pipeDown1.bitmap = greenPipeDownBitmap;
  pipeUp0.bitmap = greenPipeUpBitmap;
  pipeDown0.bitmap = greenPipeDownBitmap;

  if (testPixelCollision(&flappyBird, &pipeUp0)) {
    pipeUp0.bitmap = redPipeUpBitmap;
    if (!collisionDetected) {
      collisionDetected = 1;
      lives--;
    }
  } else if (testPixelCollision(&flappyBird, &pipeUp1)) {
    pipeUp1.bitmap = redPipeUpBitmap;
    if (!collisionDetected) {
      collisionDetected = 1;
      lives--;
    }
  } else if (testPixelCollision(&flappyBird, &pipeDown0)) {
    pipeDown0.bitmap = redPipeDownBitmap;
    if (!collisionDetected) {
      collisionDetected = 1;
      lives--;
    }
  } else if (testPixelCollision(&flappyBird, &pipeDown1)) {
    pipeDown1.bitmap = redPipeDownBitmap;
    if (!collisionDetected) {
      collisionDetected = 1;
      lives--;
    }
  }

  if (pipeUp0.x + pipeUp0.width == flappyBird.x) {
    if (!collisionDetected) {
      currentScore++;
      
      //SerialUSB.println(currentScore);

      if (!(currentScore % 10)) {
        lives++;
      }
      sprintf(score.stringChars, "%d", currentScore);
      score.x = 50;
      score.y = 20;
      showScore = 1;
    }
    collisionDetected = 0;
  }

  if (hearts.x > (lives * 7) - hearts.width + 2) {
    hearts.x--;
  }
  if (hearts.x < (lives * 7) - hearts.width + 2) {
    hearts.x++;
  }

  if (!lives) {
    if (currentScore > highScore) {
      highScore = currentScore;
    }
    startScreen = 1;
    currentScore = 0;
  }

  if (showScore) {
    if (score.y > -20)
      score.y -= 2;
    else
      showScore = 0;
  }

  if (startScreen) {
    if (flappyBird.x > -25) {
      flappyBird.x--;
      wing.x--;
    }
    if (title.y < 5)
      title.y += 2;
    else if (getInput()) {
      startScreen = 0;
      title.y = -60;
      flappyBird.x = 25;
      wing.x = 25;
      flappyBird.y = (pipeUp0.y + pipeUp1.y) / 2 - (pipeSpacingY / 2) - (flappyBird.height / 3);
      wing.y = flappyBird.y + 1;
      lives = 9999;
    }
  }

  unsigned long timer = micros();
  drawBuffer();
  timer = micros() - timer;
  //while(!SerialUSB);
  //SerialUSB.println(timer);
  //delay(100);

  //for game speed
  int delayTime = 30000 - speedUp - (currentScore / 10) * 1000;
  if (delayTime < 0) delayTime = 0;
  delayMicroseconds(delayTime);
  if (speedUp > 0) speedUp -= 50;
  if (speedUp < 0) speedUp += 100;

  if (!(frame % 4)) {
    if (doDarkBox) {
      doDarkBox--;
      currentBrightness--;
      if (currentBrightness < 0) currentBrightness = 0;
      display.setBrightness(currentBrightness);
    } else if (currentBrightness < defaultBrightness) {
      currentBrightness++;
      display.setBrightness(currentBrightness);
    }
  }
}



#define zmax(a, b) ((a) > (b) ? (a) : (b))
#define zmin(a, b) ((a) < (b) ? (a) : (b))

bool testBitmapCollision(ts_sprite *s1, ts_sprite *s2) {
  if (s1->x < s2->x + s2->width && s1->x + s1->width > s2->x)
    if (s2->y < s1->y + s1->height && s2->y + s2->height > s1->y)
      return true;
  return false;
}

bool testPixelCollision(ts_sprite *s1, ts_sprite *s2) {
  if (!testBitmapCollision(s1, s2)) return false;
  int startX = zmax(s1->x, s2->x);
  int endX = zmin(s1->x + s1->width, s2->x + s2->width);
  int startY = zmax(s1->y, s2->y);
  int endY = zmin(s1->y + s1->height, s2->y + s2->height);
  for (int y = startY; y < endY; y++) {
    for (int x = startX; x < endX; x++) {
      if (s1->bitmap[(y - s1->y) * s1->width + (x - s1->x)] != ALPHA && s2->bitmap[(y - s2->y) * s2->width + (x - s2->x)] != ALPHA)
        return true;
    }
  }
  return false;
}

int writeCount = 0;

void drawBuffer() {
  uint8_t lineBuffer[96 * 64 * 2];
  display.startData();
  for (int y = 0; y < 64; y++) {
    for (int b = 0; b < 96; b++) {
      lineBuffer[b * 2] = TS_16b_Blue >> 8;
      lineBuffer[b * 2 + 1] = TS_16b_Blue;
    }
    for (int spriteIndex = 0; spriteIndex < amtSprites; spriteIndex++) {
      ts_sprite *cs = spriteList[spriteIndex];
      if (y >= cs->y && y < cs->y + cs->height) {
        int endX = cs->x + cs->width;
        if (cs->x < 96 && endX > 0) {
          int bitmapNumOffset = cs->bitmapNum * cs->width * cs->height;
          int xBitmapOffset = 0;
          int xStart = 0;
          if (cs->x < 0) xBitmapOffset -= cs->x;
          if (cs->x > 0) xStart = cs->x;
          int yBitmapOffset = (y - cs->y) * cs->width;
          for (int x = xStart; x < endX; x++) {
            unsigned int color = cs->bitmap[bitmapNumOffset + xBitmapOffset + yBitmapOffset++];
            if (color != ALPHA) {
              lineBuffer[(x)*2] = color >> 8;
              lineBuffer[(x)*2 + 1] = color;
            }
          }
        }
      }
    }
    putString(y, score.x, score.y, score.stringChars, lineBuffer, liberationSans_16ptFontInfo);
    if (startScreen) {
      putString(y, 1, 38, score.stringChars, lineBuffer, liberationSans_10ptFontInfo);
      char hs[10];
      sprintf(hs, "%d", highScore);
      putString(y, 74, 38, hs, lineBuffer, liberationSans_10ptFontInfo);
    }
    display.writeBuffer(lineBuffer, 96 * 2);
  }


  display.endTransfer();
}


void putString(int y, int fontX, int fontY, char *string, uint8_t *buff, const FONT_INFO &fontInfo) {
  const FONT_CHAR_INFO *fontDescriptor = fontInfo.charDesc;
  int fontHeight = fontInfo.height;
  if (y >= fontY && y < fontY + fontHeight) {
    const unsigned char *fontBitmap = fontInfo.bitmap;
    int fontFirstCh = fontInfo.startCh;
    int fontLastCh = fontInfo.endCh;
    //if(!_fontFirstCh)return 1;
    //if(ch<_fontFirstCh || ch>_fontLastCh)return 1;
    //if(_cursorX>xMax || _cursorY>yMax)return 1;
    int stringChar = 0;
    int ch = string[stringChar++];
    while (ch) {
      uint8_t chWidth = pgm_read_byte(&fontDescriptor[ch - fontFirstCh].width);
      int bytesPerRow = chWidth / 8;
      if (chWidth > bytesPerRow * 8)
        bytesPerRow++;
      unsigned int offset = pgm_read_word(&fontDescriptor[ch - fontFirstCh].offset) + (bytesPerRow * fontHeight) - 1;

      for (uint8_t byte = 0; byte < bytesPerRow; byte++) {
        uint8_t data = pgm_read_byte(fontBitmap + offset - (y - fontY) - ((bytesPerRow - byte - 1) * fontHeight));
        uint8_t bits = byte * 8;
        for (int i = 0; i < 8 && (bits + i) < chWidth; i++) {
          if (data & (0x80 >> i)) {
            buff[(fontX)*2] = TS_16b_Yellow >> 8;
            buff[(fontX)*2 + 1] = TS_16b_Yellow;
            // setPixelInBuff(y,16+fontX,0);
            //lineBuffer[16+fontX]=0;
          } else {
            //SPDR=_fontBGcolor;
          }
          fontX++;
        }
      }
      fontX += 2;
      ch = string[stringChar++];
    }
  }
}



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void main_program() {
/*  
  const byte Loading_Page        = 0;
  const byte Ready               = 1;
  const byte Difficulty_Selected = 2;
  const byte Countdown_5s        = 3;
  const byte Game_Start          = 4; 
  const byte Finish_Results      = 5;
  const byte Pause               = 6;
  const byte Continue_15s        = 7;
*/
/*
  // volatile flags
  loading_flag        = 0; // Loading Screen
  wifi_flag           = 0; // WiFi Connection
  sensorChecks_flag   = 0; // Sensor Checks
  TS_flag             = 0; // User Data obtained
  diffSelect_flag     = 0; // Difficulty Selection
  countdown_flag      = 0; // Countdown flag
  gameOver_flag       = 0; // To see if game is over
  flappyBitDepth_flag = 0; // 16 bit depth for Flappy Bird
  normalBitDepth_flag = 0; // 8 bit depth normally
  screenReset_flag    = 0; // To reset the screen
  servo_flag          = 0; // check servo 
  ldr_flag            = 0; // check ldr
  pause_flag          = 0; // check if should pause
  resume_flag         = 0; // check if can resume
  easyMode_flag       = 0; // check if in easy mode
  hardMode_flag       = 0; // check if in hard mode




  -- timers --
  x5s_flag             = 0; // timer of 5s -> x5s_elapsed
  x10s_flag            = 0; // timer of 10s -> x10s_elapsed
  x15s_flag            = 0; // timer of 15s -> x15s_elapsed
  x60s_flag            = 0; // timer of 60s -> x60s_elapsed
  x500ms_flag          = 0; // timer of 500ms -> x500ms_elapsed
  exitTimer_flag       = 1; // timer for exiting pause sequence

*/


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

  switch (current_state) {

// Main Cycle
//------------------------------
    case Loading_Page:
      
      while (loading_flag != 1){
        if (wifi_flag != 1){
          connect_to_wifi();
        }

        if (sensorChecks_flag != 1){
          sensor_checks();
          loading_page();
        }

        if (wifi_flag == 1 && sensorChecks_flag == 1){
          // To code TS_flag related functions
          loading_flag = 1;
        }
      }
      
      current_state = Ready;
      break;
    
    case Ready: //TS_flag, diffSelect_flag
      //lines;
      //Ready screen
      //check if can Start in machine field

      TS_flag = 0;
      room_ldrVal = analogRead(ldrPin);
      userStatus = 2; // temporary value
      ReadyScreen();
      checkUserReady();
      getUserData();

      current_state = Difficulty_Selected;
      
      break;

    case Difficulty_Selected:
      //lines;
      //Show User Welcome Message for 5s
      //Show difficulty selected screen for 5s      
      welcomeUser();
      delay(5000);
      showDiff();

      if (difficultySelected == 1){
        easy_mode();
      }
      if (difficultySelected == 2){
        hard_mode();
      }

      delay(5000);

      current_state = Countdown_5s;
      break;

    case Countdown_5s:
      //lines
      //Show countdown screen for 5s
      countdown_5s();

      current_state = Game_Start;
      break;

    case Game_Start:
      //lines;
      
      previousMillis = millis(); // start timing total game time
      poll_millis = millis();

      SerialUSB.println("\nGame begin");

      time_paused = 0;

      counter = 0;
      //actual game loop
      while (gameOver_flag != 1){ // game over when elapsed time is 60s
        FlappyBird();
        if (x60s_flag != 1){
          x60s_elapsed();
        }
        gameOver_flag = x60s_flag; // currently 20s game time
        pause_poll();       
      }

      counter = 0;

      resetScreen(); //resets the Bit Depth set for flappy bird
      normalMode(); // To show display.print properly


      mainLoopFlags_reset(); //resets relevant flags for main loop
      


      // screenReset_flag = 0; //reset relevant flags back to 0
      // gameOver_flag = 0;
      // x60s_flag = 0;

      
      //resultsScreen();
      //screenFadeOut(); // not working
      // poll every 0.5s if need to pause
      // if pause, 
      //current_state = Pause;
      //FlappyBird();
      //use millis to end game
      // can use fade out & fade in
      current_state = Finish_Results;
      break;

    case Finish_Results:
      //lines
      //send 2 to TS field to indicate Stop, and send results and load data as well
      //display results screen for 15s
      //when 15s elapsed, go back to Ready screen to restart
      int send_score = 0;
      
      resultsScreen();


      float field1Value = patientRPM;
      //float field2Value = load_value;

      SerialUSB.print("RPM: ");
      SerialUSB.println(patientRPM);
      //SerialUSB.println("\n");

      
      //ThingSpeak.setField(patientInputFieldScore, field1Value);
      //ThingSpeak.setField(patientInputFieldLOAD, field2Value);


      int temp_status = 0;

      temp_status = userStatus;

      delay(15000);

      //send stop signal to machine ThingSpeak //2 for Stop
      int x = ThingSpeak.writeField(machineChannelNumber, 1, 2, machineWriteAPIKey);
      while (x != 200){
        SerialUSB.println("Problem updating channel. HTTP error code " + String(x));
        delay(2000);
        x = ThingSpeak.writeField(machineChannelNumber, 1, 2, machineWriteAPIKey);
      }

      delay(15000);

      //ThingSpeak.setField(patientInputFieldScore, field1Value);


      //change to temp_status == 0 to send only if game finishes successfully
      if (temp_status > -1){ //send user's results if finished exercise
        // //for testing
        patientInputFieldScore = 1;
        // final_score = 20;
        send_score = ThingSpeak.writeField(patientChannelNumber, patientInputFieldScore, field1Value, patientWriteAPIKey);

        //send_score = ThingSpeak.writeField(patientChannelNumber, patientInputFieldScore, final_score, patientWriteAPIKey);
        
        while (send_score != 200){ //attempts to send the score every 2 seconds
          SerialUSB.println("Problem updating channel. HTTP error code " + String(send_score));
          delay(2000);

          send_score = ThingSpeak.writeField(patientChannelNumber, patientInputFieldScore, field1Value, patientWriteAPIKey);
          
          //send_score = ThingSpeak.writeField(patientChannelNumber, patientInputFieldScore, final_score, patientWriteAPIKey);
        }

      }
      


      display.clearScreen();
      display.setCursor(48 - (display.getPrintWidth("Getting ready...") / 2), 32 - (display.getFontHeight() / 2));
      display.println("\nGetting ready..."); // This is centralised
      SerialUSB.println("\nGetting ready");

      //initialise RPM variables back to 0
      patientRPM = 0;
      numRevs = 0;

      //checks if can restart Main Cycle
      delay(15000);    
      current_state = Ready;

      break;

// End Main Cycle
//-------------------------------

  }

}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------





void start_message(){
  normalMode();
  display.setCursor(48 - (display.getPrintWidth("Group 3") / 2), 32 - (display.getFontHeight() / 2));
  display.print("Group 3"); // This is printed in the centre of the screen
  delay(2000);  
  display.clearScreen();  
}
//-------------------------------------------------------------------------------------------------      


void loading_page(){

  normalMode();

// (3/3) Start of code excerpt from http://www.geekmomprojects.com/tinyscreen-case-with-buttons/  
  // Button pressed
  byte buttonVal = display.getButtons();
  if (buttonVal && buttonPressed == false) {
    
    // Left buttons change polygon shape
    if (buttonVal == 8) {
      pol_index = (pol_index + 1) % NPOLS;
    } else if (buttonVal == 4) {
      pol_index = (pol_index == 0 ? NPOLS - 1 : pol_index -1);
    } else if ((buttonVal == 2) || (buttonVal == 1)) {
      col_index = (col_index + 1) % N_DARK_COLORS;
    }
    buttonPressed = true;
  }
  
  //Button released
  if (buttonPressed && !display.getButtons()) {
    buttonPressed = false;
  }
 
  // Copy nodes over fresh each time
  Polygon* pol = pol_list[pol_index];
  pol->copy_nodes(new_nodes);
  
  // Rotate and draw the polygon
  pol->rotate3D(angle_y, angle_p, angle_r, new_nodes);
  //display.clearWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  // Need this delay statement to give the screen time to clear
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1, dark_colors[col_index][0], dark_colors[col_index][1], dark_colors[col_index][2]);
  delayMicroseconds(100);
  pol->draw(display, new_nodes);
  
  // Increment angles
  angle_y = ((int) (angle_y + inc_y)) % 360;
  angle_p = ((int) (angle_p + inc_p)) % 360;
  angle_r = ((int) (angle_r + inc_r)) % 360;

  delay(10 + (10 * wifi_flag)); // delay of 20 is desired. Delay of 10 in wifi function
// (3/3) End of code excerpt from http://www.geekmomprojects.com/tinyscreen-case-with-buttons/
}
//-------------------------------------------------------------------------------------------------      

//wifi_flag
void connect_to_wifi(){
  // Connect or reconnect to WiFi
  int wifi_timer = WIFI_DELAY;
  
  if(WiFi.status() != WL_CONNECTED){
    SerialUSB.print("Connecting to WiFi: ");
    SerialUSB.println(SECRET_SSID);
    
    while(WiFi.status() != WL_CONNECTED){
      if (wifi_timer == WIFI_DELAY){
        WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. 
                              //Change this line if using open or WEP network
        SerialUSB.print(".");
        //SerialUSB.println("_______________________________________________________");
        wifi_timer = 0;        
      }     
      loading_page();     
      delay(10);
      wifi_timer += 20;
      //SerialUSB.println(wifi_timer);     //For troubleshooting
    }
    SerialUSB.println("\nConnected");
  }
  wifi_flag = 1;  
}
//-------------------------------------------------------------------------------------------------      


//sensorChecks_flag
void sensor_checks(){ // to update
// Servo & LDR flags

// // Servo: (pin 5) // to change or update
//   int servo_mode = difficultySelected;  
//   if (servo_mode > 0){
//     servo_flag = 1;
//   }
  servo_flag = 1; //for testing

// LDR: (pin A1)
  //ldrVal = analogRead(ldrPin);
  
  ldrVal = 1; //for testing 

  while (ldrVal == 0){
    ldrVal = analogRead(ldrPin);
    delay(20); // keep updating if LDR not working
  }

  if (ldrVal > 0){
    ldr_flag = 1;
  }
 
// Sensors OK?:
  if (servo_flag == 1 && ldr_flag == 1){
    sensorChecks_flag = 1;
  }

}




//------------------------------------------------------------------------------------------------
//
//Servo codes
//------------------------------------------------------------------------------------------------

#define START_POS 50
#define END_POS 90
#define SPEED 50 //lower is faster
int pos = 0;
/*
  void hard_mode(){
    for (pos = START_POS; pos <= END_POS; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(SPEED);                       // waits 15 ms for the servo to reach the position
    }
  }

  void easy_mode(){
    for (pos = END_POS; pos >= START_POS; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(SPEED);                       // waits 15 ms for the servo to reach the position
    }
  }
*/

void hard_mode(){ //easyMode_flag, hardMode_flag
  if (hardMode_flag != 1){
    for (pos = 180; pos >= 90; pos -= 4) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myServo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(SPEED);                       // waits 15 ms for the servo to reach the position
    }
    SerialUSB.println("."); //for testing
  }
  SerialUSB.println("Servo ready: Hard mode");
  easyMode_flag = 0;  
  hardMode_flag = 1;
}

void easy_mode(){ //easyMode_flag, hardMode_flag
  if (easyMode_flag != 1){
    for (pos = 0; pos <= 90; pos += 4) { // goes from 180 degrees to 0 degrees
      myServo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(SPEED);                       // waits 15 ms for the servo to reach the position
    }
    SerialUSB.println("."); //for testing
  }
  SerialUSB.println("Servo ready: Easy mode");
  hardMode_flag = 0;  
  easyMode_flag = 1;
}


//End Servo codes
//------------------------------------------------------------------------------------------------


//Needed to play flappy bird, and then to go back to normal screen stuff
//as flappy bird uses a different bit depth for colors
void resetScreen(){ //screenReset_flag

  TinyScreen display = TinyScreen(TinyScreenPlus);
  
  display.begin();
  display.setBrightness(BRIGHTNESS);
  display.setFont(thinPixel7_10ptFontInfo);
  display.fontColor(TS_8b_White,TS_8b_Black); // (text_color,background_color)
  display.setFlip(false); // comment out if screen is flipped

  pinMode(19, INPUT_PULLUP);  // set the button pins as an input with internal pull-up resistor enabled
  attachInterrupt(digitalPinToInterrupt(19), button_isr, FALLING);  // attach interrupt to the falling edge of any button pin

}


// Get data from ThingSpeak
//---------------------------------------------------------------------------
bool stringComplete = false;  // whether the string is complete
//---------------------------------------------------------------------------
int index1, index2, index3; // For the processing of ThingSpeak data. These 
                            // store the index positions of the relevant data 
                            // from the input string
//---------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//int patientInputFieldScore = 0;
//int patientInputFieldLOAD = 0;
//int levelSelected = 0;
//int difficultySelected = 0;

// Format: <name>.<write_field>.<level>.<difficulty>
void getUserData(){ //diffSelect_flag
  
  rawUserData = ThingSpeak.readStringField(machineChannelNumber, userDataA);
  //rawUserData += "Ryan.3.3.2"; // for testing


  index1 = rawUserData.indexOf('.');
  index2 = rawUserData.indexOf('.', index1 + 1);
  index3 = rawUserData.indexOf('.', index2 + 1);
  
  patientName = rawUserData.substring(0, index1); // Name of patient

  String patientInputFieldstr = rawUserData.substring(index1+1, index2); // Input field for patient's data
  patientInputFieldScore = patientInputFieldstr.toInt(); // Field for Score data
  patientInputFieldLOAD = patientInputFieldScore + 1; // Field for Load data

  String levelSelectedstr = rawUserData.substring(index2+1, index3);
  levelSelected = levelSelectedstr.toInt(); // For level selection (1, 2, or 3)

  String difficultySelectedstr = rawUserData.substring(index3+1, index3+2);
  difficultySelected = difficultySelectedstr.toInt(); // For difficulty selection (1 or 2)
  // send data to servo

  String Hard_Easy = "";

  if (difficultySelected == 2){
    Hard_Easy = "Hard";
  }
  if (difficultySelected == 1){
    Hard_Easy = "Easy";
  }


  SerialUSB.print("\nPatient: ");
  SerialUSB.println(patientName);

  SerialUSB.print("Score field: ");
  SerialUSB.println(patientInputFieldScore);

  SerialUSB.print("Load field: ");
  SerialUSB.println(patientInputFieldLOAD);

  SerialUSB.print("Level: ");
  SerialUSB.println(levelSelected);

  SerialUSB.print("Difficulty: ");
  SerialUSB.println(Hard_Easy);
  
  diffSelect_flag = 1;
}
//---------------------------------------------------------------------------


void ReadyScreen(){

  display.clearScreen();
  display.setCursor(48 - (display.getPrintWidth("Ready to begin") / 2), 32 - (display.getFontHeight() / 2));
  
  display.print("\nReady to begin"); // This is printed in the centre of the screen
  SerialUSB.println("Ready to begin");
}


void checkUserReady(){ //TS_flag
  while (TS_flag != 1){
    userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);
    SerialUSB.print(".");

    if (userStatus == 0){ // 0 for Start/Resume
      TS_flag = 1;
      SerialUSB.println(".");      
    }
    delay(1000);    
  }
}


void welcomeUser(){
  display.clearScreen();

  display.setCursor(48 - (display.getPrintWidth("Welcome") / 2), 30 - (display.getFontHeight()));
  display.print("\nWelcome"); // This is centralised

  int nameWidth = getWordWidth(patientName);

  display.setCursor(48 - (nameWidth / 2), 34);
  display.println(patientName); // This is centralised
}


void showDiff(){
  display.clearScreen();

  //all numbers have same width

  display.setCursor(48 - (display.getPrintWidth("Level: 1") / 2), 26 - (display.getFontHeight()));
  display.print("\nLevel: "); // This is centralised
  display.println(levelSelected);


  String Hard_Easy = "";

  if (difficultySelected == 2){
    Hard_Easy = "Hard";

    display.setCursor(48 - (display.getPrintWidth("Difficulty: Hard") / 2), 34);
    display.print("\nDifficulty: "); // This is centralised
    display.println(Hard_Easy);    
  }

  if (difficultySelected == 1){
    Hard_Easy = "Easy";
    
    display.setCursor(48 - (display.getPrintWidth("Difficulty: Easy") / 2), 34);
    display.print("\nDifficulty: "); // This is centralised
    display.println(Hard_Easy);
  }
}


void countdown_5s(){
  display.clearScreen();

  display.setCursor(48 - (display.getPrintWidth("Get ready!") / 2), 26 - (display.getFontHeight()));
  display.print("\nGet ready!"); // This is centralised


  display.setCursor(48 - (display.getPrintWidth("<   >") / 2), 34);
  display.println("<   >");

  display.setCursor(48 - (display.getPrintWidth("5") / 2), 34);
  
  int count_5s = 5; 
  while (count_5s != 0){
    display.setCursor(48 - (display.getPrintWidth("5") / 2), 34);
    display.println(count_5s);
    delay(1000);
    count_5s -= 1;
  }  
}


void resultsScreen(){
  display.clearScreen();

  display.setCursor(48 - (display.getPrintWidth("Congratulations") / 2), 20 - (display.getFontHeight()));
  display.print("\nCongratulations"); // This is centralised


  display.setCursor(48 - (display.getPrintWidth("Score:") / 2), 34);
  display.println("Score:");

  final_score = currentScore * levelSelected * difficultySelected;
  
  int resultsWidth = getWordWidth(String(final_score));
  display.setCursor(48 - (resultsWidth / 2), 44);
  display.println(final_score);

  SerialUSB.println("\nGame finished");

  SerialUSB.println("");
  SerialUSB.print(patientName);
  SerialUSB.print("'s score: ");
  SerialUSB.print(final_score);
  SerialUSB.println("\n");
  
  currentScore = 0;
}

/*
  void finish_sequence(){ // to complete
    //send results, load data to user's field
    //send 2 to indicate stop to machine field
  }
*/


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


void mainLoopFlags_reset(){ //resets flags needed for main loop

  diffSelect_flag = 0;
  TS_flag = 0;

  screenReset_flag = 0;
  gameOver_flag = 0;
  x60s_flag = 0;

}

// with debouncing
// may need to press pause button a few times
// this is to prevent accidental presses from intefering with game
void pause_poll(){ //x500ms_flag, pause_flag, gameOver_flag
  //polls every 0.5s if should pause
  //if pause, halt game and print pause on display
  //if stop, gameOver_flag = 1
  x500ms_elapsed();

  if (x500ms_flag == 1){
    if((millis() - lastPress) > debounceTime && pause_flag){
      lastPress = millis();   //update lastPress                                                     
      if(digitalRead(19) == 0){   //if button is pressed and was released last change
        SerialUSB.println("Paused"); 
        start_time_paused = millis();
        pause_sequence(); 
      }
      pause_flag = 0; // end innterrupt
    }
    x500ms_flag = 0; //reset timer flag
  }  

}

void pause_sequence(){ //resume_flag, x500ms_flag, gameOver_flag
  //after 15s, checks if should resume every 0.5s
  //if resume, clear screen and continue the game

  resetScreen(); //resets the Bit Depth set for flappy bird
  normalMode(); // To show display.print properly

  display.clearScreen();
  display.setCursor(48 - (display.getPrintWidth("Paused") / 2), 32 - (display.getFontHeight() / 2));
  display.println("\nPaused"); // This is centralised

  //for testing
  int paused = ThingSpeak.writeField(machineChannelNumber, 1, 1, machineWriteAPIKey);
  while (paused != 200){
    SerialUSB.println("Problem updating channel. HTTP error code " + String(paused));
    delay(2000);
    paused = ThingSpeak.writeField(machineChannelNumber, 1, 1, machineWriteAPIKey);
  }

  delay(15000);
  SerialUSB.println("Can resume?");
  display.setCursor(48 - (display.getPrintWidth("Can Resume?") / 2), 32 - (display.getFontHeight() / 2));
  display.println("\nCan Resume?"); // This is centralised

  exitTimer_flag = 0;
  exit_pause = millis();

  while (resume_flag != 1){
    x500ms_elapsed();
    exitTimer_elapsed();
    
    if (x500ms_flag == 1){
      userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);
      if (exitTimer_flag == 1){        
        int stopped = ThingSpeak.writeField(machineChannelNumber, 1, 2, machineWriteAPIKey);
        while (stopped != 200){
          SerialUSB.print("\nProblem updating channel. HTTP error code " + String(stopped));
          delay(2000);
          stopped = ThingSpeak.writeField(machineChannelNumber, 1, 2, machineWriteAPIKey);
        }
        SerialUSB.print("\nNo inputs received...");
        userStatus = 2;
        exitTimer_flag = 0;
      }
      //SerialUSB.println(userStatus); //for testing

      if (userStatus == 0){ // Can resume
                            // 0 for Start/Resume, 1 for Pause, 2 for Stop
        resume_flag = 1;
        SerialUSB.println("\nResuming");
        
        countdown_5s();
         

        resetScreen(); //resets the Bit Depth set for flappy bird
        flappyMode(); // To show FlappyBird properly
        time_paused += millis() - start_time_paused;
        SerialUSB.println(time_paused); // for testing

        x60s_flag = 0;
      }

      else if (userStatus == 2){
        display.clearScreen();
        display.setCursor(48 - (display.getPrintWidth("Game stopped") / 2), 32 - (display.getFontHeight() / 2));
        display.println("\nGame stopped"); // This is centralised
        SerialUSB.println("\nGame stopped");

        delay(5000);
        resume_flag = 1;
        gameOver_flag = 1;
      }      
      else { 
        SerialUSB.print(".");
      }
      x500ms_flag = 0; //reset timer flag
    }
  }
  resume_flag = 0; //reset resume_flag
}




//timer functions


void x5s_elapsed() { //x5s_flag  //remember to reset flag to 0 when done
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 5000) { // check if 5 seconds have elapsed
    previousMillis = currentMillis; // reset the previousMillis variable
    x5s_flag = 1; // set the flag to 1
  }
}

void x10s_elapsed() { //x10s_flag  //remember to reset flag to 0 when done
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 10000) { // check if 10 seconds have elapsed
    previousMillis = currentMillis; // reset the previousMillis variable
    x10s_flag = 1; // set the flag to 1
  }
}

void x15s_elapsed() { //x15s_flag  //remember to reset flag to 0 when done
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 15000) { // check if 15 seconds have elapsed
    previousMillis = currentMillis; // reset the previousMillis variable
    x15s_flag = 1; // set the flag to 1
  }
}

//currently set as 20s
//used only for the game
void x60s_elapsed() { //x60s_flag  //remember to reset flag to 0 when done
  unsigned long currentMillis = millis();
  if (currentMillis - (previousMillis + time_paused) >= GAME_DURATION) { // check if game time has elapsed
    previousMillis = currentMillis; // reset the previousMillis variable
    x60s_flag = 1; // set the flag to 1
  }
}

//x500ms_flag
void x500ms_elapsed() { //x500ms_flag //remember to reset flag to 0 when done
  unsigned long currentMillis = millis();
  
  if (currentMillis - poll_millis >= 500) { // check if 5 seconds have elapsed
    poll_millis = currentMillis; // reset the previousMillis variable
    x500ms_flag = 1;
  }
}


void exitTimer_elapsed() { //exitTimer_flag  //remember to reset flag to 0 when done
  unsigned long currentMillis = millis();

  if (currentMillis - exit_pause >= 30000) { // check if 30 seconds have elapsed
    exit_pause = currentMillis; // reset the previousMillis variable
    exitTimer_flag = 1; // set the flag to 1
  }
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------



//verified working above this line

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//test functions


















//---------------------------------------------------------------------------
// causes the game to be too laggy
/*

void pause_poll(){ //x500ms_flag, pause_flag, gameOver_flag
  //polls every 0.5s if should pause
  //if pause, halt game and print pause on display
  //if stop, gameOver_flag = 1
  x500ms_elapsed();

  if (x500ms_flag == 1){
    userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);
    if (userStatus == 1){ // 0 for Start/Resume, 1 for Pause, 2 for Stop
      pause_flag = 1;
      SerialUSB.println("Paused"); 
      pause_sequence();           
    }
    if (userStatus == 2){ //stop game
      gameOver_flag = 1;
    }
  }

  x500ms_flag = 0; //reset timer flag

}

void pause_sequence(){ //resume_flag, x500ms_flag
  //after 15s, checks if should resume every 0.5s
  //if resume, clear screen and continue the game

  resetScreen(); //resets the Bit Depth set for flappy bird
  normalMode(); // To show display.print properly

  display.clearScreen();
  display.setCursor(48 - (display.getPrintWidth("Paused") / 2), 20 - (display.getFontHeight()));
  display.println("\nPaused"); // This is centralised

  delay(15000);
  SerialUSB.println("Can resume?");


  while (resume_flag != 1){
    x500ms_elapsed();

    if (x500ms_flag == 1){
      userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);
      if (userStatus == 0){ // 0 for Start/Resume, 1 for Pause, 2 for Stop
        resume_flag = 1;
        SerialUSB.println("\nResuming"); 
        resetScreen(); //resets the Bit Depth set for flappy bird
        flappyMode(); // To show FlappyBird properly
      }
      else { 
        SerialUSB.print(".");
      }
      x500ms_flag = 0; //reset timer flag
    }
  }
}
*/
//---------------------------------------------------------------------------

//Sample loop flow
/*
  connect_to_wifi();
  //getUserData();
  //FlappyBird();
  //delay(10000);
  //screenFadeOut();
  //screenFadeIn();
  
  ReadyScreen();
  checkUserReady();
  getUserData();
  welcomeUser();
  
  delay(3000);
  showDiff();
  delay(3000);
  countdown_5s();

  previousMillis = millis(); // start timing total game time
  poll_millis = millis();

  while (gameOver_flag != 1){ // game over when elapsed time is 60s
    FlappyBird();

    if (x60s_flag != 1){
      x60s_elapsed();
    }
    gameOver_flag = x60s_flag;
    
    pause_poll();             
  }

  resetScreen(); //resets the Bit Depth set for flappy bird
  normalMode(); // To show display.print properly

  mainLoopFlags_reset(); //resets relevant flags for main loop

  resultsScreen();

  delay(15000);


*/



void updateRPM() {
  // read the LDR value and calculate the time elapsed since the last reading
  int ldrValue = analogRead(ldrPin);  
  if (ldrValue - room_ldrVal > 150){  
    numRevs += 1;    
  }

  patientRPM = numRevs * RPMmultiplier;// should return a whole number
}


/*
void updateRPM() {
  // read the LDR value and calculate the time elapsed since the last reading
  int ldrValue = analogRead(ldrPin);
  unsigned long timeElapsed;
  
  if (ldrValue - room_ldrVal > 150){  
    unsigned long currentTime = millis();
    timeElapsed = currentTime - lastReadings[numReadings % NUM_READINGS];
  
    // store the current time in the lastReadings array
    lastReadings[numReadings % NUM_READINGS] = currentTime;
    numReadings++;

    // calculate the RPM based on the time elapsed since the last few readings
    if (numReadings >= NUM_READINGS) {
      float timePerRevolution = (float)RPM_PERIOD_MS / NUM_READINGS;
      float revolutionsPerTime = timePerRevolution / timeElapsed;
      patientRPM = (int)(revolutionsPerTime * 60);
    }


  }

}
*/




//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


// Misc. Functions


int wordCount = 1;

int getWordWidth(String theWord) { // display.getPrintWidth() does not accept variables,
                                   // so we get the width of each character of the word
                                   // and sum them. Basically replaces the 
                                   // display.getPrintWidth() function
    int wordWidth = 0;
    for (int a = 0; a < theWord.length(); a++){
        if (theWord[a] == 'i' || theWord[a] == 'j' || theWord[a] == 'l'){
            wordWidth += display.getPrintWidth("i"); // i, j, k have smaller width
        }
        else if (theWord[a] == 'm' || theWord[a] == 'w'){ 
            wordWidth += display.getPrintWidth("m"); // m, w have bigger width
        }
        else if (theWord[a] == 1 || theWord[a] == 2 || theWord[a] == 3 ||
        theWord[a] == 4 || theWord[a] == 5 || theWord[a] == 6 || theWord[a] == 7 ||
        theWord[a] == 8 || theWord[a] == 9 || theWord[a] == 0){
            
            wordWidth += display.getPrintWidth("1"); // all numbers have same width
        }
        else if (isupper(theWord[a])){
            wordWidth += display.getPrintWidth("A"); // all uppercase letters have
                                                     // the same width
        }
        else {
            wordWidth += display.getPrintWidth("a"); // all other letters have the
                                                     // same width
        }
    }
    return wordWidth;
}

//------------------------------------------------------------------------------------------------
//1000


void wrapText(String text, int x, int y) {

//######################################################################  
  // Split the text into words
  int count = 0;
  char delimiter = ' '; // Split the sentence into individual words

  // Get total number of words
  for (int i = 0; i < text.length(); i++) {
    if (text[i] == delimiter) {
      wordCount++;
    }
  }
//######################################################################
  // String array declared with wordCount as number of elements
  String output[wordCount];
  
  String currentString = "";


  // This portion will store every word as a seperate element in an array
  for (int i = 0; i < text.length(); i++) {
  // This looks for spaces in the whole input text
  // Since spaces seperates the words, we can get the individual words
    if (text[i] != delimiter) {
      currentString += text[i];
    }
    else {
      if (count < wordCount) {
  // Replace the array's current element to become the current word
        output[count] = currentString; 
        count++;
        currentString = "";
      }
    }
  }
  if (currentString.length() > 0 && count < wordCount) {
    output[count] = currentString;
  }  


  // Set the cursor position
  display.setCursor(x, y);

  // Initialize variables to keep track of the current line
  String line = "";
  int lineWidth = 0;

  line += output[0];
  lineWidth += getWordWidth(output[0]);
  
  // Iterate over the words
  for (int i = 1; i < wordCount; i++) {
    // Add the next word to the line
    String nextWord = output[i];
    
    lineWidth += getWordWidth(nextWord);

    // Check if the line is too long
    if (lineWidth > (86 - (2 * x))) {
      // Print the current line
      display.println(line);

      // Start a new line
      line = "";
      lineWidth = 0;

      // Move the cursor down
      y += display.getFontHeight();
      display.setCursor(x, y);

      line += nextWord;
    } 
    
    else {
      // Add a space after the word
      line += " ";
      line += nextWord;
      
      lineWidth += display.getPrintWidth(" ");
    }
  }
    // Print the last line
    display.println(line);
}


void buttonLoop() {

// Displays the values that will be input if the respective buttons are pressed

  display.setCursor(5, 11);
  display.println("1");

  display.setCursor(5, 46);
  display.println("2");
  
  display.setCursor(85, 11);
  display.println("3");

  display.setCursor(85, 46);
  display.println("4");
  
  ifButtonPressed(); // function for when buttons are pressed
  
}

int number = 0;
void ifButtonPressed(){

// if a button is pressed, the screen is cleared and the cursor is set such that the 
// line printed onto the TinyScreen is centralised. The input value of the button is 
// sent to your ThingSpeak channel, specifically onto the second field.
// A line is displayed to let the user know that the input value is successfully sent.

  
  if (display.getButtons(TSButtonUpperLeft)) {

    display.clearScreen();

    display.setCursor(48 - (display.getPrintWidth("Hard Mode") / 2), 
    32 - (display.getFontHeight() / 2));
    
    number = 1;

    int lastInput = ThingSpeak.readFloatField(machineChannelNumber, 2);  
    if(lastInput == 1){
      SerialUSB.println("Already in Hard Mode");
      display.println("Hard Mode");
      delay(2000);
      display.clearScreen();
    }
    else{
      int x = ThingSpeak.writeField(machineChannelNumber, 2, number, machineWriteAPIKey);
      if(x == 200){
        SerialUSB.print("Hard Mode");
        display.println("Hard Mode");   
        hard_mode();
      }
      else{
        SerialUSB.println("Problem updating channel. HTTP error code " + String(x));
      }
      delay(15000);
      SerialUSB.println(".");
      display.clearScreen();
    }
  } 
  

  if (display.getButtons(TSButtonLowerLeft)) {

    display.clearScreen();
    
    display.setCursor(48 - (display.getPrintWidth("Easy Mode") / 2), 
    32 - (display.getFontHeight() / 2));
    
    number = 2;    

    int lastInput = ThingSpeak.readFloatField(machineChannelNumber, 2);  
    if(lastInput == 2){
      SerialUSB.println("Already in Easy Mode");
      display.println("Easy Mode");
      delay(2000);
      display.clearScreen();
    }
    else{    
      int x = ThingSpeak.writeField(machineChannelNumber, 2, number, machineWriteAPIKey);
      if(x == 200){
        SerialUSB.print("Easy Mode");
        display.println("Easy Mode");
        easy_mode();
      }
      else{
        SerialUSB.println("Problem updating channel. HTTP error code " + String(x));
      }
      delay(15000);
      SerialUSB.println(".");
      display.clearScreen();
    }

  } 
  
  
  if (display.getButtons(TSButtonUpperRight)) {

    display.clearScreen();
    
    display.setCursor(48 - (display.getPrintWidth("Button 3 pressed") / 2), 
    32 - (display.getFontHeight() / 2));
    
    number = 3;
    
    int x = ThingSpeak.writeField(machineChannelNumber, 2, number, machineWriteAPIKey);
    if(x == 200){
      SerialUSB.println("Button 3 pressed");
      display.println("Button 3 pressed");
    }
    else{
      SerialUSB.println("Problem updating channel. HTTP error code " + String(x));
    }
    delay(2000);
    display.clearScreen();
  } 
  
  
  if (display.getButtons(TSButtonLowerRight)) {

    display.clearScreen();
    
    display.setCursor(48 - (display.getPrintWidth("Button 4 pressed") / 2), 
    32 - (display.getFontHeight() / 2));
    
    number = 4;

    int x = ThingSpeak.writeField(machineChannelNumber, 2, number, machineWriteAPIKey);
    if(x == 200){
      SerialUSB.println("Button 4 pressed");
      display.println("Button 4 pressed");
    }
    else{
      SerialUSB.println("Problem updating channel. HTTP error code " + String(x));
    }
     
    delay(2000);  
    display.clearScreen();
  } 


// Get the input from the serial monitor
  char inChar = (char)SerialUSB.read(); // stores the input to inChar

// if the input from the serial monitor is '.', the last input value is displayed
// on the TinyScreen
  if (inChar == '.'){

    display.clearScreen();
    
    display.setCursor(48 - (display.getPrintWidth("Getting last input") / 2), 
    32 - (display.getFontHeight() / 2));
    
    display.println("Getting last input");
    delay(1000);
    
    // Read in field 4 of the public channel recording the temperature
    int lastInput = ThingSpeak.readFloatField(machineChannelNumber, 2);  

    // Check the status of the read operation to see if it was successful
    int statusCode = ThingSpeak.getLastReadStatus();
    
    if(statusCode == 200){
      SerialUSB.println("Last input: " + String(lastInput));
      display.clearScreen();

      display.setCursor(48 - (display.getPrintWidth("Last input: 1") / 2), 
      32 - (display.getFontHeight() / 2));
  
      display.println("Last input: " + String(lastInput));
      delay(2000);
      display.clearScreen();
    }
    else{
      SerialUSB.println("Problem reading channel. HTTP error code " + String(statusCode)); 
      display.clearScreen();
      wrapText("Problem reading channel. HTTP error code " + String(statusCode),x_pos,y_pos); 
      // Text is wrapped
    }   
  }  
}


void screenFadeOut(){
  int toFadeOut = BRIGHTNESS;
  while (toFadeOut != 0){
    toFadeOut -= 0.1; // smoother fade
    display.setBrightness(toFadeOut);
    delay(10); // total fade time = 0.5s // 2 for 0.2s
  }
  display.clearScreen();
}




void screenFadeIn(){
  // call next function to display before fading in

  int toFadeIn = 0;
  while (toFadeIn != BRIGHTNESS){ // change to 'if' for looping in another function
    toFadeIn += 0.1; // smoother fade
    display.setBrightness(toFadeIn);
    delay(10); // total fade time = 0.5s // 2 for 0.2s
  }
}






//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------



//Sample ISR code from ChatGPT

/*
volatile int ISR_flag = 0;  // declare a volatile flag variable to be used in the ISR

void setup() {
  pinMode(19, INPUT_PULLUP);  // set the button pins as an input with internal pull-up resistor enabled
  pinMode(25, INPUT_PULLUP);
  pinMode(30, INPUT_PULLUP);
  pinMode(31, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(19), button_isr, FALLING);  // attach interrupt to the falling edge of any button pin
  attachInterrupt(digitalPinToInterrupt(25), button_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(30), button_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(31), button_isr, FALLING);
}


void loop() {
  if (ISR_flag == 1) {
    // do something in response to the button press
    ISR_flag = 0;  // reset the flag
  }
}


void button_isr() {
  ISR_flag = 1;  // set the flag to 1 when any button is pressed
}

*/





//
