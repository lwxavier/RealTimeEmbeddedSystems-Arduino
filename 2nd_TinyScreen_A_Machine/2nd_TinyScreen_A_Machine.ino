/*
// HX711 Arduino Library by Bogdan Necula

Load cell code:
partially from Rui Santos
-> Complete project details at https://RandomNerdTutorials.com/arduino-load-cell-hx711/

*/


#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi101.h>
#include "secrets.h"

#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros


// WiFi parameters
//-------------------------------------------------------------------------------------------------
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

#define WIFI_DELAY 2000 // attempt connection every 2 seconds
//-------------------------------------------------------------------------------------------------


// Instantiate a TinyScreen object.
TinyScreen display = TinyScreen(TinyScreenPlus);

#define BRIGHTNESS 10 // set brightness at level 10


#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 4;

HX711 scale;

//Load cell variables
int num_readings = 0;
const int BUFFER_SIZE = 50; // Change this value to set the maximum number of readings to store
float readings[BUFFER_SIZE];
float average_load = 0.0;
const int CALIBRATION_FACTOR = 200; // Change to calibration value



volatile int wifi_flag      = 0;
volatile int x1s_flag    = 0;
volatile int pause_flag     = 0;
volatile int gameOver_flag  = 0;
volatile int resume_flag    = 0;


// For sending load cell input to ThingSpeak
volatile int diffSelect_flag     = 0; // Difficulty Selection
int patientInputFieldLOAD = 0;

// not needed
int patientInputFieldScore = 0;
//int patientInputFieldLOAD = 0;
int levelSelected = 0;
int difficultySelected = 0;


//main function variables
int userStatus = 5; // temporary value

volatile int TS_flag = 0;

String patientName = "";
String rawUserData = "";

int index1 = 0;
int index2 = 0;
int index3 = 0;


unsigned long poll_millis = 0;

volatile int pauseMessage_flag = 0;


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
 
//---------------------------------------------------------------------------
//
// TinyScreen setup parameters
//-------------------------------------------------------------------------------------------------      
  display.begin();
  display.setBrightness(BRIGHTNESS);
  display.setFont(thinPixel7_10ptFontInfo);
  display.fontColor(TS_8b_White,TS_8b_Black); // (text_color,background_color)
  display.setFlip(true); // comment out if screen is flipped
  
  display.setBitDepth(TSBitDepth8); // Normal Bit Depth is 8 bit
  //display.setBitDepth(TSBitDepth16); // Bit Depth for Flappy Bird is 16 bit
  
  //display.setFlip(false);
//---------------------------------------------------------------------------  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  rawUserData.reserve(200); // put in setup!! impt

// start message  
  start_message();

// load cell setup
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); //connect load cell to TinyScreen
  scale.set_scale();    
  SerialUSB.println("Tare... remove any weights from the scale.");
  delay(1000);

  scale.tare();

  SerialUSB.println("Tare done");

//connect to wifi
  connect_to_wifi();  

}



void loop() {

  userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);

  ReadyScreen();
  checkUserReady();
  getUserData();

  gettingReady();
  delay(15000); // game should start in 15s after receiving user data

  //scale.tare();
  initialiseLoadData();
  getLoadScreen();

  gameOver_flag = 0; // game starts

  userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);
  
  poll_millis = millis();
  
  //SerialUSB.println("..."); // for testing

  while (gameOver_flag != 1){
//get load data

    //SerialUSB.println("flag is working"); // for testing
    x1s_elapsed();
    if (x1s_flag == 1){
      //SerialUSB.println("flag is working"); // for testing
      userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);
      if (userStatus == 0){
        //long reading = scale.get_units(10);
        getLoadCellReadings();
        // //for testing
        // float reading = scale.get_units();
        // SerialUSB.println(reading);
        SerialUSB.print(".");
        //SerialUSB.println(reading);
        //getLoadData();
        pauseMessage_flag = 0;
      }
      if (userStatus == 1){
        if (pauseMessage_flag != 1){
          SerialUSB.println("\nPaused");
          pauseMessage_flag = 1;
        }
      }
      if (userStatus == 2){
        SerialUSB.println("\nGame stopped");
        gameOver_flag = 1;
      }
      x1s_flag = 0; //reset timer flag
    }
  }

  TS_flag = 0;
  pauseMessage_flag = 0;

  updateAverageLoad();

  SerialUSB.println("\nAverage load: ");
  SerialUSB.print(average_load / 1000);
  SerialUSB.print(" kilograms");
  SerialUSB.println("\n");

  //send load data
  sendLoadData();

  finishScreen();

  initialiseLoadData();

  delay(10000); // for testing

}



