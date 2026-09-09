// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Piya Verma and Nila Arunkumar
// Last Modified: January 12, 2026
#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
// --- Constants ---
#define FPS 15 
#define GROUND_Y 159
#define DINO_X 10
#define DINO_GROUND_Y 154
#define JUMP_VELOCITY -12
#define GRAVITY 2
// --- Global Variables ---
uint32_t FrameCounter = 0;
int32_t GroundX = 0;
int32_t DinoVelY = 0;
uint8_t IsJumping = 0;
uint8_t IsCrouching = 0;
uint32_t Score = 0;
uint32_t HighScore = 0; // NEW: Tracks the highest score across games
uint32_t LastScore = 0xFFFFFFFF; 
uint8_t Language = 0; // 0 = English, 1 = Spanish
// --- Potentiometer Variables ---
volatile uint32_t PotValue = 0;
int32_t GameSpeed = 5; 
typedef struct {
 int32_t x, y;
 int32_t oldX, oldY;
 int32_t w, h;
 int32_t oldW, oldH;
 const uint16_t *img;
} Sprite_t;
Sprite_t Dino;
Sprite_t Bird;
Sprite_t Cacti;
Sprite_t Cloud;
Sprite_t Cloud2;
Sprite_t Cloud3;
Sprite_t Cacti2;
Sprite_t Cacti3;
int32_t ActiveObstacle = 0;
// Function Prototypes
void Init_All(void);
void Draw(void);
void GameOver(void);
void IntroScreen(void); 
uint8_t CheckCollision(void);
void DrawScore(void);
// External ADC functions from ADC1.c
extern void ADCinit(void);
extern uint32_t ADCin(void);
uint32_t M;
uint32_t Random32(void){
 M = 1664525*M+1013904223;
 return M;
}
uint32_t Random(uint32_t n){
 return (Random32()>>16)%n;
}
// --- Periodic TimerG12 ISR for ADC ---
void TIMG12_IRQHandler(void){
 if((TIMG12->CPU_INT.IIDX) == 1){ 
 PotValue = ADCin(); 
 }
}
// --- Collision Detection ---
uint8_t CheckCollision(void) {
 int32_t dinoLeft = Dino.x + 2; 
 int32_t dinoRight = Dino.x + Dino.w - 2;
 int32_t dinoTop = Dino.y - Dino.h + 2;
 int32_t dinoBottom = Dino.y;
 int32_t obsX, obsY, obsW, obsH;
 if(ActiveObstacle == 0) { obsX = Cacti.x; obsY = Cacti.y; obsW = 
Cacti.w; obsH = Cacti.h; }
 else if(ActiveObstacle == 1) { obsX = Cacti2.x; obsY = Cacti2.y; obsW = 
Cacti2.w; obsH = Cacti2.h; }
 else if(ActiveObstacle == 2) { obsX = Cacti3.x; obsY = Cacti3.y; obsW = 
Cacti3.w; obsH = Cacti3.h; }
 else { obsX = Bird.x; obsY = Bird.y; obsW = Bird.w;
obsH = Bird.h; }
 int32_t obsLeft = obsX + 1;
 int32_t obsRight = obsX + obsW - 1;
 int32_t obsTop = obsY - obsH + 1;
 int32_t obsBottom = obsY;
 if(dinoRight >= obsLeft &&
 dinoLeft <= obsRight &&
 dinoBottom >= obsTop &&
 dinoTop <= obsBottom) {
 return 1;
 }
 return 0;
}
// --- Language Selection Screen ---
void IntroScreen(void) {
 ST7735_FillScreen(ST7735_WHITE);
 ST7735_SetCursor(1, 2);
 ST7735_OutString("SELECT LANGUAGE:");
 ST7735_SetCursor(1, 3);
 ST7735_OutString("SELECCIONAR IDIOMA");
 ST7735_SetCursor(1, 6);
 ST7735_OutString("JUMP/SALTA = ENG");
 ST7735_SetCursor(1, 8);
 ST7735_OutString("DUCK/ABAJO = ESP");
 while(1) {
 uint32_t switches = Switch_In();
 uint8_t jump = (switches & 0x01);
 uint8_t crouch = (switches & 0x02) >> 1;
 if(jump) {
 Language = 0; // Set to English
 break;
 }
 if(crouch) {
 Language = 1; // Set to Spanish
 break;
 }
 Clock_Delay1ms(10);
 }
 // Wait for the user to release the button
 while(Switch_In() & 0x03);
 Clock_Delay1ms(50); 
 ST7735_FillScreen(ST7735_WHITE);
}
// --- Draw Score Bar ---
void DrawScore(void) {
 if(Score == LastScore) return; 
 LastScore = Score;
 // --- NEW: Live update High Score if they beat it while playing! ---
 if (Score > HighScore) {
 HighScore = Score;
 }
 // Erase old score area (full top row)
 ST7735_FillRect(0, 0, 128, 10, ST7735_WHITE);
 // Draw current score on the left
 ST7735_SetCursor(1, 0); 
 if(Language == 0) ST7735_OutString("S:");
 else ST7735_OutString("P:");
 ST7735_OutUDec(Score);
 // --- NEW: Draw high score on the right ---
 ST7735_SetCursor(10, 0); 
 if(Language == 0) ST7735_OutString("HI:");
 else ST7735_OutString("MAX:");
 ST7735_OutUDec(HighScore);
}
void GameOver(void) {
 Sound_Killed();
 ST7735_FillScreen(ST7735_WHITE);
 // Final safety check to lock in high score
 if (Score > HighScore) {
 HighScore = Score;
 }
 // Show English OR Spanish text based on selection
 if(Language == 0) {
 ST7735_SetCursor(3, 3);
 ST7735_OutString("GAME OVER");
 
 ST7735_SetCursor(3, 5);
 ST7735_OutString("SCORE: ");
 ST7735_OutUDec(Score);
 
 // --- NEW: Display High Score on Game Over ---
 ST7735_SetCursor(3, 6);
 ST7735_OutString("HIGH: ");
 ST7735_OutUDec(HighScore);
 
 ST7735_SetCursor(2, 8);
 ST7735_OutString("PRESS JUMP");
 ST7735_SetCursor(3, 9);
 ST7735_OutString("TO RESTART");
 } else {
 ST7735_SetCursor(1, 3);
 ST7735_OutString("FIN DEL JUEGO");
 
 ST7735_SetCursor(3, 5);
 ST7735_OutString("PUNTOS: ");
 ST7735_OutUDec(Score);
 
 // --- NEW: Display High Score on Game Over ---
 ST7735_SetCursor(3, 6);
 ST7735_OutString("MAXIMO: ");
 ST7735_OutUDec(HighScore);
 
 ST7735_SetCursor(2, 8);
 ST7735_OutString("SALTA PARA");
 ST7735_SetCursor(3, 9);
 ST7735_OutString("REINICIAR");
 }
 // Debounced Wait Logic 
 while(Switch_In() & 0x01); // Wait for release
 Clock_Delay1ms(50);
 while(!(Switch_In() & 0x01)); // Wait for fresh press
 Clock_Delay1ms(50);
 while(Switch_In() & 0x01); // Wait for release BEFORE going back to Intro 
Screen
 Clock_Delay1ms(50);
 // Reset all game state (NOTICE: We do NOT reset HighScore here!)
 Score = 0;
 LastScore = 0xFFFFFFFF;
 FrameCounter = 0;
 GroundX = 0;
 DinoVelY = 0;
 IsJumping = 0;
 IsCrouching = 0;
 Dino.x = DINO_X; Dino.y = DINO_GROUND_Y; Dino.w = 20; Dino.h = 21;
 Dino.oldX = Dino.x; Dino.oldY = Dino.y; Dino.oldW = Dino.w; Dino.oldH = Dino.h;
 Cacti.x = -50; Cacti2.x = -50; Cacti3.x = -50; Bird.x = -50;
 ActiveObstacle = (int32_t)Random(4);
 if(ActiveObstacle == 0) Cacti.x = 128;
 else if(ActiveObstacle == 1) Cacti2.x = 128;
 else if(ActiveObstacle == 2) Cacti3.x = 128;
 else { Bird.x = 128; Bird.y = 148; }
 // Go back to the Intro Screen so they can pick a language for the next round
 IntroScreen();
}
int main(void) {
 Init_All();
 
 IntroScreen();
 while(1) {
 FrameCounter++;
 // Map the 0-4095 ADC value to a game speed of ~3 to 10 pixels per frame
 GameSpeed = 3 + ((PotValue * 7) / 4096);
 // --- READ SWITCHES ---
 uint32_t switches = Switch_In();
 uint8_t jumpPressed = (switches & 0x01);
 uint8_t crouchPressed = (switches & 0x02) >> 1;
 if(jumpPressed || crouchPressed) {
 Sound_BeepOn();
 } else {
 Sound_BeepOff();
 }
 // --- SAVE OLD STATE ---
 Dino.oldX = Dino.x; Dino.oldY = Dino.y;
 Dino.oldW = Dino.w; Dino.oldH = Dino.h;
 Cloud.oldX = Cloud.x; Cloud.oldY = Cloud.y;
 Cloud2.oldX = Cloud2.x; Cloud2.oldY = Cloud2.y;
 Cloud3.oldX = Cloud3.x; Cloud3.oldY = Cloud3.y;
 Cacti.oldX = Cacti.x;
 Cacti2.oldX = Cacti2.x;
 Cacti3.oldX = Cacti3.x;
 Bird.oldX = Bird.x; Bird.oldY = Bird.y;
 // --- JUMP LOGIC ---
 if(jumpPressed && !IsJumping && !IsCrouching) {
 IsJumping = 1;
 DinoVelY = JUMP_VELOCITY;
 }
 if(IsJumping) {
 Dino.y += DinoVelY;
 DinoVelY += GRAVITY;
 if(Dino.y >= DINO_GROUND_Y) {
 Dino.y = DINO_GROUND_Y;
 IsJumping = 0;
 DinoVelY = 0;
 }
 }
 // --- CROUCH LOGIC ---
 IsCrouching = (crouchPressed && !IsJumping) ? 1 : 0;
 // --- UPDATE DINO SIZE ---
 if(IsCrouching) {
 Dino.w = 20; Dino.h = 14;
 } else {
 Dino.w = 20; Dino.h = 21;
 }
 // --- COLLISION CHECK ---
 if(CheckCollision()) {
 GameOver();
 continue; 
 }
 // --- UPDATE WORLD (USING SLIDE POT SPEED) ---
 GroundX -= GameSpeed;
 
 // Parallax clouds: moving slower than the ground
 int32_t cloudSpeed = (GameSpeed / 3) > 0 ? (GameSpeed / 3) : 1;
 Cloud.x -= cloudSpeed; 
 Cloud2.x -= cloudSpeed; 
 Cloud3.x -= cloudSpeed;
 if(ActiveObstacle == 0) Cacti.x -= GameSpeed;
 else if(ActiveObstacle == 1) Cacti2.x -= GameSpeed;
 else if(ActiveObstacle == 2) Cacti3.x -= GameSpeed;
 else if(ActiveObstacle == 3) Bird.x -= (GameSpeed + 1); 
 if(GroundX <= -128) GroundX = 0;
 // --- OBSTACLE CYCLING + SCORING ---
 int32_t currentX = 0;
 if(ActiveObstacle == 0) currentX = Cacti.x;
 else if(ActiveObstacle == 1) currentX = Cacti2.x;
 else if(ActiveObstacle == 2) currentX = Cacti3.x;
 else if(ActiveObstacle == 3) currentX = Bird.x;
 if(currentX < -40) {
 Score++; 
 uint32_t nextObstacle;
 do {
 nextObstacle = Random(4);
 } while(nextObstacle == (uint32_t)ActiveObstacle);
 ActiveObstacle = (int32_t)nextObstacle;
 Cacti.x = -50; Cacti2.x = -50; Cacti3.x = -50; Bird.x = -50;
 int32_t spawnX = 128 + (int32_t)Random(40);
 if(ActiveObstacle == 0) Cacti.x = spawnX;
 else if(ActiveObstacle == 1) Cacti2.x = spawnX;
 else if(ActiveObstacle == 2) Cacti3.x = spawnX;
 else {
 Bird.x = spawnX;
 Bird.y = 140;
 }
 }
 // --- CLOUD WRAP ---
 if(Cloud.x < -20) { Cloud.x = 128 + (int32_t)Random(30); Cloud.y = 60 + 
(int32_t)Random(20); }
 if(Cloud2.x < -20) { Cloud2.x = 128 + (int32_t)Random(30); Cloud2.y = 65 + 
(int32_t)Random(20); }
 if(Cloud3.x < -20) { Cloud3.x = 128 + (int32_t)Random(30); Cloud3.y = 70 + 
(int32_t)Random(20); }
 Draw();
 Clock_Delay1ms(1000/FPS);
 }
}
void Init_All(void) {
 Clock_Init80MHz(0);
 LaunchPad_Init();
 ST7735_InitR(INITR_REDTAB);
 Switch_Init();
 Sound_Init();
 
 ADCinit(); 
 TimerG12_IntArm(1600000, 2); 
 __enable_irq();
 M = SysTick->VAL ^ 0xDEADBEEF;
 Dino.x = DINO_X; Dino.y = DINO_GROUND_Y; Dino.w = 20; Dino.h = 21;
 Dino.oldX = Dino.x; Dino.oldY = Dino.y; Dino.oldW = Dino.w; Dino.oldH = Dino.h;
 Cacti.w = 30; Cacti.h = 9; Cacti.x = -50; Cacti.y = 154; Cacti.img = 
cacti;
 Cacti2.w = 5; Cacti2.h = 10; Cacti2.x = -50; Cacti2.y = 154; Cacti2.img = 
cacti1;
 Cacti3.w = 20; Cacti3.h = 20; Cacti3.x = -50; Cacti3.y = 154; Cacti3.img = 
cactithree;
 Bird.w = 20; Bird.h = 14; Bird.x = -50; Bird.img = bird;
 ActiveObstacle = (int32_t)Random(4);
 if(ActiveObstacle == 0) Cacti.x = 128;
 else if(ActiveObstacle == 1) Cacti2.x = 128;
 else if(ActiveObstacle == 2) Cacti3.x = 128;
 else { Bird.x = 128; Bird.y = 140; }
 Cloud.x = 40; Cloud.y = 70; Cloud.w = 20; Cloud.h = 6; Cloud.img = cloud;
 Cloud2.x = 110; Cloud2.y = 85; Cloud2.w = 20; Cloud2.h = 6; Cloud2.img = cloud;
 Cloud3.x = 180; Cloud3.y = 65; Cloud3.w = 20; Cloud3.h = 6; Cloud3.img = cloud;
}
void Draw(void) {
 // --- 1. ERASE OLD DINO AND REDRAW NEW ONE BACK TO BACK ---
 // Erase the full old bounding box
 ST7735_FillRect(Dino.oldX - 1, Dino.oldY - Dino.oldH - 1, 
 Dino.oldW + 2, Dino.oldH + 2, ST7735_WHITE);
 // Pick new animation frame BEFORE drawing
 if(IsCrouching) {
 if((FrameCounter / 4) % 2 == 0) { Dino.img = duck1; Dino.h = 9; }
 else { Dino.img = duck2; Dino.h = 14; }
 } else if(IsJumping) {
 Dino.img = run1;
 } else {
 if((FrameCounter / 4) % 2 == 0) Dino.img = run2;
 else Dino.img = run3;
 }
 // Redraw dino IMMEDIATELY after erase, before anything else
 ST7735_DrawBitmap(Dino.x, Dino.y, Dino.img, Dino.w, Dino.h);
 // --- 2. DRAW BACKGROUND ---
 ST7735_DrawBitmap(GroundX, GROUND_Y, ground, 128, 41);
 ST7735_DrawBitmap(GroundX + 128, GROUND_Y, ground, 128, 41);
 // --- 3. DRAW OBSTACLES ---
 if(ActiveObstacle == 0) ST7735_DrawBitmap(Cacti.x, Cacti.y, Cacti.img, 
Cacti.w, Cacti.h);
 else if(ActiveObstacle == 1) ST7735_DrawBitmap(Cacti2.x, Cacti2.y, Cacti2.img, 
Cacti2.w, Cacti2.h);
 else if(ActiveObstacle == 2) ST7735_DrawBitmap(Cacti3.x, Cacti3.y, Cacti3.img, 
Cacti3.w, Cacti3.h);
 else ST7735_DrawBitmap(Bird.x, Bird.y, Bird.img, 
Bird.w, Bird.h);
 // Redraw dino on top of ground (ground may have partially overwritten it)
 ST7735_DrawBitmap(Dino.x, Dino.y, Dino.img, Dino.w, Dino.h);
 // --- 4. DRAW SCORE ---
 DrawScore();
 // --- 5. CLOUDS ---
 ST7735_FillRect(Cloud.oldX - 1, Cloud.oldY - Cloud.h - 1, Cloud.w + 2, Cloud.h 
+ 2, ST7735_WHITE);
 ST7735_DrawBitmap(Cloud.x, Cloud.y, Cloud.img, Cloud.w, Cloud.h);
 ST7735_FillRect(Cloud2.oldX - 1, Cloud2.oldY - Cloud2.h - 1, Cloud2.w + 2, 
Cloud2.h + 2, ST7735_WHITE);
 ST7735_DrawBitmap(Cloud2.x, Cloud2.y, Cloud2.img, Cloud2.w, Cloud2.h);
 ST7735_FillRect(Cloud3.oldX - 1, Cloud3.oldY - Cloud3.h - 1, Cloud3.w + 2, 
Cloud3.h + 2, ST7735_WHITE);
 ST7735_DrawBitmap(Cloud3.x, Cloud3.y, Cloud3.img, Cloud3.w, Cloud3.h);
}
