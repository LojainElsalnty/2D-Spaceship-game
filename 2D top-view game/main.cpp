#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <stdlib.h>
#include <cmath>
#include <random>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <unistd.h>

// for Sound
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


void Display();
void idle();
void specialKeys(int key, int x, int y);
void specialKeysUp(int key, int x, int y);
void drawSpaceship();
void adjustSpaceshipDirection();
void generatePositions();
void drawStars();
void drawGoal();
void drawShields();
void drawScorePowerUp();
void drawAsteroids();
void drawBorder();
void drawHealthBar();
void drawAnimBackground();
void handleAsteroidCollision(int i);
void handleStarCollision(int i);
void handleScoreCollision(int i);
void handleShieldCollision(int i);
void renderEndScene();
void toPrint();
bool checkCollisionWithObject();
void checkCollision();
bool checkCollisionWithGoal();

// music
void  playBackgroundSound();
void  resumeBackgroundSound();

void print(int x, int y, char* string);
void drawCircle(int x, int y, float r);

// not used till now

int checkEmptyPosition(float x, float y, int n);


bool upPressed = false;
bool downPressed = false;
bool leftPressed = false;
bool rightPressed = false;
float spaceshipX =300.0f;
float spaceshipY = 300.0f;
float spaceshipXmovement= 0.0f;
float spaceshipYmovement = 0.0f;
float spaceshipROT = 0.0f;
float movementSpeed = 10.0f;
int spaceshipDirection=2;  // spaceship face is directed to up at first of the game
int timeAnim=1;
int anim=1;
int earthAnim=1;
bool shieldON=false;
bool doubleScoreON=false;
bool collideWithAlien= false;

char* p0s[20];
int playerScore=0;
int lives= 5;
int gameTime = 60; // for now game time is just 60 seconds
int secondsInt=0;
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

enum GameState { GAME_PLAYING, GAME_OVER };
GameState gameState;

// Music
Mix_Music* bgm ;
Mix_Chunk* soundEffect;
int channel;
std::chrono::high_resolution_clock::time_point soundTime;


//Power ups time
std::chrono::high_resolution_clock::time_point scoreCollisionTime;
std::chrono::high_resolution_clock::time_point shieldCollisionTime;


const int numStars = 5;

float position[14][2];  // position[0]--> spaceship, position[1-5]--> star collactables(5)
                         // position[6-7]--> sheild power up, position[8-9]--> score power up
                         // position[10-14]--> obstacles(5)


