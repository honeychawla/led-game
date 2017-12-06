#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define ENEMY_COLOR LED_GREEN
#define PLAYER_COLOR LED_YELLOW

Adafruit_BicolorMatrix matrix1 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix2 = Adafruit_BicolorMatrix();

// Screens
int matrix1Screen[8][8] = { {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0} };
int matrix2Screen[8][8] = { {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0} };

// Player variables
int playerX = 0;
int playerY = 0;

// Joystick
const int joystickYAxis = A1;
const int joystickXAxis = A0;
int xReading;
int yReading;
String dirToMove;

void setup() {
  Serial.begin(9600);

  pinMode(joystickXAxis, INPUT);
  pinMode(joystickYAxis, INPUT);
  int level = random(0,5);
  randomSeed(analogRead(level));

  generatePlayer();

  matrix1.begin(0x70);
  matrix2.begin(0x71);
  matrix1.setRotation(3);
  matrix2.setRotation(3);
  matrix1.clear();
  matrix2.clear();
  matrix1.writeDisplay();
  matrix2.writeDisplay();

  drawMatrix1();
  drawMatrix2();
}

void loop() {
  int playing = 0;
  
  while (playing < 30) {
    movePlayer();
    
    // Generate some enemies
    generateEnemies();
    
    drawMatrix1();
    drawMatrix2();
    delay(300);
  
    // Move enemies
    moveEnemies();
    delay(random(1400,1800));
    
    playing++;
  }

  winner();
}

void movePlayer() {
  xReading = analogRead(joystickXAxis);
  yReading = analogRead(joystickYAxis);
  int newPlayerX = playerX;
  int newPlayerY = playerY;

  // Determine which way the player is trying to move
  if (xReading < 200) {
    dirToMove = "LEFT";
    if (playerX == 0) {
      // Already at the leftmost column
      newPlayerX = playerX;
    } else {
      newPlayerX--;
    }
  } else if (xReading > 900) {
    dirToMove = "RIGHT";
    if (playerX == 6) {
      // The player can't move to the 7th column because we need room for their weapon
      newPlayerX = playerX;
    } else {
      newPlayerX++;
    }
  } else if (yReading < 200) {
    dirToMove = "UP";
    if (playerY == 0) {
      newPlayerY = playerY;
    } else {
      newPlayerY--;
    }
  } else if (yReading > 900) {
    dirToMove = "DOWN";
    if (playerY == 7) {
      newPlayerY = playerY;
    } else {
      newPlayerY++;
    }
  } else {
    dirToMove = "STAY";
  }

  // Move the player and gun to their new location

  if (matrix1Screen[newPlayerX][newPlayerY] == 0) {
    matrix1Screen[playerX][playerY] = 0;
    matrix1Screen[newPlayerX][newPlayerY] = 1;
  } else if (matrix1Screen[newPlayerX][newPlayerY] == 3) {
    // Moving to where an enemy is will kill me and will end the game
    matrix1Screen[playerX][playerY] = 0;
    gameOver();
  }

  playerX = newPlayerX;
  playerY = newPlayerY;
  
  drawMatrix1();
  drawMatrix2();
}

void moveEnemies() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {      
      if (i == 0 && matrix1Screen[i][j] == 3) {
        matrix1Screen[i][j] = 0;
      } else if (matrix1Screen[i][j] == 3) {
        if (matrix1Screen[i-1][j] == 0) {
          // If the spot is open, turn off the current spot and turn on the forward spot
          matrix1Screen[i][j] = 0;
          matrix1Screen[i-1][j] = 3;
        } else if (matrix1Screen[i-1][j] == 1) {
          // Kill the player
          matrix1Screen[i][j] = 0;
          matrix1Screen[i-1][j] = 3;
          gameOver();
        }
      } else if (matrix2Screen[i][j] == 3) {
        if (i == 0) {
          // Moving to matrix 1
          if (matrix1Screen[7][j] == 0) {
            matrix2Screen[i][j] = 0;
            matrix1Screen[7][j] = 3;
          }
        } else if (matrix2Screen[i-1][j] == 0) {
          if (matrix2Screen[i-1][j] == 0) {
            // If the spot is open, turn off the current spot and turn on the forward spot
            matrix2Screen[i][j] = 0;
            matrix2Screen[i-1][j] = 3;
          } else if (matrix2Screen[i-1][j] == 1) {
            // Kill the player
            matrix2Screen[i][j] = 0;
            matrix2Screen[i-1][j] = 3;
            gameOver();
          }
        }
      }
    }
  }
}

