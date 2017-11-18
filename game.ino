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

// Joystick
const int joystickXAxis = A1;
const int joystickYAxis = A0;
int joystickSelect = 7;
int xReading;
int yReading;
int shoot;
String dirToMove;

// Player variables
int playerX = 0;
int playerY = 0;
int playerMatrix = 0;

// Gun variables
int gunX = 0;
int gunY = 0;
int gunMatrix = 0;

void setup() {
  Serial.begin(9600);

  pinMode(joystickXAxis, INPUT);
  pinMode(joystickYAxis, INPUT);
  pinMode(joystickSelect, INPUT_PULLUP);
  
  randomSeed(analogRead(0));

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
    // Generate some enemies
    generateEnemies();
    delay(500);
  
    // Move enemies
    moveEnemies();

    drawMatrix1();
    drawMatrix2();
    
    delay(random(1800,2800));
  }
}

void movePlayer() {
  xReading = analogRead(joystickXAxis);
  yReading = analogRead(joystickYAxis);
  int newPlayerX = playerX;
  int newPlayerY = playerY;
  int newPlayerMatrix = playerMatrix;
  int newGunX = gunX;
  int newGunY = gunY;
  int newGunMatrix = gunMatrix;

  // Determine which way the player is trying to move
  if (xReading < 200) {
    dirToMove = "LEFT";
    if (playerMatrix == 1 and playerX == 0) {
      newPlayerX = playerX;
    } else if (playerMatrix == 2 && playerX == 0) {
      newPlayerMatrix = 1;
      newPlayerX = 7;
    } else {
      newPlayerX--;
    }
  } else if (xReading > 900) {
    dirToMove = "RIGHT";
    if (playerMatrix == 2 and playerX == 7) {
      newPlayerX = playerX;
    } else if (playerMatrix == 1 && playerX == 7) {
      newPlayerMatrix = 2;
      newPlayerX = 0;
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

  // What would the gun's new position be?
  if (newPlayerMatrix == 1 && newPlayerX == 7) {
    newGunMatrix = 2;
    newGunX = 0;
    newGunY = newPlayerY;
  } else if (newPlayerMatrix == 2) {
      gunX = newPlayerX + 1;
      gunY = newPlayerY;
  }

  // Now that we know what matrix we're going to be on and our new position, check if that position is open to move to
  if (newPlayerMatrix == 1) {
    if (matrix1Screen[newPlayerX][newPlayerY] == 0) {
      // There is absolutely nothing there, so we're good.
      // Have to move the player and the gun.
    } else if (matrix1Screen[newPlayerX][newPlayerY] == 2) {
      // My gun is there, it also needs to move tho
    } else if (matrix1Screen[newPlayerX][newPlayerY] == 3) {
      // I want to move where an enemy is
    }
  } else if (newPlayerMatrix == 2) {
    
  }
}

void moveEnemies() {
  for (int i = 0; i < 8; i++) {
    Serial.println(i);
    for (int j = 0; j < 8; j++) {
      
      //Looping through matrix 1
      if (i == 0 && matrix1Screen[i][j] == 3) {
        matrix1Screen[i][j] = 0;
      } else if (matrix1Screen[i][j] == 3 || matrix2Screen[i][j] == 3) {
        // We found an enemy! What direction should we move them in?
        // FORWARD = 0 FORWARDUP = 1 FORWARDDOWN = 2
        int val = random(3);
        String dir = "";
        
        if (val == 0) {
          dir = "FORWARD";
        } else if (val == 1) {
          dir = "FORWARDUP";
        } else if (val == 2) {
          dir = "FORWARDDOWN";
        }

        // Now that we know what direction to move, let's check if that spot is open in the right matrix
        if (matrix1Screen[i][j] == 3) {
          if (dir.equals("FORWARD")) {
            if (matrix1Screen[i-1][j] == 0) {
              // The spot is open
              // Turn off the current spot and turn on the next spot
              matrix1Screen[i][j] = 0;
              matrix1Screen[i-1][j] = 3;
            }
          } else if (dir.equals("FORWARDUP")) {
            if (matrix1Screen[i-1][j-1] == 0) {
              // The spot is open
              // Turn off the current spot and turn on the next spot
              matrix1Screen[i][j] = 0;
              matrix1Screen[i-1][j-1] = 3;
            }
          } else if (dir.equals("FORWARDDOWN")) {
            if (matrix1Screen[i-1][j+1] == 0) {
              // The spot is open
              // Turn off the current spot and turn on the next spot
              matrix1Screen[i][j] = 0;
              matrix1Screen[i-1][j+1] = 3;
            }
          }
        } else if (matrix2Screen[i][j] == 3) {
            if (i == 0) {
              // Have to move to matrix 1
              if (dir.equals("FORWARD")){
                if (matrix1Screen[7][j] == 0) {
                  matrix2Screen[i][j] = 0;
                  matrix1Screen[7][j] = 3;
                }
              } else if (dir.equals("FORWARDUP")) {
                if (matrix1Screen[7][j-1] == 0) {
                  matrix2Screen[i][j] = 0;
                  matrix1Screen[7][j-1] = 3;
                }
              } else if (dir.equals("FORWARDDOWN")) {
                if (matrix1Screen[7][j+1] == 0) {
                  matrix2Screen[i][j] = 0;
                  matrix1Screen[7][j+1] = 3;
                }
              }
            } else {
                if (dir.equals("FORWARD")) {
                  if (matrix2Screen[i-1][j] == 0) {
                    // The spot is open
                    // Turn off the current spot and turn on the next spot
                    matrix2Screen[i][j] = 0;
                    matrix2Screen[i-1][j] = 3;
                  }
                } else if (dir.equals("FORWARDUP")) {
                    if (matrix2Screen[i-1][j-1] == 0) {
                      // The spot is open
                      // Turn off the current spot and turn on the next spot
                      matrix2Screen[i][j] = 0;
                      matrix2Screen[i-1][j-1] = 3;
                    }
                } else if (dir.equals("FORWARDDOWN")) {
                    if (matrix2Screen[i-1][j+1] == 0) {
                      // The spot is open
                      // Turn off the current spot and turn on the next spot
                      matrix2Screen[i][j] = 0;
                      matrix2Screen[i-1][j+1] = 3;
                    }
                }
            }
        }
      }
    }
  }
}

void generatePlayer() {
  // The player will always be on matrix 1
  playerMatrix = 1;

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

  // The matrix that the gun is on is the same as the player upon generation
  gunMatrix = 1;

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
