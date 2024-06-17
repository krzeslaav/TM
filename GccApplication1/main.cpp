#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define F_CPU 8000000

#include "lcd.h"
#include "lcd.cpp"

#define true 1
#define false 0
#define dimX 84
#define dimY 46

using namespace std;

LCD lcd;

char LAST_DIRECTION = 'n';
uint8_t CLOCK_STOP = 0;
uint8_t INNER_TIMER_GAME = 0;
uint8_t INNER_TIMER_SCORE = 0;
uint8_t CURRENT_MIN = 0;
uint8_t CURRENT_SEC = 0;


int	snakeHeadX = 0;
int	snakeHeadY = 0;
int	fruitX = 0;
int	fruitY = 0;
int	snakeTailLength = 0;
uint8_t	savedSeconds = 0;
uint8_t	savedMinutes = 0;
int snakeTailX[300];
int snakeTailY[300];
int score = 0;
uint16_t game_over = 0;

void updateSnake();
void printSnake();

void wait(){
	volatile uint16_t i,j;
	for (i = 0; i< 1000; i++){
		for (j = 0; j<1000; j++);;
	}
}

void print_start(){
	lcd.clear();
	lcd.render();
	
	lcd.setCursor( 18,8);
	lcd.writeString("Press any",1); //letters : 9
	lcd.setCursor( 25, 23);
	lcd.writeString("button",1);
	lcd.setCursor( 12, 38);
	lcd.writeString("to start :D",1); //letters : 11
	lcd.render();
}

void print_over(){
	char scoreString[5];
	snprintf(scoreString, sizeof(scoreString), "%u", score);
	
	char secondsString[5];
	snprintf(secondsString, sizeof(secondsString), "%u", savedSeconds);
	
	char minutesString[5];
	snprintf(minutesString, sizeof(minutesString), "%u", savedMinutes);
	
	lcd.clear();
	lcd.render();
	
	lcd.setCursor( 20, 0);
	lcd.writeString("Game over ",1);
	lcd.setCursor( 10, 10);
	lcd.writeString("score   ",1);
	lcd.writeString(scoreString, 1);
	lcd.setCursor( 10, 18);
	lcd.writeString("time   ",1);
	
	if (savedMinutes <= 9)
	{
		lcd.writeString("0",1);
		lcd.writeString(minutesString, 1);
	}
	else
	{
		lcd.writeString(minutesString, 1);
	}
	
	lcd.writeString(":", 1);
	
	if (savedSeconds <= 9)
	{
		lcd.writeString("0",1);
		lcd.writeString(secondsString, 1);
	}
	else
	{
		lcd.writeString(secondsString, 1);
	}
	
	lcd.setCursor( 18, 30);
	lcd.writeString("Press any",1);
	lcd.setCursor( 0,40);
	lcd.writeString("button to exit",1);
	lcd.render();
	
}


char detect_button(){
	char dir = 'a';
	switch(PINB){
		case 0b10001110:
		dir = 'd';
		break;
		case 0b10001101:
		dir = 'a';
		break;
		case 0b10001011:
		dir = 's';
		break;
		case 0b10000111:
		dir = 'w';
		break;
		default:
		dir = LAST_DIRECTION;
		
		break;
	}
	LAST_DIRECTION = dir;
	return dir;
}

void main_clock(){
	// MAIN TIMER, SET FOR t = 0.016 [s], f = 61 [Hz]
	sei();						// ENABLE INTERRUPTS
	TCCR0A = (1 << WGM01);		// SET CTC BIT - CLEAR ON COMPARE, TIMER CONTROL REGISTER A
	OCR0A = 255;				// VALUE TO COMPARE WITH
	TIMSK0 = (1 << OCIE0A);		// SET MASK TO ENABLE INTERRUPTS FOR OCIE0A
						
	TCCR0B = (1 << CS02) | (1 << CS00);
}

// FRAME, SCOREBOARD, TIME 
void toggle_scr(){				// TURN ON SCREEN 
	lcd.begin();
	lcd.setPower(1);
	lcd.writeString("STARTING!",1);
	lcd.render();
}

void toggle_frame(){			// PRINT SNAKE FRAME
	uint8_t i;
	
	//	<-----0 : dimX -1------->		
	//	_____________________  
	//	|					|  
	//	|					|
	//	|					|
	//	|					|   8 : dimY-1
	//	|					|
	//	|					|
	//  |___________________|

	for (i = 1; i<=dimY-8; i++){
		lcd.setPixel(0, i+7, 1);		// set left line 
		lcd.setPixel(dimX-1, i+7, 1);		// set right line 
	} 
	for (i = 0; i<=dimX;i++){
		lcd.setPixel(i, 8 , 1);			// set top line
		lcd.setPixel(i, dimY-1, 1);		// set bottom line
	}
	lcd.render();
}

