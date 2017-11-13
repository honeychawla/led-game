#include <Wire.h>
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
int playerMatrix = 0;

// Gun variables
int gunX = 0;
int gunY = 0;
int gunMatrix = 0;

//Enemy variables
int numEnemies = 8;
// {enemyMatrix, enemyX, enemyY, enemyAlive}
int enemies[8][4];

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));

  matrix1.begin(0x70);
  matrix2.begin(0x71);
  matrix1.setRotation(3);
  matrix2.setRotation(3);
  matrix1.clear();
  matrix2.clear();
  matrix1.writeDisplay();
  matrix2.writeDisplay();

  generatePlayer();
  generateGun();
  generateEnemies();

  drawMatrix1();
  drawMatrix2();
}

void loop() {
  
}

void generatePlayer() {
  // Decide whether the player should be generated on matrix 1 or matrix 2
  playerMatrix = random(2) + 1;

  // The x location of the player should be between 0 and 6 so as to leave a spot on the right side for the player's weapon
  playerX = random(7);
  // The y location can be any spot
  playerY = random(8);

  // Update the matrices screens
  if (playerMatrix == 1) {
    matrix1Screen[playerX][playerY] = 1;
  } else if (playerMatrix == 2) {
    matrix2Screen[playerX][playerY] = 1;
  }
}

void generateGun() {
  // The gun should be generated on the pixel directly to the right of the player
  gunX = playerX + 1;
  gunY = playerY;

  // The matrix that the gun is on is the same as the player upon generation
  gunMatrix = playerMatrix;

  // Update the matrices screens
  if (gunMatrix == 1) {
    matrix1Screen[gunX][gunY] = 2;
  } else if (gunMatrix == 2) {
    matrix2Screen[gunX][gunY] = 2;
  }
}

void generateEnemies() {
  // For each enemy we have to spawn
  for (int i = 0; i < numEnemies; i++) {
    int conflict = 2;
    int enemyMatrix;
    int enemyX;
    int enemyY;
    int enemyAlive = 1;

    // Find new locations until there isn't a conflict
    while (conflict != 0) {
      // Determine which matrix they're on
      enemyMatrix = random(2) + 1;
      // Determine their x and y location
      enemyX = random(8);
      enemyY = random(8);

      // Make sure another enemy isn't at that location
      if (enemyMatrix == 1 && matrix1Screen[enemyX][enemyY] == 0) {
        conflict--;
      } else if (enemyMatrix == 2 && matrix2Screen[enemyX][enemyY] == 0) {
        conflict--;
      }

      // If the enemy gets spawned on the same matrix as the player
      if (enemyMatrix == playerMatrix) {
        // Make sure the player has a one pixel border around them where enemies can't spawn
        if ((enemyX == playerX && enemyY != playerY + 1 && enemyY != playerY - 1) &&
            (enemyX == gunX && enemyY != gunY + 1 && enemyY != gunY - 1) &&
            (enemyX == playerX - 1 && enemyY != playerY - 1 && enemyY != playerY && enemyY != playerY + 1) &&
            (enemyX == gunX + 1 && enemyY != gunY - 1 && enemyY != gunY && enemyY != gunY + 1)) {
              conflict--;
        }
      } else {
        // If they're not spawned on the same matrix, check the edges of the matrix
        if (playerMatrix == 1) {
          if (enemyX == 0 && enemyY != gunY && enemyY != gunY + 1 && enemyY != gunY - 1) {
            conflict = 0;
          }
        } else if (playerMatrix == 2) {
          if (enemyX == 7 && enemyY != playerY && enemyY != playerY + 1 && enemyY != playerY - 1) {
            conflict = 0;
          }
        }
      }      
    }

    // Assign the legitimate values
    enemies[i][0] = enemyMatrix;
    enemies[i][1] = enemyX;
    enemies[i][2] = enemyY;
    enemies[i][3] = enemyAlive;

    if (enemyMatrix == 1) {
      matrix1Screen[enemyX][enemyY] = 3;
    } else if (enemyMatrix == 2) {
      matrix2Screen[enemyX][enemyY] = 3;
    }
  }
}

void drawMatrix1() {
  // Take matrix1screen and loop through it and draw every pixel
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (matrix1Screen[i][j] == 1) {
        // Draw the player
        matrix1.drawPixel(i, j, PLAYER_COLOR);
      } else if (matrix1Screen[i][j] == 2) {
        matrix1.drawPixel(i, j, GUN_COLOR);
      } else if (matrix1Screen[i][j] == 3) {
        matrix1.drawPixel(i, j, ENEMY_COLOR);
      }
    }
  }
  
  matrix1.writeDisplay();
}

void drawMatrix2() {
  // Take matrix2screen and loop through it and draw every pixel
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (matrix2Screen[i][j] == 1) {
        matrix2.drawPixel(i, j, PLAYER_COLOR);
      } else if (matrix2Screen[i][j] == 2) {
        matrix2.drawPixel(i, j, GUN_COLOR);
      } else if (matrix2Screen[i][j] == 3) {
        matrix2.drawPixel(i, j, ENEMY_COLOR);
      }
    }
  }

  matrix2.writeDisplay();
}