void playBackgroundSound(){
    // Initialize SDL
     if (SDL_Init(SDL_INIT_AUDIO) < 0) {
         printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
         return;
     }

     // Initialize SDL_mixer
     if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
         printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
         return;
     }

     // Load music
     Mix_Music* bgm = Mix_LoadMUS("/Users/loginelsalnty/Desktop/Semester 7 guc/Graphics/Assignments/2D top-view game/audio/Breakout.mp3");
     if (bgm == NULL) {
         printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
         return;
     }

     // Play music
     Mix_PlayMusic(bgm, -1);
    
}
void  resumeBackgroundSound(){
    auto currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> duration = currentTime - soundTime;
        float secondsScore = duration.count();
        if (secondsScore >= 2.0f) {
             Mix_HaltChannel(channel);
        }
    
}
void playAlienSound(){
    // Initialize SDL
     if (SDL_Init(SDL_INIT_AUDIO) < 0) {
         printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
         return;
     }

     // Initialize SDL_mixer
     if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
         printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
         return;
     }

     // Load music
     Mix_Music* bgm = Mix_LoadMUS("/Users/loginelsalnty/Desktop/Semester 7 guc/Graphics/Assignments/2D top-view game/audio/alien.mp3");
     if (bgm == NULL) {
         printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
         return;
     }

     // Play music
     Mix_PlayMusic(bgm, -1);
    
}
void playLoseSound(){
    // Initialize SDL
     if (SDL_Init(SDL_INIT_AUDIO) < 0) {
         printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
         return;
     }

     // Initialize SDL_mixer
     if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
         printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
         return;
     }

     // Load music
     Mix_Music* bgm = Mix_LoadMUS("/Users/loginelsalnty/Desktop/Semester 7 guc/Graphics/Assignments/2D top-view game/audio/lose.mp3");
     if (bgm == NULL) {
         printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
         return;
     }

     // Play music
     Mix_PlayMusic(bgm, -1);
}
void playSoundEffect(const char* filePath) {
    if (soundEffect != nullptr) {
        Mix_FreeChunk(soundEffect);
        soundEffect = nullptr;
    }

    soundEffect = Mix_LoadWAV(filePath);
    if (soundEffect == nullptr) {
        printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }

     channel = Mix_PlayChannel(-1, soundEffect, 0);
    if (channel == -1) {
        printf("Failed to play sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }
    resumeBackgroundSound();
}

void playCollactableSound() {
    playSoundEffect("/Users/loginelsalnty/Desktop/Semester 7 guc/Graphics/Assignments/2D top-view game/audio/collectable.mp3");
}

void playObstacleSound() {
    playSoundEffect("/Users/loginelsalnty/Desktop/Semester 7 guc/Graphics/Assignments/2D top-view game/audio/obstacle.mp3");
}

int main(int argc,  char** argr) {
    glutInit(&argc, argr);
    
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Space Exploration");
    
    srand(time(0));  // stars generated but not randomly after each rub
    generatePositions();
    
    playerScore =0; // re-intialize the score in every run
    lives=5;
    secondsInt=0;
    startTime = std::chrono::high_resolution_clock::now();
    gameState = GAME_PLAYING; // Set the initial state to playing.
    playBackgroundSound();

    
    glutDisplayFunc(Display);
    glutSpecialUpFunc(specialKeysUp);
    glutSpecialFunc(specialKeys);
    glutIdleFunc(idle);


   
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    gluOrtho2D(0.0, 600, 0.0, 650);
    glutMainLoop();
    return 0;
}


void Display(){
    glClear(GL_COLOR_BUFFER_BIT);
    drawSpaceship();
    drawStars();
    drawGoal();
    drawShields();
    drawScorePowerUp();
    drawAsteroids();
    drawBorder();
    drawHealthBar();
    drawAnimBackground();
   
    toPrint();

    if(gameTime - secondsInt==0 || lives==0 || collideWithAlien){
        gameState=GAME_OVER;
        renderEndScene();
    }
   
    glutSwapBuffers();
    glFlush();
}
void renderEndScene(){

    glClear(GL_COLOR_BUFFER_BIT);
    if(collideWithAlien){
        playAlienSound();
        sprintf((char*)p0s, "You won: %d ! :)", playerScore);
    }
    else{
        //playSound(5);
        playLoseSound();
        sprintf((char*)p0s, "You lose :( ");
    }
    print(250, 350, (char*)p0s);
    glutSwapBuffers();
    glFlush();
   


}
void toPrint(){
    //Displayed throught the game
    // Score
    sprintf((char*)p0s, "Score: %d", playerScore);
    print(210, 620, (char*)p0s);
    // Time
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = currentTime - startTime;
    float seconds = duration.count();
    secondsInt = static_cast<int>(seconds);
    sprintf((char*)p0s, "Time: %d s",gameTime- secondsInt);
    print(10, 620, (char*)p0s);
    
    // Power Ups ON
    // Double Score
    if(doubleScoreON){
        std::chrono::duration<float> duration = currentTime - scoreCollisionTime;
        float secondsScore = duration.count();
        if (secondsScore >= 5.0f) {
            doubleScoreON= false;
        } else {
            // Update the countdown
            int remainingTime = static_cast<int>(secondsScore);
            sprintf((char*)p0s, "x2                     %d s",5-remainingTime);
            print(175, 620, (char*)p0s);
        }
    }
    
    // Shield
    if(shieldON){
        std::chrono::duration<float> duration = currentTime - shieldCollisionTime;
        float secondsShield = duration.count();
        if (secondsShield >= 5.0f) {
            shieldON= false;
        } else {
            // Update the countdown
            int remainingTime = static_cast<int>(secondsShield);
            sprintf((char*)p0s,"Shield ON",5-remainingTime);
            print(490, 620, (char*)p0s);
        }
    }
}

// Functionality
void generatePositions(){
    // asteroids positions
    position[10][0]= 314;
    position[10][1]=542;
    
    position[11][0]= 187;
    position[11][1]=125;
    
    position[12][0]= 490;
    position[12][1]=398;
    
    position[13][0]= 75;
    position[13][1]=279;
    
    position[14][0]= 526;
    position[14][1]=62;
    
    // generate Stars positions
        int i =0;
        while( i<numStars){
            float tempx, tempy;
            do {
                tempx = static_cast<float>(rand() % 550);  // Random x-coordinate between 0 and 600
                tempy = static_cast<float>(rand() % 550);  // Random y-coordinate between 0 and 600
            } while (((tempx >= spaceshipX - 50 && tempx<= spaceshipX + 50 ) && (tempy >= spaceshipY - 50 && tempy <= spaceshipY + 50 )));
            
            if( (tempx>60 && tempx<500) && (tempy>60 && tempy<500)    ){
                
                position[i+1][0]= tempx;
                position[i+1][1]=tempy;
                i++;
            }
        }
    
    // generate shield power up  positions
         i =0;
        while( i<2){
            float tempx, tempy;
            
            do {
                tempx = static_cast<float>(rand() % 550);  // Random x-coordinate between 0 and 600
                tempy = static_cast<float>(rand() % 550);  // Random y-coordinate between 0 and 600
            } while (((tempx >= spaceshipX - 50 && tempx<= spaceshipX + 50 ) && (tempy >= spaceshipY - 50 && tempy <= spaceshipY + 50 )));
            
            if( (tempx>60 && tempx<500) && (tempy>60 && tempy<500) ){
               
                position[i+6][0]= tempx;
                position[i+6][1]=tempy;
                i++;
            }
        }
    
    // generate score x2 power up  positions
         i =0;
        while( i<2){
            float tempx, tempy;
            
            do {
                tempx = static_cast<float>(rand() % 550);  // Random x-coordinate between 0 and 600
                tempy = static_cast<float>(rand() % 550);  // Random y-coordinate between 0 and 600
            } while (((tempx >= spaceshipX - 50 && tempx<= spaceshipX + 50 ) && (tempy >= spaceshipY - 50 && tempy <= spaceshipY + 50 )));
            
            if( (tempx>60 && tempx<500) && (tempy>60 && tempy<500) ){
               
                position[i+8][0]= tempx;
                position[i+8][1]=tempy;
                i++;
            }
        }
}
void print(int x, int y, char* string){
    int len, i;
    glColor3f(1.0f, 1.0f, 1.0f);
    //set the position of the text in the window using the x and y coordinates
    glRasterPos2f(x, y);

    //get the length of the string to display
    len = (int)strlen(string);

    //loop to display character by character
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
    }
}

// Draw Objects
void drawBorder() {
    glLineWidth(5.0); // Set the line width
    glColor3f(1.0f, 1.0f, 1.0f); // Set the color to white
    
   
    // Draw a rectangle
    glBegin(GL_LINE_LOOP);
    glVertex3f(10.0f, 10.0f, 0.0f);
    glVertex3f(10.0f, 600.0f, 0.0f);
    glVertex3f(590.0f, 600.0f, 0.0f);
    glVertex3f(590.0f, 10.0f, 0.0f);
    glEnd();
    
    // Stars
    glPointSize(10.0f);
    glColor3f(1.0, 1.0, 0.0); // Yellow color
    glBegin(GL_POINTS);
    glVertex3f(10.0f, 10.0f, 0.0f);
    glVertex3f(10.0f, 600.0f, 0.0f);
    glVertex3f(590.0f, 600.0f, 0.0f);
    glVertex3f(590.0f, 10.0f, 0.0f);
    glEnd();
    
    glPointSize(15.0f);
    glColor3f(1.0, 1.0, 0.0); // Yellow color
    glBegin(GL_POINTS);
    glVertex3f(70.0f, 10.0f, 0.0f);
    glVertex3f(80.0f, 600.0f, 0.0f);
    glVertex3f(590.0f, 90.0f, 0.0f);
    glVertex3f(590.0f, 290.0f, 0.0f);
    glEnd();
    
    glPointSize(20.0f);
    glColor3f(1.0, 1.0, 0.0); // Yellow color
    glBegin(GL_POINTS);
    glVertex3f(10.0f, 100.0f, 0.0f);
    glVertex3f(10.0f, 500.0f, 0.0f);
    glVertex3f(590.0f, 420.0f, 0.0f);
    glVertex3f(590.0f, 500.0f, 0.0f);
    glEnd();
    
    glPointSize(15.0f);
    glColor3f(1.0, 1.0, 0.0); // Yellow color
    glBegin(GL_POINTS);
    glVertex3f(10.0f, 400.0f, 0.0f);
    glVertex3f(300.0f, 600.0f, 0.0f);
    glVertex3f(490.0f, 10.0f, 0.0f);
    glVertex3f(220.0f, 10.0f, 0.0f);
    
    glEnd();
}
void drawAnimBackground(){
    
    drawCircle(140, 620, 15);
    glColor3f(0.0f, 0.39f, 0.0f); // Set the color to a shade of green for land
    glPointSize(7.0f);
    
    glPushMatrix();
    glTranslatef(140.0f, 620.0f, 0.0f);
    
    
    glRotatef(earthAnim, 0, 0, 1);
    
    
    glBegin(GL_POINTS);
    glVertex3f(5.0f, 0.0f, 0.0f);  // These coordinates create a circle of points around the center.
//    glVertex3f(4.5f, 2.6f, 0.0f);
    glVertex3f(5.0f, 1.0f, 0.0f);
    glVertex3f(-4.0f, -2.9f, 0.0f);

    glVertex3f(-5.0f, -0.0f, 0.0f);
//    glVertex3f(-4.5f, -2.6f, 0.0f);
    glVertex3f(-4.0f, -2.9f, 0.0f);
    
    glVertex3f(6.0f, 8.5f, 0.0f);
//    glVertex3f(7.5f, 7.6f, 0.0f);
//    glVertex3f(8.0f, 6.9f, 0.0f);
    glEnd();
  
    glPopMatrix();
}

void drawCircle(int x, int y, float r) {
  glColor3f(0.0f, 0.5f, 0.78f); // Set the color to a shade of blue for oceans
  glPushMatrix();
  glTranslatef(x, y, 0);
  GLUquadric *quadObj = gluNewQuadric();
  gluDisk(quadObj, 0, r, 50, 50);
  glPopMatrix();
    
    
}
void drawHealthBar() {

    for(int i =0 ; i<5;i++){
        // Draw the background of the health bar (red for the lost health)
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glVertex3f(380.0+(20*i),610.0f, 0.0f);
        glVertex3f(380.0+(20*i),640.0f,0.0f);
        glVertex3f(400.0+(20*i),640.0f,0.0f);
        glVertex3f(400.0+(20*i),610.0f,0.0f);
        glEnd();
    }
        for(int i =0 ; i<lives ;i++){
        // Draw the actual health (green part)
        glColor3f(0.0, 1.0, 0.0);
        glBegin(GL_QUADS);
        glVertex3f(380.0+(20*i),610.0f, 0.0f);
        glVertex3f(380.0+(20*i),640.0f,0.0f);
        glVertex3f(400.0+(20*i),640.0f,0.0f);
        glVertex3f(400.0+(20*i),610.0f,0.0f);
        glEnd();
        

    }
}
// Asteroids obstacles
void drawAsteroids(){
    for (int i = 10; i < 15; i++) {
        float x = position[i][0];
        float y = position[i][1];
        
        glPushMatrix();
        glTranslatef(x, y, 0);
        glScaled(0.5, 0.5, 1);
        glRotated(anim, 0, 0, 1);
        
        // Asteroid body
        glBegin(GL_POLYGON);
        glColor3f(0.7f, 0.7f, 0.7f);  // Set color to light gray
        glVertex3f(0.0f, -30.0f, 0.0f);
        glVertex3f(-30.0f, -10.0f, 0.0f);
        glVertex3f(-30.0f, 10.0f, 0.0f);
        glVertex3f(0.0f, 30.0f, 0.0f);
        glVertex3f(30.0f, 10.0f, 0.0f);
        glVertex3f(30.0f, -10.0f, 0.0f);
        glEnd();
        
        // Asteroid Boarder
        glLineWidth(3.0);
        glBegin(GL_LINE_STRIP);
        glColor3f(0.0f, 0.0f, 0.0f);  // Set color to black
        glVertex3f(0.0f, -30.0f, 0.0f);
        glVertex3f(-30.0f, -10.0f, 0.0f);
        glVertex3f(-30.0f, 10.0f, 0.0f);
        glVertex3f(0.0f, 30.0f, 0.0f);
        glVertex3f(30.0f, 10.0f, 0.0f);
        glVertex3f(30.0f, -10.0f, 0.0f);
        glVertex3f(0.0f, -30.0f, 0.0f);
        glEnd();
        
        // rocks on Asteroid
        glPointSize(3.0f);
        glColor3f(0.3f, 0.3f, 0.3f);  // Set color to dark gray
        glBegin(GL_POINTS);
        glVertex3f(-20.0f, 0.0f, 0.0f);
        glVertex3f(10.0f, -10.0f, 0.0f);
        glEnd();
        glPointSize(5.0f);
        glColor3f(0.3f, 0.3f, 0.3f);  // Set color to dark gray
        glBegin(GL_POINTS);
        glVertex3f(0.0f, 10.0f, 0.0f);
        glVertex3f(-10.0f, -10.0f, 0.0f);
        glEnd();
        
        glPopMatrix();
    }
}

// Power ups
void drawShields(){
    for (int i = 6; i < 8; i++) {
        float x = position[i][0];
        float y = position[i][1];
        
        glPushMatrix();
        glTranslatef(x, y, 0);
        glScaled(0.5, 0.5, 1);
        glRotated(anim, 0, 0, 1);
        
        // lower part of shield
        glBegin(GL_TRIANGLES);
        glColor3f(0.64f, 0.16f, 0.16f); // Set color to brown
        glVertex3f(40.0f, 10.0f, 0.0f);
        glVertex3f(10.0f, 70.0f, 0.0f);
        glVertex3f(70.0f, 70.0f, 0.0f);
        glEnd();
        
        // upper part of shield
        glBegin(GL_QUADS);
        glColor3f(0.64f, 0.16f, 0.16f); // Set color to brown
        glVertex3f(10.0f, 70.0f, 0.0f);
        glVertex3f(30.0f, 90.0f, 0.0f);
        glVertex3f(50.0f, 90.0f, 0.0f);
        glVertex3f(70.0f, 70.0f, 0.0f);
        glEnd();
        
        // shape on ploygon
        glBegin(GL_POLYGON);
        glColor3f(0.75f, 0.75f, 0.75f);  // Set color to silver
        glVertex3f(40.0f, 70.0f, 0.0f);
        glVertex3f(50.0f, 60.0f, 0.0f);
        glVertex3f(40.0f, 50.0f, 0.0f);
        glVertex3f(30.0f, 60.0f, 0.0f);
        glEnd();
        
        // sheild border
        glLineWidth(3.0);
        glBegin(GL_LINE_STRIP);
        // glColor3f(0.64f, 0.16f, 0.16f);
        glColor3f(0.75f, 0.75f, 0.75f);  // Set color to silver
        
        glVertex3f(40.0f, 10.0f, 0.0f);
        glVertex3f(10.0f, 70.0f, 0.0f);
        glVertex3f(30.0f, 90.0f, 0.0f);
        glVertex3f(50.0f, 90.0f, 0.0f);
        glVertex3f(70.0f, 70.0f, 0.0f);
        glVertex3f(70.0f, 70.0f, 0.0f);
        glVertex3f(40.0f, 10.0f, 0.0f);
        glEnd();
        
        glPopMatrix();
    }
    
}

// Score Power up
void drawScorePowerUp(){
    for (int i = 8; i < 10; i++) {
        float x = position[i][0];
        float y = position[i][1];
        
        glPushMatrix();
        glTranslatef(x, y, 0);
        glScaled(0.5, 0.5, 1);
        glRotated(anim, 0, 0, 1);
        
        // red square
        glBegin(GL_POLYGON);
        glColor3f(1.0f, 0.0f, 0.0f); // Set color to red
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 50.0f, 0.0f);
        glVertex3f(50.0f, 50.0f, 0.0f);
        glVertex3f(50.0f, 0.0f, 0.0f);
        glEnd();
        
        // X
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(15.0f, 10.0f, 0.0f);
        glVertex3f(5.0f, 40.0f, 0.0f);
        glVertex3f(10.0f, 45.0f, 0.0f);
        glVertex3f(20.0f, 16.0f, 0.0f);
        glEnd();
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(5.0f, 16.0f, 0.0f);
        glVertex3f(15.0f, 45.0f, 0.0f);
        glVertex3f(20.0f, 40.0f, 0.0f);
        glVertex3f(10.0f, 10.0f, 0.0f);
        glEnd();
        
        // 2
        glLineWidth(3.0);
        glBegin(GL_LINES);
        glColor3f(1.0f, 1.0f, 1.0f);  // Set color to white
        glVertex3f(25.0f, 40.0f, 0.0f);
        glVertex3f(40.0f, 40.0f, 0.0f);
        
        glVertex3f(40.0f, 40.0f, 0.0f);
        glVertex3f(40.0f, 25.0f, 0.0f);
        
        glVertex3f(40.0f, 25.0f, 0.0f);
        glVertex3f(25.0f, 25.0f, 0.0f);
        
        glVertex3f(25.0f, 25.0f, 0.0f);
        glVertex3f(25.0f, 10.0f, 0.0f);
        
        glVertex3f(25.0f, 10.0f, 0.0f);
        glVertex3f(40.0f, 10.0f, 0.0f);
        glEnd();
        
        // Score power up border
        glLineWidth(3.0);
        glBegin(GL_LINE_STRIP);
        glColor3f(1.0f, 1.0f, 1.0f);  // Set color to white
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 50.0f, 0.0f);
        glVertex3f(50.0f, 50.0f, 0.0f);
        glVertex3f(50.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glEnd();
        
        glPopMatrix();
        
    }
}

