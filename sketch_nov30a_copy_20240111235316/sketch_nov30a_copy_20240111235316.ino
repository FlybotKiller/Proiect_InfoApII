#include <SPI.h>               //Serial Peripheral Interface, commonly used for communicating with peripherals (displays, sensors, etc.)
#include <Wire.h>              //used for I2C(Inter-Integrated Circuit) communication, usually between microcontrollers and peripherals
#include <Adafruit_GFX.h>      //part of Adafruit Graphics Library, provides a set of graphic functions for drawing shapes, text and other elements
#include <Adafruit_SSD1306.h>  //provides functions to control SSD1306-based displays (initialize display, draw shapes and/or text, update the screen)
//libraries

const int SW_pin = 4;    //digital pin connected to switch output for player 1
const int SW_pin2 = 2;   //digital pin connected to switch output for player 2
const int LR = 9;        //digital pin connected to red LED
const int LG = 6;        //digital pin connected to green LED
const int LY = 8;        //digital pin connected to yellow LED
const int LB = 10;       //digital pin connected to blue LED
const int player1 = A1;  //analog pin connected to X output for player 1
const int player2 = A0;  //analog pin connected to X output for player 2
//digital pins are used for detecting if there is a voltage present in a pin or not while analog pins are
//connected to an internal Analog to Digital Converter and can measure the actual voltage on the pin

enum GameState {
  RUNNING,
  PAUSED,
  IDLE
};
//declared an enumeration "GameState" with two states: RUNNING and PAUSED

GameState gameState = IDLE;  //initialized a variable "gameState" with the default value "IDLE"


const unsigned long PADDLE_RATE = 40;                          //defined a constant for paddle update rate, determines how often the paddle's position is updated in the game loop (lower value means frequent updates resulting in higher speed)
const unsigned long BALL_RATE = 0;                             //defined a constant for ball update rate, determines how often the ball's position is updated in the game loop (lower value means frequent updates resulting in higher speed)
const uint8_t PADDLE_HEIGHT = 14;                              //defined a constant value for the paddle height
int player1Score = 0;                                          //defined default player 1 score
int player2Score = 0;                                          //defined default player 2 score
int maxScore = 5;                                              //defined max score, whoever reaches this score first, wins
int BEEPER = 12;                                               //defined the digital pin number to which the piezo buzzer is connected
bool resetBall = false;                                        //defined a flag to indicate when the ball needs to be reset, default value is set to false
#define SCREEN_WIDTH 128                                       //OLED display width, in pixels
#define SCREEN_HEIGHT 64                                       //OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);  // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
//this class is part of the Adafruit Graphics Library and is specifically designed for controlling OLED displays
//based on the SSD1306 controller (common controller used in small monochrome OLED displays)

void drawCourt();
void drawScore();
void drawPaused();
void startGame();
void startSound();
void confirmSound();
//function prototypes for different functions, without these prototypes, if a function is called
//before its implementation is seen by the compiler it will result in a compilation error

uint8_t ball_x = 64, ball_y = 32;        //declared initial x and y ball coordinates
uint8_t ball_dir_x = 1, ball_dir_y = 1;  //declared initial x and y ball direction (both are 1 so ball is heading to the right and downward)
unsigned long ball_update;               //declared variable used to store the timestamp (in ms) when the last update was made to the ball's position

unsigned long paddle_update;   //declared variable used to store the timestamp (in ms) when the last update was made to the paddles' positions
const uint8_t PLAYER2_X = 22;  //initialised player 2's starting position on the x axis using a constant variable
uint8_t player2_y = 26;        //initialised player 2's starting position on the y axis using a constant variable

const uint8_t PLAYER_X = 105;  //initialised player 1's starting position on the x axis using a constant variable
uint8_t player1_y = 26;        //initialised player 1's starting position on the y axis using a constant variable