//-------------------------------------------------------------------
//main functions



//wifi_flag
void connect_to_wifi(){
  // Connect or reconnect to WiFi
  int wifi_timer = WIFI_DELAY;
  
  display.clearScreen();
  display.setCursor(48 - (display.getPrintWidth("Connecting to WiFi...") / 2), 32 - (display.getFontHeight() / 2));
  display.print("\nConnecting to WiFi..."); // This is printed in the centre of the screen

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
      delay(10);
      wifi_timer += 20;
      //SerialUSB.println(wifi_timer);     //For troubleshooting
    }
    SerialUSB.println("\nConnected");
  }
  wifi_flag = 1;  
}

void start_message(){
  //normalMode(); //not needed for 2nd TS
  display.setCursor(48 - (display.getPrintWidth("Group 3") / 2), 32 - (display.getFontHeight() / 2));
  display.print("Group 3"); // This is printed in the centre of the screen
  delay(2000);  
  display.clearScreen();  
}



//-------------------------------------------------------------------
//load cell functions


// for second TinyScreen
// start getting load cell inputs once game has started
// send average readings after game ends 
void getLoadCellReadings() { //num_readings, BUFFER_SIZE
  float rawReading = scale.get_units();
  float reading = rawReading / CALIBRATION_FACTOR;
  readings[num_readings] = reading;
  num_readings++;
  if (num_readings == BUFFER_SIZE) {
    num_readings = 0; // Start overwriting the oldest readings once we reach the end of the buffer
  }
}

void updateAverageLoad() {
  int zero_counter = 0;
  float sum = 0.0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (readings[i] != 0){
      sum += readings[i];
    }
    else{
      zero_counter += 1;
    }
  }
  average_load = sum / (BUFFER_SIZE - zero_counter);
  if(average_load < 0){
    average_load = -1 * average_load;
  }
}

void initialiseLoadData() {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    readings[i] = 0;
  }
}

void sendLoadData(){
  SerialUSB.print("\nUploading load data...");
  int send_load = ThingSpeak.writeField(patientChannelNumber, patientInputFieldLOAD, average_load, patientWriteAPIKey);
  while (send_load != 200){
    delay(2000);
    SerialUSB.print(".");
    send_load = ThingSpeak.writeField(patientChannelNumber, patientInputFieldLOAD, average_load, patientWriteAPIKey);
  }
  SerialUSB.println("\n");  
}

//---------------------------------------------------------------------------

//int patientInputFieldScore = 0;
//int patientInputFieldLOAD = 0;
//int levelSelected = 0;
//int difficultySelected = 0;

