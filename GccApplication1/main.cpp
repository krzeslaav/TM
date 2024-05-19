#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>


#include "lcd.h"
#include "lcd.cpp"


#define F_CPU 16000000
#define true 1
#define false 0
LCD lcd;

#define dimX 84
#define  dimY 48

uint8_t CURRENT_MIN = 0;
uint8_t CURRENT_SEC = 0;

void wait(){
	volatile uint16_t i,j;
	for (i = 0; i< 1000; i++){
		for (j = 0; j<1000; j++);;
	}
}

void toggle_scr(){
	lcd.begin();
	lcd.setPower(1);
	lcd.writeString("STARTING!",1);
	lcd.render();
}

void toggle_frame(){
	uint8_t i;
//	for (i = 0; i<=dimX; i++){
//		lcd.setPixel(i, 0, 1);
//		lcd.setPixel(i, dimY-1, 1);			
//	}
	for (i = 0; i<=dimY; i++){
		lcd.setPixel(0, i, 1);
		lcd.setPixel(dimY, i, 1);			
		lcd.setPixel(i, 0, 1);
		lcd.setPixel(i, dimY-1, 1);			
	}
	lcd.render();
}

void toggle_scoreboard(){
	// game screen - dimY x dimY
	// score screen - dimY + 1
	
		// set time
	lcd.setCursor(dimY+2, 0);
	lcd.writeString("TIME", 1);
	lcd.setCursor(dimY+5, 10);
	lcd.writeString("XX:XX", 1);
	
		// set score
	lcd.setCursor(dimY+2, dimY/2 + 5);
	lcd.writeString("SCORE", 1);
	lcd.setCursor(dimY+5, dimY/2 + 15);
	lcd.writeString("XXXX", 1);
	
	lcd.render();
}

void clear_scoreborard(){
	//lcd.setCursor(dimY+3, dimY/2 + 15);
	unsigned int i,j;
	for (i = dimY+5; i< dimX; i++){
		for (j = dimY/2 + 15; j<dimY;j++){
			lcd.setPixel(i,j,0);
		}
	}
	//lcd.setCursor(dimY+3, dimY/2 + 15);
	lcd.render();
}

void change_scoreboard(uint16_t score){
	char str[5];
	 snprintf(str, sizeof(str), "%u", score);
	
	clear_scoreborard();		
	
	lcd.setCursor(dimY+5, dimY/2 + 15);
	
	 if(score <=9 ){
			lcd.writeString( "000", 1);
			lcd.writeString( str, 1);
	}if(score <=99 && score > 9){
			lcd.writeString( "00", 1);
			lcd.writeString( str, 1);
	}if(score <=999 && score > 99){
			lcd.writeString( "0", 1);
			lcd.writeString( str, 1);
	}if(score > 999){
			lcd.writeString( "", 1);
			lcd.writeString( str, 1);
	}
	

	lcd.render();
	
}

void clear_time(){
	unsigned int i,j;
	//	lcd.setCursor(dimY+5, 10);

	for (i = dimY+5; i< dimX; i++){
		for (j = 10 ; j<dimY/2 ;j++){
			lcd.setPixel(i,j,0);
		}
	}
	
	lcd.render();
}

void set_time(){
	
	lcd.setCursor(dimY+5, 10);
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
	lcd.setCursor(dimY+5, 10);
	
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
	
 	toggle_scr();
	lcd.clear();
	toggle_frame();
	toggle_scoreboard();
	clear_time();
	set_time();
	
}

int main(void)
{
	 uint16_t score = 1519;
	enable_lcd();

	change_scoreboard(score	);




	lcd.render();


	while(1) {

		for (int i = 0; i<120; i++){
			increment_time();
			wait();
		}
	}
}