// alien
void drawGoal(){
    
    glPushMatrix();
    glRotated(anim, 0, 0, 1);
    glScaled(0.5, 0.5, 1);
    
    // lower part of Alien face
    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 1.0f, 0.5f);
    glVertex3f(60.0f, 20.0f, 0.0f);
    glVertex3f(20.0f, 70.0f, 0.0f);
    glVertex3f(100.0f, 70.0f, 0.0f);
    glEnd();
    
    // upper part of Alien face
    glBegin(GL_QUADS);
    glColor3f(0.0f, 1.0f, 0.5f);  // Set color to lime green
    glVertex3f(20.0f, 70.0f, 0.0f);
    glVertex3f(40.0f, 100.0f, 0.0f);
    glVertex3f(80.0f, 100.0f, 0.0f);
    glVertex3f(100.0f, 70.0f, 0.0f);
    glEnd();
    
    // Alien eyes
    glPointSize(8.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
    glVertex3f(50.0f, 80.0f, 0.0f);
    glVertex3f(70.0f, 80.0f, 0.0f);
    glEnd();
    
    glPointSize(5.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    glVertex3f(53.0f, 82.0f, 0.0f);
    glVertex3f(73.0f, 82.0f, 0.0f);
    glEnd();
    
    // Alien ears
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glColor3f(0.0f, 1.0f, 0.5f);
    glVertex3f(80.0f, 100.0f, 0.0f);
    glVertex3f(90.0f, 120.0f, 0.0f);
    glVertex3f(40.0f, 100.0f, 0.0f);
    glVertex3f(30.0f, 120.0f, 0.0f);
    glEnd();
    
    // Alien mouth
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(50.0f, 50.0f, 0.0f);
    glVertex3f(50.0f, 60.0f, 0.0f);
    glVertex3f(70.0f, 50.0f, 0.0f);
    glVertex3f(70.0f, 60.0f, 0.0f);
    glEnd();
    
    glPopMatrix();
}

//collectables

void drawStars() {
    for (int i = 0; i < numStars; i++) {
        float x = position[i+1][0];
        float y = position[i+1][1];
        
        glPushMatrix();
        glTranslatef(x, y, 0);
        glScaled(0.5, 0.5, 1);
        glRotated(anim, 0, 0, 1);
            
        // Draw the outer part of the star
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 1.0f, 0.0f);  // Set color to yellow
        glVertex3f(45.0f, 80.0f, 0.0f);
        glVertex3f(10.0f, 45.0f, 0.0f);
        glVertex3f(80.0f, 45.0f, 0.0f);
        glEnd();
            
        // Draw the inner part of the star
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);  // Set color to yellow
        glVertex3f(20.0f, 20.0f, 0.0f);
        glVertex3f(70.0f, 20.0f, 0.0f);
        glVertex3f(70.0f, 70.0f, 0.0f);
        glVertex3f(20.0f, 70.0f, 0.0f);
        glEnd();
          
        // Draw the outer part of the star
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 1.0f, 0.0f);  // Set color to yellow
        glVertex3f(45.0f, 10.0f, 0.0f);
        glVertex3f(10.0f, 45.0f, 0.0f);
        glVertex3f(80.0f, 45.0f, 0.0f);
        glEnd();
            
        // Draw the outline of the star
        glLineWidth(4.0);
        glBegin(GL_LINE_STRIP);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(10.0f, 45.0f, 0.0f);
        glVertex3f(20.0f, 51.0f, 0.0f);
        glVertex3f(20.0f, 70.0f, 0.0f);
        glVertex3f(35.0f, 70.0f, 0.0f);
        glVertex3f(45.0f, 80.0f, 0.0f);
        glVertex3f(51.0f, 70.0f, 0.0f);
        glVertex3f(70.0f, 70.0f, 0.0f);
        glVertex3f(70.0f, 51.0f, 0.0f);
        glVertex3f(80.0f, 45.0f, 0.0f);
        glVertex3f(70.0f, 35.0f, 0.0f);
        glVertex3f(70.0f, 20.0f, 0.0f);
        glVertex3f(51.0f, 20.0f, 0.0f);
        glVertex3f(45.0f, 10.0f, 0.0f);
        glVertex3f(35.0f, 20.0f, 0.0f);
        glVertex3f(20.0f, 20.0f, 0.0f);
        glVertex3f(20.0f, 35.0f, 0.0f);
        glVertex3f(10.0f, 45.0f, 0.0f);
        glEnd();
        
        glPopMatrix();
    }
}
    