void generatePlayer() {
  // The x location of the player should be between 0 and 6 so as to leave a spot on the right side for the player's weapon
  playerX = random(7);
  // The y location can be any spot
  playerY = random(8);

  // Update the matrices screens
  matrix1Screen[playerX][playerY] = 1;
}

void generateEnemies() {
  // Decide how many enemies to generate - between 1 and 3
  int numToGen = random(1,4);
  
  // For each enemy we have to spawn
  for (int i = 0; i < numToGen; i++) {
    boolean conflict = true;

    while (conflict) {
      int enemyMatrix = 2;
      int enemyX = 7;
      int enemyY = random(8);

      // If the spot is open, assign the enemy to that starting location
      if (matrix2Screen[enemyX][enemyY] == 0) {
        matrix2Screen[enemyX][enemyY] = 3;
        conflict = false;
      }
    }
  }
}

void winner() {
  matrix1.clear();
  matrix2.clear();
  matrix1.writeDisplay();
  matrix2.writeDisplay();

  while (true) {
    matrix1.clear();
    matrix1.drawLine(0,0, 7,7, LED_YELLOW);
    matrix1.writeDisplay();  // write the changes we just made to the display
    delay(500);

    matrix2.clear();
    matrix2.drawLine(0,0, 7,7, LED_YELLOW);
    matrix2.writeDisplay();  // write the changes we just made to the display
    delay(500);
  
    matrix1.clear();
    matrix1.drawRect(0,0, 8,8, LED_RED);
    matrix1.fillRect(2,2, 4,4, LED_GREEN);
    matrix1.writeDisplay();  // write the changes we just made to the display
    delay(500);

    matrix2.clear();
    matrix2.drawRect(0,0, 8,8, LED_RED);
    matrix2.fillRect(2,2, 4,4, LED_GREEN);
    matrix2.writeDisplay();  // write the changes we just made to the display
    delay(500);
  
    matrix1.clear();
    matrix1.drawCircle(3,3, 3, LED_YELLOW);
    matrix1.writeDisplay();  // write the changes we just made to the display
    delay(500);
    
    matrix2.clear();
    matrix2.drawCircle(3,3, 3, LED_YELLOW);
    matrix2.writeDisplay();  // write the changes we just made to the display
    delay(500);
  }
}

void gameOver() {
  matrix1.clear();
  matrix2.clear();
  matrix1.writeDisplay();
  matrix2.writeDisplay();

  while (true) {
    matrix1.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
    matrix1.setTextSize(1);
    matrix1.setTextColor(LED_GREEN);
    matrix2.setTextColor(LED_GREEN);
    for (int8_t x=7; x>=-25; x--) {
      matrix1.clear();
      matrix1.setCursor(x,0);
      matrix1.print("Game");
      matrix1.writeDisplay();
      delay(85);
    }
    for (int8_t x=7; x>=-25; x--) {
      matrix2.clear();
      matrix2.setCursor(x,0);
      matrix2.print("Over");
      matrix2.writeDisplay();
      delay(85);
    }
  }
}

void drawMatrix1() {
  // Take matrix1screen and loop through it and draw every pixel
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (matrix1Screen[i][j] == 1) {
        matrix1.drawPixel(i, j, PLAYER_COLOR);
      } else if (matrix1Screen[i][j] == 3) {
        matrix1.drawPixel(i, j, ENEMY_COLOR);
      } else if (matrix1Screen[i][j] == 0) {
        matrix1.drawPixel(i, j, 0);
      }
    }
  }
  
  matrix1.writeDisplay();
}

void drawMatrix2() {
  // Take matrix2screen and loop through it and draw every pixel
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (matrix2Screen[i][j] == 3) {
        matrix2.drawPixel(i, j, ENEMY_COLOR);
      } else if (matrix2Screen[i][j] == 0) {
        matrix2.drawPixel(i, j, 0);
      }
    }
  }

  matrix2.writeDisplay();
}