void toggle_scoreboard(){		// PRINT SCOREBOARD 
	
	/*
	012345....																	    DX-1
	__________________________________________________________________________________
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|--T-- --\0--  --X-- --X-- --:-- --X-- --X-- --\0-- --S-- --0-- --0-- --0-- --0-- |
	|_________________________________________________________________________________|
							One letter size : 5px X 5px 
	*/

	lcd.setCursor(0, 0);
	lcd.writeString("T XX:XX   XXXX", 1);
	lcd.render();
}

void clear_scoreborard(){
	unsigned int i,j;
	for (i = dimX - 25; i< dimX-1; i++){
		for (j = 0 ; j<8;j++){
			lcd.setPixel(i,j,0);
		}
	}
	lcd.render();
}

void change_scoreboard(uint16_t scoore){
	char str[5];
	snprintf(str, sizeof(str), "%u", scoore);
	
	clear_scoreborard();		
	
	lcd.setCursor(dimX - 25,0);
	
	 if(scoore <=9 ){
			lcd.writeString( "000", 1);
			lcd.writeString( str, 1);
	}if(scoore <=99 && scoore > 9){
			lcd.writeString( "00", 1);
			lcd.writeString( str, 1);
	}if(scoore <=999 && scoore > 99){
			lcd.writeString( "0", 1);
			lcd.writeString( str, 1);
	}if(scoore > 999){
			lcd.writeString( "", 1);
			lcd.writeString( str, 1);
	}
	lcd.render();

}

void clear_time(){
	unsigned int i,j;
	lcd.setCursor(10, 0);

	for (i = 10; i< 60; i++){
		for (j = 0 ; j<8 ;j++){
			lcd.setPixel(i,j,0);
		}
	}
	
	lcd.render();
}

void clearSnakeFrame()
{
	unsigned int i,j;
	
	for (i = 1; i< 83; i++){
		for (j = 9 ; j<45 ;j++){
			lcd.setPixel(i,j,0);
		}
	}
	
	lcd.render();
}

void set_time(){
	
	lcd.setCursor(10, 0);
	lcd.writeString("00:00", 1);
	lcd.render();
	CURRENT_SEC = 0;
	CURRENT_MIN = 0;
}

void increment_time(){
	
	CURRENT_SEC ++;
	if ( CURRENT_SEC == 60 ) {CURRENT_MIN++; CURRENT_SEC= 0;}
	
	char str_min[3];
	snprintf(str_min, sizeof(str_min), "%u",CURRENT_MIN); // changes uint8_t to char array
	
	char str_sec[3];
	snprintf(str_sec, sizeof(str_sec), "%u",CURRENT_SEC); // changes uint8_t to char array
	
	clear_time();
	lcd.setCursor(10, 0);
	
	if (CURRENT_MIN <= 9) {
		lcd.writeString("0",1);
		lcd.writeString(str_min, 1);
	}else{
		lcd.writeString(str_min, 1);
	}
	lcd.writeString(":", 1);
	
	if (CURRENT_SEC <= 9) {
		lcd.writeString("0",1);
		lcd.writeString(str_sec, 1);
	}else{
		lcd.writeString(str_sec, 1);
	}

	lcd.render();
	
}

void enable_lcd(){
	// also set the DDRB for the buttons
	
	
 	toggle_scr();
	lcd.clear();
	toggle_frame();
	toggle_scoreboard();
	clear_time();
	set_time();
	lcd.render();
	
}

ISR(TIMER0_COMPA_vect){
		
	
	
	// TO INCREMENT SCOREBOARD TIME - 30.30 INNER_TIMER NEEDED - EVERY 1 SECOND
	// MAIN GAME SPEED TIME - 	15 INNER_TIMER NEEDED - EVERY 0.5 SECOND
	//
	if (CLOCK_STOP != 1){
		INNER_TIMER_SCORE++;
		INNER_TIMER_GAME++;
	
		if(INNER_TIMER_GAME == 20){
			INNER_TIMER_GAME = 0;
			PORTB = ~PORTB;
	
		}
		if(INNER_TIMER_SCORE == 63){
			INNER_TIMER_SCORE = 0;
			increment_time();
			change_scoreboard(score);
			printSnake();
		}
	}
}


void stop_clock(bool var){ // if do == 1, CLOCK_STOP = 1, else CLOCK STOP = 0
	if (var == 1){
		CLOCK_STOP = 1;
	} 
	if (var == 0){
		CLOCK_STOP = 0;
	}
}