// Player
void drawSpaceship()
{
//    std::cout << "lastPressed: " << lastPressed << std::endl;
    
    adjustSpaceshipDirection();
    glPushMatrix();
  // old
//    glTranslatef(spaceshipXmovement+spaceshipX, spaceshipYmovement+spaceshipY, 0.0f);
//    glRotatef(spaceshipROT, 0.0f, 0.0f, 1.0f);
//    glTranslatef(-(spaceshipXmovement), -(spaceshipYmovement), 0.0f);
    
    // new
        glTranslatef(spaceshipX+spaceshipXmovement-22, spaceshipY+spaceshipYmovement-35, 0.0f);
        glRotatef(spaceshipROT, 0.0f, 0.0f, 1.0f);
        glTranslatef(-(spaceshipXmovement+22), -(spaceshipYmovement+35), 0.0f);

    
    //front shooter
        glPointSize(7.0);
        glBegin(GL_POINTS);
        glColor3f(1,1,1);//111
        glVertex2f(spaceshipXmovement+23, spaceshipYmovement+75);
        glEnd();

        //middle body
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex2f(spaceshipXmovement+10, spaceshipYmovement+60);
        glVertex2f(spaceshipXmovement+35, spaceshipYmovement+60);
        glVertex2f(spaceshipXmovement+35, spaceshipYmovement+10);
        glVertex2f(spaceshipXmovement+10, spaceshipYmovement+10);
        glEnd();

        // Baby blue color
        glPointSize(13.0);
        glBegin(GL_POINTS);//design on middle
        glColor3f(0.6f, 0.8f, 1.0f);
        glVertex2f(spaceshipXmovement+23, spaceshipYmovement+45);
        glEnd();

        //right wing
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(spaceshipXmovement+35, spaceshipYmovement+60);
        glVertex2f(spaceshipXmovement+45, spaceshipYmovement+0);
        glVertex2f(spaceshipXmovement+35, spaceshipYmovement+10);
        glEnd();

        //left wing
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(spaceshipXmovement+10, spaceshipYmovement+60); // vertex top
        glVertex2f(spaceshipXmovement+10, spaceshipYmovement+10); // bottom vertex in direction of middle body
        glVertex2f(spaceshipXmovement+0, spaceshipYmovement+0);  // outter bottom vertex
        glEnd();

        //up body
        glBegin(GL_POLYGON);
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(spaceshipXmovement+10, spaceshipYmovement+60);
        glVertex2f(spaceshipXmovement+17, spaceshipYmovement+75);
        glVertex2f(spaceshipXmovement+28, spaceshipYmovement+75);
        glVertex2f(spaceshipXmovement+35, spaceshipYmovement+60);
        glEnd();
        
        // fire
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.5f, 0.0f);  // orange color
        glVertex3f(spaceshipXmovement+22.0f, spaceshipYmovement+8.0f, 0.0f);
        glVertex3f(spaceshipXmovement+15.0f, spaceshipYmovement+0.0f, 0.0f);
        glVertex3f(spaceshipXmovement+29.0f, spaceshipYmovement+0.0f, 0.0f);
        glEnd();
  
   
   
    glPopMatrix();
}