// Format: <name>.<write_field>.<level>.<difficulty>
void getUserData(){ //diffSelect_flag
  
  rawUserData = ThingSpeak.readStringField(machineChannelNumber, userDataA);
  //rawUserData += "Ryan.3.3.2";


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

  SerialUSB.print("\nPatient: ");
  SerialUSB.println(patientName);

  SerialUSB.print("Score field: ");
  SerialUSB.println(patientInputFieldScore);

  SerialUSB.print("Load field: ");
  SerialUSB.println(patientInputFieldLOAD);

  SerialUSB.print("Level: ");
  SerialUSB.println(levelSelected);

  SerialUSB.print("Difficulty: ");
  SerialUSB.println(difficultySelected);
  
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



void gettingReady(){
  display.clearScreen();
  display.setCursor(48 - (display.getPrintWidth("Getting ready...") / 2), 32 - (display.getFontHeight() / 2));
  display.println("\nGetting ready..."); // This is centralised
  SerialUSB.println("\nGetting ready...");
}


void getLoadScreen(){
  display.clearScreen();

  display.setCursor(48 - (display.getPrintWidth("Getting load data") / 2), 32 - (display.getFontHeight() / 2));
  display.print("\nGetting load data"); // This is centralised

  SerialUSB.println("\nGetting load data");
}


void finishScreen(){
  display.clearScreen();

  display.setCursor(48 - (display.getPrintWidth("Initialising...") / 2), 32 - (display.getFontHeight() / 2));
  display.print("\nInitialising..."); // This is centralised

  SerialUSB.println("\nInitialising...");
}



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




//---------------------------------------------------------------------------
/*
pause_poll();
pause_sequence();
*/

// causes the game to be too laggy

//x1s_flag
//currently at 1000ms
void x1s_elapsed() { //x1s_flag //remember to reset flag to 0 when done
  unsigned long currentMillis = millis();
  
  if (currentMillis - poll_millis >= 1000) { // check if 1 seconds have elapsed
    poll_millis = currentMillis; // reset the previousMillis variable
    x1s_flag = 1;
  }
}

void pause_poll(){ //x1s_flag, pause_flag, gameOver_flag
  //polls every 0.5s if should pause
  //if pause, halt game and print pause on display
  //if stop, gameOver_flag = 1
  x1s_elapsed();

  if (x1s_flag == 1){
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
  x1s_flag = 0; //reset timer flag
}

void pause_sequence(){ //resume_flag, x1s_flag, gameOver_flag
  //after 15s, checks if should resume every 0.5s
  //if resume, clear screen and continue the game

  // resetScreen(); //resets the Bit Depth set for flappy bird
  // normalMode(); // To show display.print properly

  display.clearScreen();
  display.setCursor(48 - (display.getPrintWidth("Paused") / 2), 32 - (display.getFontHeight() / 2));
  display.println("\nPaused"); // This is centralised

  delay(15000);

  while (resume_flag != 1){
    x1s_elapsed();
    
    if (x1s_flag == 1){
      userStatus = ThingSpeak.readFloatField(machineChannelNumber, start_stop_pause);

      if (userStatus == 0){ // Can resume
                            // 0 for Start/Resume, 1 for Pause, 2 for Stop
        resume_flag = 1;
        SerialUSB.println("\nResuming");         
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
      x1s_flag = 0; //reset timer flag
    }
  }
  resume_flag = 0; //reset resume_flag
}

//---------------------------------------------------------------------------


/*

  //From ChatGPT

  void initialiseLoadData() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
      loadCellReadings[i] = 0;
    }
  }



  //in setup:
  //initialise the array for load cell readings:
    for (int i = 0; i < BUFFER_SIZE; i++) {
      loadCellReadings[i] = 0;
    }



  #include "HX711.h"

  // HX711 circuit wiring
  const int LOADCELL_DOUT_PIN = 3;
  const int LOADCELL_SCK_PIN = 4;

  HX711 scale;

  const int BUFFER_SIZE = 100; // Change this value to set the maximum number of readings to store
  float readings[BUFFER_SIZE];
  float average_load = 0.0;

  int num_readings = 0;


  void setup() {
    Serial.begin(9600);
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(1.0); // Adjust this value based on your calibration
    scale.tare();
  }

  void getLoadCellReadings() {
    float reading = scale.get_units();
    readings[num_readings] = reading;
    num_readings++;
    if (num_readings == BUFFER_SIZE) {
      num_readings = 0; // Start overwriting the oldest readings once we reach the end of the buffer
    }
  }

  void updateAverageLoad() {
    int zero_counter = 0;
    float sum = 0.0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      if (readings[i] != 0){
        sum += readings[i];
      }
      else{
        zero_counter += 1;
      }
    }
    average_load = sum / (BUFFER_SIZE - zero_counter);
  }

  void loop() {
    getLoadCellReadings();
    updateAverageLoad();
    Serial.print("Current Load: ");
    Serial.print(readings[num_readings - 1]); // Print the latest reading
    Serial.print(", Average Load: ");
    Serial.println(average_load);
    delay(1000);
  }





*/













//
