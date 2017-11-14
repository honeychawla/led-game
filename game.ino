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

  generatePlayer();
  generateGun();
  generateEnemies();

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

  moveEnemies();
  delay(1000);
  drawMatrix1();
  drawMatrix2();
  delay(1000); 
}

void moveEnemies() {
  // {enemyMatrix, enemyX, enemyY, enemyAlive}
  for (int i = 0; i < 8; i++) {
    int oldMatrix = enemies[i][0];
    int oldX = enemies[i][1];
    int oldY = enemies[i][2];
    int alive = enemies[i][3];
    
    boolean found = false;

    int newMatrix = oldMatrix;
    int newX = oldX;
    int newY = oldY;
    
    if (alive == 1) {
      while (!found) {
        // UP = 0 DOWN = 1 LEFT = 2 RIGHT = 3
        int val = random(4);
        String dir = "";
        
        if (val == 0) {
          dir = "UP";
        } else if (val == 1) {
          dir = "DOWN";
        } else if (val == 2) {
          dir = "LEFT";
        } else if (val == 3) {
          dir = "RIGHT";
        }

        // Handle literal edge cases        
        if ((oldY == 0 && dir.equals("UP")) ||
            (oldY == 7 && dir.equals("DOWN")) ||
            (oldMatrix == 1 && oldX == 0 && dir.equals("LEFT")) ||
            (oldMatrix == 2 && oldX == 7 && dir.equals("RIGHT"))) {
          // Top row supposed to go up or bottom row and supposed to go down
          found = false;
        } else {
          // We're not in an edge case, so now we have to check if the spot we want to move to is open
          if (dir.equals("UP")) {
            if (oldMatrix == 1) {
              if (matrix1Screen[oldX][oldY - 1] == 0) {
                newY = oldY - 1;
                found = true;
              }
            } else if (oldMatrix == 2) {
              if (matrix2Screen[oldX][oldY - 1] == 0) {
                newY = oldY - 1;
                found = true;
              }
            }
          } else if (dir.equals("DOWN")) {
            if (oldMatrix == 1) {
              if (matrix1Screen[oldX][oldY + 1] == 0) {
                newY = oldY + 1;
                found = true;
              }
            } else if (oldMatrix == 2) {
              if (matrix2Screen[oldX][oldY + 1] == 0) {
                newY = oldY + 1;
                found = true;
              }
            }
          } else if (dir.equals("LEFT")) {
            if (oldMatrix == 1) {
              if (matrix1Screen[oldX - 1][oldY] == 0) {
                newX = oldX - 1;
                found = true;
              }
            } else if (oldMatrix == 2) {
              if (oldX == 0) {
                if (matrix1Screen[7][oldY] == 0) {
                  newMatrix = 1;
                  newX = 7;
                  found = true;
                }
              } else if (oldX > 0) {
                if (matrix2Screen[oldX - 1][oldY] == 0) {
                  newX = oldX - 1;
                  found = true;
                }
              }
            }
          } else if (dir.equals("RIGHT")) {
            if (oldMatrix == 1) {
              if (oldX == 7) {
                if (matrix2Screen[0][oldY] == 0) {
                  newMatrix = 2;
                  newX = 0;
                  found = true;
                }
              } else if (oldX < 7) {
                if (matrix1Screen[oldX + 1][oldY] == 0) {
                  newX = oldX + 1;
                  found = true;
                }
              }
            } else if (oldMatrix == 2) {
              if (matrix2Screen[oldX + 1][oldY] == 0) {
                newX = oldX + 1;
                found = true;
              }
            }
          }
        }
      }

      if (found) {
        // Update enemies array
        enemies[i][0] = newMatrix;
        enemies[i][1] = newX;
        enemies[i][2] = newY;
  
        // Update screen matrices
        if (oldMatrix == 1) {
          matrix1Screen[oldX][oldY] = 0;
        } else if (oldMatrix == 2) {
          matrix2Screen[oldX][oldY] = 0;
        }
  
        if (newMatrix == 1) {
          matrix1Screen[newX][newY] = 3;
        } else if (newMatrix == 2) {
          matrix2Screen[newX][newY] = 3;
        }
      }
    }
  }
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
    boolean conflict = true;
    int enemyMatrix;
    int enemyX;
    int enemyY;
    int enemyAlive = 1;

    // Find new locations until there isn't a conflict
    while (conflict) {
      // Determine which matrix they're on
      enemyMatrix = random(2) + 1;
      // Determine their x and y location
      enemyX = random(8);
      enemyY = random(8);

      // Make sure another entity isn't at that location
      if ((enemyMatrix == 1 && matrix1Screen[enemyX][enemyY] == 0) || (enemyMatrix == 2 && matrix2Screen[enemyX][enemyY] == 0)) {
        if (enemyMatrix == playerMatrix) {
          // They're on the same matrix
          int diffPlayerX = abs(playerX - enemyX);
          int diffPlayerY = abs(playerY - enemyY);
  
          int diffGunX = abs(gunX - enemyX);
          int diffGunY = abs(gunY - enemyY);
  
          if (diffPlayerX > 1 && diffPlayerY > 1 && diffGunX > 1 && diffPlayerY > 1) {
            conflict = false;
          }
        } else {
          // They're on different matrices
          if (enemyMatrix == 1) {
            if (playerX > 0 && gunX > 0) {
              conflict = false;
            }
          } else if (enemyMatrix == 2) {
            if (playerX != 7 && gunX != 7) {
              conflict = false;
            }
          }
  
          if (conflict) {
            // Actually have to check for conflict, through their y values
            int diffPlayerY = abs(enemyY - playerY);
            int diffGunY = abs(enemyY - gunY);
    
            if (diffPlayerY > 1 && diffGunY > 1) {
              conflict = false;
            }
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
      if (matrix2Screen[i][j] == 1) {
        matrix2.drawPixel(i, j, PLAYER_COLOR);
      } else if (matrix2Screen[i][j] == 2) {
        matrix2.drawPixel(i, j, GUN_COLOR);
      } else if (matrix2Screen[i][j] == 3) {
        matrix2.drawPixel(i, j, ENEMY_COLOR);
      } else if (matrix2Screen[i][j] == 0) {
        matrix2.drawPixel(i, j, 0);
      }
    }
  }

  matrix2.writeDisplay();
}