// Handle Spaceship Movemenet
void specialKeys(int key, int x, int y) {
    
    switch (key) {
            case GLUT_KEY_UP:
                upPressed = true;
                break;
            case GLUT_KEY_DOWN:
                downPressed = true;
                break;
            case GLUT_KEY_LEFT:
                leftPressed = true;
                break;
            case GLUT_KEY_RIGHT:
                rightPressed = true;
                break;
        }
    
}
void specialKeysUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            upPressed = false;
            break;
        case GLUT_KEY_DOWN:
            downPressed = false;
            break;
        case GLUT_KEY_LEFT:
            leftPressed = false;
            break;
        case GLUT_KEY_RIGHT:
            rightPressed = false;
            break;
    }
    glutPostRedisplay();
}

void idle() {
    if(gameState==GAME_PLAYING){
        if (upPressed && (spaceshipY + spaceshipYmovement )<575) {
            spaceshipYmovement += movementSpeed;
            
        } else if( upPressed &&(spaceshipY + spaceshipYmovement )> 575){
            lives --;
            spaceshipY -= movementSpeed;
            
        }
        else if (downPressed && (spaceshipY +spaceshipYmovement )>99) {
            spaceshipYmovement -= movementSpeed;
            
        }else if(downPressed &&(spaceshipY + spaceshipYmovement )<99){
            lives --;
            spaceshipY += movementSpeed;
        }
        else if (leftPressed && (spaceshipX +spaceshipXmovement )>89) {
            spaceshipXmovement -= movementSpeed;
        }else if (leftPressed && (spaceshipX +spaceshipXmovement )<89) {
            lives --;
            spaceshipX += movementSpeed;
        }
        else if (rightPressed  && (spaceshipX +spaceshipXmovement )<565) {
            spaceshipXmovement += movementSpeed;
        }else if (rightPressed  && (spaceshipX +spaceshipXmovement )> 565) {
            lives--;
            spaceshipX -= movementSpeed;
        }
        if( timeAnim%2==0){
            anim+=1 ;
        }else{
            anim-=1;
        }
        if (timeAnim % 5 == 0) {
                earthAnim += 45.0f;  // Increment the rotation angle
                if (earthAnim >= 360.0f) {
                    earthAnim = 0.0f;  // Reset the angle after a full rotation
                }
            }
        timeAnim++;
        checkCollision();
        glutPostRedisplay();
    }
}