void updateSnake( char & direction )
{

			int prevX = snakeTailX[0];
			int prevY = snakeTailY[0];
			int prev2X, prev2Y;
			snakeTailX[0] = snakeHeadX;
			snakeTailY[0] = snakeHeadY;
			
			for (int i = 1; i < snakeTailLength; i++)
			{
				prev2X = snakeTailX[i];
				prev2Y = snakeTailY[i];
				snakeTailX[i] = prevX;
				snakeTailY[i] = prevY;
				prevX = prev2X;
				prevY = prev2Y;
			}
			
			switch (direction) {
				case 'a':
				snakeHeadX--;
				break;
				case 'd':
				snakeHeadX++;
				break;
				case 'w':
				snakeHeadY--;
				break;
				case 's':
				snakeHeadY++;
				break;
				default:
				break;
			}
			
			if (snakeHeadX > 26 || snakeHeadX < 0 || snakeHeadY > 11 || snakeHeadY < 0) //check for collisions with wall
			{
				game_over = 1;
				savedMinutes = CURRENT_MIN;
				savedSeconds = CURRENT_SEC;
			}
			
			for (int i = 0; i < snakeTailLength; i++) //check for collisions with tail
			{
				if (snakeTailX[i] == snakeHeadX && snakeTailY[i] == snakeHeadY)
				{
					game_over = 1;
					savedMinutes = CURRENT_MIN;
					savedSeconds = CURRENT_SEC;
				}
			}
			
			if (snakeHeadX == fruitX && snakeHeadY == fruitY) //check for collisions with fruit
			{
				score += 10;
				
				while(true)
				{
					fruitX = rand() % 27;
					fruitY = rand() % 12;
					bool newFruitPositionIsOk = true;
					
					for(int i = 0; i<snakeTailLength; i++)
					{
						if(fruitX == snakeTailX[i] && fruitY == snakeTailY[i])
						{
							newFruitPositionIsOk = false;
							break;
						}
					}
					
					if(newFruitPositionIsOk)
					{
						break;
					}
				}
				
				snakeTailLength++;
			}
			
		
	
}

void printSnake()
{
	clearSnakeFrame();
	
	//drawing head
	// ###
	// ###
	// ###
	
	for(int i = ((snakeHeadX*3)+1); i<((snakeHeadX*3)+4); i++)
	{
		for(int ii = ((snakeHeadY*3)+9); ii<((snakeHeadY*3)+12); ii++)
		{
			lcd.setPixel(i, ii , 1);
		}
	}
	
	//drawing fruit
	//  #
	// # #
	//  #
	
	for(int i = ((fruitX*3)+1); i<((fruitX*3)+4); i++)
	{
		for(int ii = ((fruitY*3)+9); ii<((fruitY*3)+12); ii++)
		{
			if(((i%2)==1 && (ii%2)==1) || ((i%2)==0 && (ii%2)==0))
			{
				lcd.setPixel(i, ii , 1);
			}
		}
	}
	
	//drawing tail
	// ###
	// ###
	// ###
	
	for(int j = 0; j<snakeTailLength; j++)
	{
		for(int i = ((snakeTailX[j]*3)+1); i<((snakeTailX[j]*3)+4); i++)
		{
			for(int ii = ((snakeTailY[j]*3)+9); ii<((snakeTailY[j]*3)+12); ii++)
			{
				lcd.setPixel(i, ii , 1);
			}
		}
	}
	
	lcd.render();
}

int main(void)
{
	DDRB = 0b110000;
	PINB = 0b001111;
	
	//DDRB = 0x77;
	//PORTB = 0b111111;
	
	uint16_t game_start = 0;
	toggle_scr();
	char current_direction = 'n';
	srand(2137);
	
	while(1) {
		print_start();
		wait();
		current_direction = 'n';
		LAST_DIRECTION = 'n';
		current_direction = detect_button();
		
		if (current_direction != 'n'){
			game_start = 1;
			
			snakeHeadX = rand() % 27;
			snakeHeadY = rand() % 12;
			
			while(true)
			{
				fruitX = rand() % 27;
				fruitY = rand() % 12;
				
				if(fruitX != snakeHeadX || fruitY != snakeHeadY)
				break;
			}
			
			snakeTailLength = 0;
			score = 0;
		}
		
		if( game_start == 1)
		{
			lcd.clear();
			
			enable_lcd();
			main_clock();
			
			while(true)
			{	
				// main game loop
				CLOCK_STOP = 0;

				current_direction = detect_button();
				updateSnake( current_direction );
				wait();
				
				// if snake hit itself, game_over = 1
				//game_over = 1;
				
				if (game_over == 1){

					lcd.clear();
					lcd.render();
					
					CLOCK_STOP = 1;
					game_start = 0;
					game_over = 0;
					
					while(true)
					{
						// wait for button press, to start over	
						// wait for press of a button to exit to the main screen, if so, game_start = 1
						
						print_over();
						wait();
						current_direction = 'n';
						LAST_DIRECTION = 'n';
						current_direction = detect_button();
						
						if (current_direction != 'n')
						{
							game_start = 1;
							
							snakeHeadX = rand() % 27;
							snakeHeadY = rand() % 12;
							
							while(true)
							{
								fruitX = rand() % 27;
								fruitY = rand() % 12;
								
								if(fruitX != snakeHeadX || fruitY != snakeHeadY)
									break;
							}
							
							snakeTailLength = 0;
							score = 0;
						}
						
						if(game_start == 1)
						{
							lcd.clear();
							lcd.render();
							toggle_frame();
							toggle_scoreboard();
							clear_time();
							set_time();
							lcd.render();
							wait();
							break;
						}
						
					}
				}
				
			}
		}
		
	}
}