void setup() {
  //setup display and pins
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //initialize the OLED display with the specified settings in the brackets using the begin() method of the Adafruit_SSD1306 class
  //display.display(); //turns on the OLED display
  pinMode(BEEPER, OUTPUT);      //sets the beeper pin as an output pin (used to send electrical signals to produce sound)
  pinMode(SW_pin, INPUT);       //sets the switch pin as an input pin for player 1 (used to read the state of a switch)
  pinMode(SW_pin2, INPUT);      //sets the switch pin as an input pin for player 2 (used to read the state of a switch)
  pinMode(LR, OUTPUT);          //sets the red LED pin as an output pin (used to send electrical signals to turn the LED on)
  pinMode(LG, OUTPUT);          //sets the green LED pin as an output pin (used to send electrical signals to turn the LED on)
  pinMode(LY, OUTPUT);          //sets the yellow LED pin as an output pin (used to send electrical signals to turn the LED on)
  pinMode(LB, OUTPUT);          //sets the blue LED pin as an output pin (used to send electrical signals to turn the LED on)
  digitalWrite(SW_pin, HIGH);   //sets default switch state to HIGH, once the switch is pressed, the pin is connected to ground and the state changes to "LOW" (player 1)
  digitalWrite(SW_pin2, HIGH);  //sets default switch state to HIGH, once the switch is pressed, the pin is connected to ground and the state changes to "LOW" (player 2)
  startSound();                 //calls the startSound() function to play a starting tune
  startGame();                  //calls the startGame() function
}