void adjustSpaceshipDirection(){
    if(upPressed){  // up =2
        if(spaceshipDirection==1){
            spaceshipROT+= 90;
        } else if(spaceshipDirection==3){
            spaceshipROT+= 270;
        } else if(spaceshipDirection==4){
            spaceshipROT+= 180;
        }
        spaceshipDirection=2;
    }
    else if(downPressed){
        if(spaceshipDirection==1){
            spaceshipROT+= 270;
        } else if(spaceshipDirection==3){
            spaceshipROT+= 90;
        } else if(spaceshipDirection==2){
            spaceshipROT+= 180;
        }
        spaceshipDirection=4;
    }
     else if(rightPressed){
        
        if(spaceshipDirection==3){     // face of spaceship was left
            spaceshipROT+= -180;
        } else if(spaceshipDirection==2){
            spaceshipROT+= 270;
        } else if(spaceshipDirection==4){
            spaceshipROT+= 90;
        }
        spaceshipDirection=1;
    }
    else if(leftPressed){
        if(spaceshipDirection==1){   // face of spaceship was right
            spaceshipROT+= 180;
        } else if(spaceshipDirection==2){
            spaceshipROT+= 90;
        } else if(spaceshipDirection==4){
            spaceshipROT+= 270;
        }
        spaceshipDirection=3;
    }
}

