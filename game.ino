#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define ENEMY_COLOR LED_GREEN
#define GUN_COLOR LED_RED
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

// Gun variables
int gunX = 0;
int gunY = 0;

// Joystick
const int joystickXAxis = A1;
const int joystickYAxis = A0;
int joystickSelect = 2;
int xReading;
int yReading;
int shoot;
String dirToMove;

void setup() {
  Serial.begin(9600);

  pinMode(joystickXAxis, INPUT);
  pinMode(joystickYAxis, INPUT);
  pinMode(joystickSelect, INPUT_PULLUP);
  
  randomSeed(analogRead(0));
  
  attachInterrupt(0, shoot_ISR, CHANGE);

  generatePlayer();
  generateGun();

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
  boolean playing = true;

  while (playing) {
    movePlayer();
    
    // Generate some enemies
    generateEnemies();
    
    drawMatrix1();
    drawMatrix2();
    delay(300);
  
    // Move enemies
    moveEnemies();
    delay(random(1400,2000));
  }
}

void shoot_ISR() {
  if (digitalRead(joystickSelect) == HIGH) {
    shootGun();
  }
}

void shootGun() {
  // The bullet is going to travel in a straight horizontal line, so the y position will stay the same but the x position increase as well as change matrices
  int currX = gunX;
  int currY = gunY;
  int currMatrix = 1;
  int gone = 0;

  if (currX != 7) {
    // Just increase the x position
    currX++;
  } else if (currX == 7 && currMatrix == 1) {
    // Change matrices
    currMatrix = 2;
    // Update x
    currX = 0;
  } else if (currX == 7 && currMatrix == 2) {
    // We need to make the bullet disappear
    gone = 1;
  }
}

void movePlayer() {
  xReading = analogRead(joystickXAxis);
  yReading = analogRead(joystickYAxis);
  int newPlayerX = playerX;
  int newPlayerY = playerY;
  int newGunX = gunX;
  int newGunY = gunY;

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
    dirToMove = "DOWN";
    if (playerY == 7) {
      newPlayerY = playerY;
    } else {
      newPlayerY++;
    }
  } else if (yReading > 900) {
    dirToMove = "UP";
    if (playerY == 0) {
      newPlayerY = playerY;
    } else {
      newPlayerY--;
    }
  } else {
    dirToMove = "STAY";
  }

  // Determine the gun's new position
  newGunX = newPlayerX + 1;
  newGunY = newPlayerY;

  // Move the player and gun to their new location

  if (matrix1Screen[newPlayerX][newPlayerY] == 0 || matrix1Screen[newPlayerX][newPlayerY] == 2) {
    matrix1Screen[playerX][playerY] = 0;
    matrix1Screen[gunX][gunY] = 0;
    matrix1Screen[newPlayerX][newPlayerY] = 1;
    matrix1Screen[newGunX][newGunY] = 2;
  } else if (matrix1Screen[newPlayerX][newPlayerY] == 3) {
    // Moving to where an enemy is will kill me and will end the game
    matrix1Screen[playerX][playerY] = 0;
    matrix1Screen[gunX][gunY] = 0;
    gameOver();
  }

  playerX = newPlayerX;
  playerY = newPlayerY;
  gunX = newGunX;
  gunY = newGunY;
  
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

void generateGun() {
  // The gun should be generated on the pixel directly to the right of the player
  gunX = playerX + 1;
  gunY = playerY;

  // Update the matrices screens
  matrix1Screen[gunX][gunY] = 2;
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

void gameOver() {
  matrix1.clear();
  matrix2.clear();
  matrix1.writeDisplay();
  matrix2.writeDisplay();

  while (true) {
    
  }
}

void drawMatrix1() {
  // Take matrix1screen and loop through it and draw every pixel
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (matrix1Screen[i][j] == 1) {
        matrix1.drawPixel(i, j, PLAYER_COLOR);
      } else if (matrix1Screen[i][j] == 2) {
        matrix1.drawPixel(i, j, GUN_COLOR);
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