void loop() {
  bool update = false;            //declared a boolean variable used to determine whether updates to the game elements need to be executed, initialized to false
  unsigned long time = millis();  //variable that records and stores the current time in ms, used to control the timing of updates in the game loop

  static bool up_state = false;
  static bool down_state = false;
  //declared two static boolean variables used to keep track of the state of certain inputs related to paddle movement

  if (resetBall)  //checks if resetBall is true
  {
    if (player1Score == maxScore || player2Score == maxScore)  //checks if one of the players has reached the max score
    {
      gameOver();  //if a player has reached max score, calls the gameOver() function
    } else         //if none of the players reached the max score:
    {
      display.fillScreen(BLACK);  //clears the display by making it black
      drawScore();                //displays the score using the drawScore() function
      drawCourt();                //displays the court using the drawCourt() function
      //sets the ball's starting coordinates in the center of the screen
      ball_x = 64;
      ball_y = 32;
      //the following do ... while() statements set random directions for the ball on the x and y axis, making sure the ball is never idle
      do {
        ball_dir_x = random(-1, 2);
      } while (ball_dir_x == 0);

      do {
        ball_dir_y = random(-1, 2);
      } while (ball_dir_y == 0);

      resetBall = false;  //finally, the resetBall flag is changed back to false
    }
  }

  if (time > ball_update && gameState == RUNNING)  //checks if it's time to update the ball's position and if the game state is set to RUNNING
  {
    //calculates the potential new position of the ball based on its current position and direction
    uint8_t new_x = ball_x + ball_dir_x;
    uint8_t new_y = ball_y + ball_dir_y;

    // Check if the ball hits the horizontal walls
    if (new_x - 1 <= 0 || new_x + 1 >= 127) {
      if (new_x - 1 <= 0)  //if the ball hits the horizontal left wall
      {
        player1Score += 1;          //increment player 1 score by 1
        display.fillScreen(BLACK);  //clear the display by making it black
        soundPoint();               //play a scoring sound
        resetBall = true;           //set the resetBall flag to true
      } else if (new_x + 1 >= 126)  //if the ball hits the horizontal wall on the right
      {
        player2Score += 1;          //increment player 2 score by 1
        display.fillScreen(BLACK);  //clear the display by making it black
        soundPoint();               //play a scoring sound
        resetBall = true;           //set the resetBall flag to true
      }
      //change the direction of the ball on the x axis and update new_x accordingly
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    // Check if the ball hits the vertical walls.
    if (new_y - 1 <= 0 || new_y + 1 >= 63)  //if the ball hits the top wall(0) or the bottom wall(63)
    {
      soundBounce();  //play a bouncing sound
      //change the direction of the ball on the y axis and update new_y accordingly
      ball_dir_y = -ball_dir_y;
      new_y += ball_dir_y + ball_dir_y;
    }

    // Check if the ball hits the player 2 paddle
    if (new_x == PLAYER2_X && new_y >= player2_y && new_y <= player2_y + PADDLE_HEIGHT)
    //if the new position on the x axis of the ball equals the x coordinate of player 2's paddle
    //and the new y coordinate is within the height of the paddle:
    {
      soundBounce();  //play a bouncing sound
      //change the direction of the ball on the x axis and update new_x accordingly
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    // Check if the ball hits the player 1 paddle
    if (new_x == PLAYER_X && new_y >= player1_y && new_y <= player1_y + PADDLE_HEIGHT)
    //if the new position on the x axis of the ball equals the x coordinate of player 1's paddle
    //and the new y coordinate is within the height of the paddle:
    {
      soundBounce();  //play a bouncing sound
      //change the direction of the ball on the x axis and update new_x accordingly
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    display.drawCircle(ball_x, ball_y, 1, BLACK);  //erase previous ball position
    display.drawCircle(new_x, new_y, 1, WHITE);    //draw the new position of the ball in white
    //update ball x and y coordinates with the new position
    ball_x = new_x;
    ball_y = new_y;

    ball_update += BALL_RATE;  //increment ball_update by BALL_RATE to control the timing of the next ball update

    update = true;  //set the update flag to true to indicate that an update has occured
  }

  if (time > paddle_update)  //checks if it's time to update the paddle positions based on the elapsed time since the last paddle update
  {
    paddle_update += PADDLE_RATE;  //increments paddle_update by PADDLE_RATE to control the timing of the next paddle update

    if (gameState == RUNNING)  //checks if the game state is set to RUNNING (ensures that paddle updates are only performed when the game is in progress)
    {
      // Player 2 paddle
      display.drawFastVLine(PLAYER2_X, player2_y, PADDLE_HEIGHT, BLACK);  //erases the current position of the paddle by drawing a black line
      const uint8_t half_paddle = PADDLE_HEIGHT >> 1;                     //calculates half the height of the paddle
      if (analogRead(player2) < 475)                                      //checks if the analog reading from the player 2 input is less than 475
      {
        player2_y -= 1;  //if true, moves the player 2 paddle up
      }
      if (analogRead(player2) > 550)  //checks if the analog reading from player 2 input is greater than 550
      {
        player2_y += 1;  //if true, moves the player 2 paddle down
      }
      //next two if statements ensure the player paddle won't go beyond the top or bottom bounds
      if (player2_y < 1)
        player2_y = 1;
      if (player2_y + PADDLE_HEIGHT > 63)
        player2_y = 63 - PADDLE_HEIGHT;
      display.drawFastVLine(PLAYER2_X, player2_y, PADDLE_HEIGHT, WHITE);  //draws the player 2 paddle at its updated position

      // Player 1 paddle
      display.drawFastVLine(PLAYER_X, player1_y, PADDLE_HEIGHT, BLACK);  //erases the current position of the paddle by drawing a black line
      if (analogRead(player1) < 475)                                     //checks if the analog reading from the player 1 input is less than 475
      {
        player1_y -= 1;  //if true, moves the player 1 paddle up
      }
      if (analogRead(player1) > 550)  //checks if the analog reading from the player 1 input is greater than 550
      {
        player1_y += 1;  //if true, moves the player 1 paddle down
      }
      up_state = down_state = false;  //resets state variables related to paddle movement
      //next two if statements ensure the player paddle won't go beyond the top or bottom bounds
      if (player1_y < 1)
        player1_y = 1;
      if (player1_y + PADDLE_HEIGHT > 63)
        player1_y = 63 - PADDLE_HEIGHT;
      display.drawFastVLine(PLAYER_X, player1_y, PADDLE_HEIGHT, WHITE);  //draws the player 1 paddle at its updated position
    }
  }
  update = true;  //sets the update flag to true, indicating that a paddle update has occured

  if (update)  //checks if an update has occured
  {
    if (gameState == PAUSED)  //checks if the game state is set to PAUSED
    {
      display.clearDisplay();  //clears the display removing any previous content
      drawPaused();            //calls the drawPaused() function
    } else                     //if the game state is set to RUNNING
    {
      drawScore();  //calls the drawScore() function
      drawCourt();  //calls the drawCourt() function
    }
    display.display();  //regardless of the game state, this line updates the display to reflect the changes made in previous steps
  }

  if (digitalRead(SW_pin) == LOW || digitalRead(SW_pin2) == LOW)  //checks if either joystick button is pressed
  {
    if (gameState == RUNNING)  //checks if the game state is set to RUNNING
    {
      gameState = PAUSED;  //sets the game state to PAUSED
    } else                 //if the game state is already set to PAUSED and the joystick button has been pressed
    {
      gameState = RUNNING;     //sets the game state back to running
      display.clearDisplay();  //clears the display of the "PAUSED" text
      drawScore();             //brings back the score with the drawScore() function
      drawCourt();             //brings back the court with the drawCourt() function
      display.display();       //updates the display to reflect the changes made
    }

    if (gameState == IDLE)  //if gameState is set to idle
    {
      gameState == RUNNING;  //sets the game state to running
    }

    // Added a small delay until the Paused screen pops up
    delay(200);
  }
}

void drawCourt() {
  display.drawRect(0, 0, 128, 64, WHITE);  //draws a rectangular white outline around the entire display which serves as the game court
  int lineSpacing = 4;                     // Adjust the spacing between the dashes
  int dashHeight = 8;                      // Adjust the height of each dash

  for (int y = 0; y < SCREEN_HEIGHT; y += dashHeight + lineSpacing) {    //loop that iterates through the height of the display, creating a series of dashed lines
    int x = SCREEN_WIDTH / 2;                                            //calculates the middle of the screen on the x axis
    int dashEndY = y + dashHeight;                                       //calculates the ending y coordinate of the current dash
    display.drawLine(x, y, x, min(dashEndY, SCREEN_HEIGHT - 1), WHITE);  //draws a white vertical dashed line at the middle of the screen starting from the current y coordinate and ending at dashEndY or the bottom of the screen
  }
}

void drawScore() {
  // draw players' scores
  display.setTextSize(2);         //sets the text size for the scores
  display.setTextColor(WHITE);    //sets the text color for the scores
  display.setCursor(45, 0);       //sets the cursor position for player 2's score
  display.println(player2Score);  //displays player 2's score
  display.setCursor(75, 0);       //sets the cursor position for player 1's score
  display.println(player1Score);  //displays player 1's score
}

void drawPaused() {
  display.setTextSize(2);       //sets text size
  display.setTextColor(WHITE);  //sets text color
  display.setCursor(30, 24);    //sets the text cursor coordinates
  display.println("PAUSED");    //displays the "PAUSED" text
}

void gameOver() {
  display.fillScreen(BLACK);        //fills the entire screen with a black color
  if (player1Score > player2Score)  //checks if player1's score is greater than player2's score
  {
    display.setCursor(20, 15);    //sets the text cursor's coordinates
    display.setTextColor(WHITE);  //sets the text color
    display.setTextSize(2);       //sets text size
    display.print("Player 1");    //displays the "Player 1" text
    display.setCursor(40, 35);    //sets the text cursor's coordinates
    display.print("won");         //displays the text "won"
  } else                          //if player2's score is greater than player1's score
  {
    display.setCursor(20, 15);    //sets the text cursor's coordinates
    display.setTextColor(WHITE);  //sets the text color
    display.setTextSize(2);       //sets the text size
    display.print("Player 2");    //displays the "Player 2" text
    display.setCursor(40, 35);    //sets the text cursor's coordinates
    display.print("won");         //displays the "won" text
  }
  delay(100);                       //small delay to allow the message to appear
  display.display();                //updates the display with the winner message
  soundWinning();                   //plays winning song
  player2Score = player1Score = 0;  //resets player scores
  player1_y = player2_y = 26;       //resets paddle position back to their default starting positions

  startGame();
}

void startGame() {
  display.clearDisplay();                    // clear the display
  display.fillScreen(BLACK);                 // set the background color to black
  display.setTextSize(3);                    // set text size
  display.setTextColor(WHITE);               // set text color
  display.setCursor(29, 2);                  // set the text cursor's coordinates
  display.println("PONG");                   // display the "PONG" prompt
  display.setTextSize(1);                    //set text size
  display.setCursor(65, 25);                 //set the text cursor's coordinates
  display.println("by Denis");               //display the "by Denis" prompt
  display.setCursor(1, 45);                  //set the text cursor's coordinates
  display.println("Press switch to start");  //display the "Press switch to start" prompt
  display.display();                         // update the display to show changes made

  //boolean flags to check whether a switch has been pressed or not, default value is false
  bool player1SwitchPressed = false;
  bool player2SwitchPressed = false;
  const int LEDPins[4] = { LR, LG, LY, LB };  //LED pins array (makes it easier to turn off all LEDs quickly)

  // Wait for Player 1 switch press
  while (!player1SwitchPressed) {
    if (digitalRead(SW_pin) == LOW) {  //if player 1 switch has been pressed
      digitalWrite(LB, HIGH);          //turn on the blue LED
      digitalWrite(LY, HIGH);          //turn on the yellow LED
      confirmSound();                  //plays a confirm tune
      player1SwitchPressed = true;     //sets boolean flag to true
      delay(100);                      // Button debouncing delay
    }
  }

  // Wait for Player 2 switch press
  while (!player2SwitchPressed) {       
    if (digitalRead(SW_pin2) == LOW) {  //if player2 switch has been pressed
      digitalWrite(LR, HIGH);  //turn on the red LED
      digitalWrite(LG, HIGH);  //turn on the green LED
      confirmSound(); //play a confirm tune
      player2SwitchPressed = true; //sets boolean flag to true
      delay(100);  // Button debouncing delay
    }
  }

  for (int i = 0; i < 4; i++) {
    digitalWrite(LEDPins[i], LOW);
  }

  // Clear the display and call score and court functions before starting the game
  display.clearDisplay();
  drawCourt();
  drawScore();
  display.display();

  // Reset game state and variables
  gameState = RUNNING;
  player1Score = 0;
  player2Score = 0;
  resetBall = true;

  // Initialize ball and paddle positions
  ball_x = 64;
  ball_y = 32;
  player1_y = 26;
  player2_y = 26;

  // Reset update times
  ball_update = millis();
  paddle_update = ball_update;
}


// Sound of ball hitting wall and paddles
void soundBounce() {
  tone(BEEPER, 500, 50);  //uses the tone() function to generate a tone at a frequency of 500Hz for 50ms
}

// Sound of point scored
void soundPoint() {
  tone(BEEPER, 100, 50);  //Used the tone() function to generate a tone at a frequency of 100Hz for 50ms
}

void confirmSound() {
  tone(BEEPER, 1000, 100);  //Used the tone() function to generate a tone at a frequency of 1000Hz for 100ms
  delay(150);               //Added a small delay
  noTone(BEEPER);           //Silenced the buzzer
  delay(50);                //Added a small delay
  tone(BEEPER, 1500, 100);  //Used the tone() function to generate a tone at a frequency of 1500Hz for 100ms
  delay(150);               //Added a small delay
  noTone(BEEPER);           //Silenced the buzzer
}

void startSound() {
  int melody[] = { 600, 800, 1000 };        // Adjust frequencies
  int noteDurations[] = { 150, 150, 150 };  // Adjust durations in ms

  for (int i = 0; i < 3; i++) {
    tone(BEEPER, melody[i], noteDurations[i]);
    delay(noteDurations[i] + 50);  // Adds a small delay between notes
    noTone(BEEPER);                // Stops the tone immediately after each note
  }
}


void soundWinning() {
  // Play a winning sound for a total of 2 seconds
  int melody[] = { 400, 500, 600, 700, 800 };         // frequencies
  int noteDurations[] = { 300, 250, 200, 150, 100 };  // durations in ms

  const int numLEDs = 4;                            //number of LEDs
  const int LEDPins[numLEDs] = { LR, LG, LY, LB };  //define the LED pins in an array

  unsigned long startTime = millis();

  int currentNote = 0;

  while (millis() - startTime < 2000) {
    tone(BEEPER, melody[currentNote], noteDurations[currentNote]);
    digitalWrite(LEDPins[currentNote], HIGH);  //Turn on the LED corresponding to the current note
    delay(noteDurations[currentNote]);
    digitalWrite(LEDPins[currentNote], LOW);  //Turn off the LED after the note duration
    delay(50);                                // Adds a small delay between notes
    noTone(BEEPER);                           // Stops the tone immediately after each note
    currentNote = (currentNote + 1) % 5;      // Moves to the next note in the sequence
  }

  for (int i = 0; i < numLEDs; i++) {  //Turn off all the LEDs after the sequence is complete
    digitalWrite(LEDPins[i], LOW);
  }
}