// Collisions


bool checkCollisionWithAsteriod(int index) {
    float dx1 = spaceshipX + spaceshipXmovement-22 - position[index][0];
    float dy1 = spaceshipY + spaceshipYmovement-43.5 - position[index][1];
    float distance1 = sqrt(dx1 * dx1 + dy1 * dy1);
    return (distance1 < 30.0f); // 30 is size of my asteriod, it is scaled
   
}
bool checkCollisionWithStar(int index) {
    float dx1 = spaceshipX + spaceshipXmovement-22 - position[index][0];
    float dy1 = spaceshipY + spaceshipYmovement-43.5 - position[index][1];
    float distance1 = sqrt(dx1 * dx1 + dy1 * dy1);
    return (distance1 < 40.0f); // 75 is size of my star, it is scaled
   
}
bool checkCollisionWithScorePowerUP(int index) {
    float dx1 = spaceshipX + spaceshipXmovement-22 - position[index][0];
    float dy1 = spaceshipY + spaceshipYmovement-43.5 - position[index][1];
    float distance1 = sqrt(dx1 * dx1 + dy1 * dy1);
    return (distance1 < 30.0f); // 25 is size of my star, it is scaled
   
}
bool checkCollisionWithShieldePowerUP(int index) {
    float dx1 = spaceshipX + spaceshipXmovement-22 - position[index][0];
    float dy1 = spaceshipY + spaceshipYmovement-43.5 - position[index][1];
    float distance1 = sqrt(dx1 * dx1 + dy1 * dy1);
    return (distance1 < 40.0f); // 25 is size of my star, it is scaled
}
bool checkCollisionWithGoal(){
    float dx1 = spaceshipX + spaceshipXmovement-22 - 50;
    float dy1 = spaceshipY + spaceshipYmovement-43.5 - 50;
    float distance1 = sqrt(dx1 * dx1 + dy1 * dy1);
    return (distance1 < 20.0f);
}
bool checkCollisionWithObject() {
    
    // Check collision with asteroids
    for (int i = 10; i < 15; i++) {
        if (checkCollisionWithAsteriod(i)) {
            return true;
           
        }
    }
    
    // Check collision with stars
    for (int i = 1; i <= numStars; i++) {
        if (checkCollisionWithStar(i)) {
            return true;
          
        }
    }
    
    // Check collision with shield power-ups
    for (int i = 6; i < 8; i++) {
        if (checkCollisionWithShieldePowerUP(i)) {
            return true;
            
        }
    }
    
    // Check collision with score power-ups
    for (int i = 8; i < 10; i++) {
        if (checkCollisionWithScorePowerUP(i)) {
            return true;
          
        }
    }
    // Check collision with goal
        if (checkCollisionWithGoal()) {
            return true;
         
        }
    return false;
   
}

void checkCollision() {
    
    // Check collision with asteroids
    for (int i = 10; i < 15; i++) {
        if (checkCollisionWithAsteriod(i)) {
            handleAsteroidCollision(i);
            break;
        }
    }
    
    // Check collision with stars
    for (int i = 1; i <= numStars; i++) {
        if (checkCollisionWithStar(i)) {
            handleStarCollision(i);
            break;
        }
    }
    
    // Check collision with shield power-ups
    for (int i = 6; i < 8; i++) {
        if (checkCollisionWithShieldePowerUP(i)) {
            handleShieldCollision(i);
            break;
        }
    }
    
    // Check collision with score power-ups
    for (int i = 8; i < 10; i++) {
        if (checkCollisionWithScorePowerUP(i)) {
            handleScoreCollision(i);
            break;
        }
    }
    // Check collision with goal
        if (checkCollisionWithGoal()) {
           // playSound(4);
            collideWithAlien=true;
         
        }
}

void  handleStarCollision(int i){
   // playSound(2);
    soundTime = std::chrono::high_resolution_clock::now();
    playCollactableSound();
    
    if(doubleScoreON){
        playerScore+=20;
    }else{
        playerScore+=10;
    }
    position[i][0]=-1000;
    position[i][1]=-1000;
}
void  handleScoreCollision(int i){
    scoreCollisionTime = std::chrono::high_resolution_clock::now();
    position[i][0]=-1000;
    position[i][1]=-1000;
    doubleScoreON=true;
}
void  handleAsteroidCollision(int i){
    // playSound(3);
    soundTime = std::chrono::high_resolution_clock::now();
    playObstacleSound();
    if(!shieldON){
        lives--;
    }
    if( upPressed){
        spaceshipY -= movementSpeed;
    }else if(downPressed){
        spaceshipY += movementSpeed;
    }else if (leftPressed) {
        spaceshipX += movementSpeed;
    }else if (rightPressed ) {
        spaceshipX -= movementSpeed;
    }
    
    
}
void handleShieldCollision(int i){
    shieldCollisionTime= std::chrono::high_resolution_clock::now();
    position[i][0]=-1000;
    position[i][1]=-1000;
    shieldON=true;
}


int checkEmptyPosition(float x, float y,int n){
    for (int i = 0; i < 14; i++) {
            if ((position[i][0] + n >= x && position[i][0] - n <= x) && (position[i][1] + n >= y && position[i][1] - n <= y)) {
                return i;
            }
        }
        return 100;
}



