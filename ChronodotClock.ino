/*
  Clock.pde - Peggy 2.0 Digital Clock
  Version 1.0 - 06/13/2008
  Copyright (c) 2008 Arthur J. Dahm III.  All right reserved.
  Email: art@mindlessdiversions.com
  Web: mindlessdiversions.com/peggy2

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
	USAGE:
	 - Press the "Any" button to cycle through time display, set hours, minutes, month and day.
	 - Press the "Off/Select" button to set the  hours, minutes, month or day.

  TODO:
  - 12/24 Hour Display
  - Alarm
  - Timer
  - Set time from computer over serial port
*/

#include <Display.h>
#include <Wire.h>
#include "Chronodot.h"

Peggy2Display Display;
Chronodot RTC;
DateTime now;

float rate = 60.0;

#define TIME_X_OFFSET 0
#define TIME_Y_OFFSET 5

#define BUTTON_DEBOUNCE 20UL	// Time in mS to wait until next poll
#define BUTTON_ANY 1			// b1 "any" button
#define BUTTON_LEFT 2			// b2 "left" button
#define BUTTON_DOWN 4			// b3 "down" button
#define BUTTON_UP 8				// b4 "up" button
#define BUTTON_RIGHT 16		// b5 "right" button
#define BUTTON_OFF_SEL 32	// s2 "off/select" button
#define BUTTONS_CURRENT (PINC & B00011111) | ((PINB & 1)<<5)

// Button variables
uint32_t buttonDebounce;
uint8_t buttonPollState;
uint8_t buttonPollStatePrev;
uint8_t buttonsPressed;
uint8_t buttonsReleased;

// Display variables
uint8_t digits[15][5] =	{
												{B11100000, B10100000, B10100000, B10100000, B11100000},	// 0
												{B01000000, B11000000, B01000000, B01000000, B11100000},	// 1
												{B11100000, B00100000, B11100000, B10000000, B11100000},	// 2
												{B11100000, B00100000, B01100000, B00100000, B11100000},	// 3
												{B10100000, B10100000, B11100000, B00100000, B00100000},	// 4
												{B11100000, B10000000, B11100000, B00100000, B11100000},	// 5
												{B11100000, B10000000, B11100000, B10100000, B11100000},	// 6
												{B11100000, B00100000, B00100000, B01000000, B01000000},	// 7
												{B11100000, B10100000, B11100000, B10100000, B11100000},	// 8
												{B11100000, B10100000, B11100000, B00100000, B11100000},	// 9
												{B00000000, B01000000, B00000000, B01000000, B00000000},	// :
												{B00000000, B00100000, B01000000, B10000000, B00000000},	// /
												{B00000000, B00000000, B00000000, B00000000, B00000000},	// ' '
												{B01100000, B01100000, B00000000, B00000000, B00000000},	// A
												{B00000000, B00000000, B00000000, B01100000, B01100000},	// P
												};

#define CHAR_COLON 10
#define CHAR_SLASH 11
#define CHAR_SPACE 12
#define CHAR_AM 13
#define CHAR_PM 14

void setup()
{
	// Set up input buttons
	PORTB = B00000001;	// Pull up on ("OFF/SELECT" button)
	PORTC = B00011111;	// Pull-ups on C
	DDRB = B11111110;		// B0 is an input ("OFF/SELECT" button)
	DDRC = B11100000;		// All inputs

	buttonPollStatePrev = BUTTONS_CURRENT;
	buttonDebounce = SafeMillis();
	
	// Set up display
	Display = Peggy2Display();
	Display.SetRefreshRate(rate);

	// Set up Chronodot
	Wire.begin();
	RTC.begin();
  now = RTC.now();


	if (!RTC.isrunning()) {
		// following line sets the RTC to the date & time this sketch was compiled
		RTC.adjust(DateTime(__DATE__, __TIME__));
	}
}

void loop()
{
//	PollButtons();
	
	Clock();

  // Write time to frame buffer
  DisplayTime();

  delay(250);
}

void Clock()
{
	// get time from Chronodot
  now = RTC.now();

}


// Display functions

void DisplayTime()
{
	uint8_t temp;
	
	// am/pm indicator
	if ((now.hour() < 12) || (now.hour() == 24))
  	DisplayTimeDigit(CHAR_AM, 5);
	else
  	DisplayTimeDigit(CHAR_PM, 5);	


	// hours
	temp = now.hour();
	if (temp > 12)
		temp -= 12;	
	
	if (temp > 9)
		DisplayTimeDigit(1, 0);
	else
		DisplayTimeDigit(CHAR_SPACE, 0);

	DisplayTimeDigit(temp%10, 1);

	// minutes
	DisplayTimeDigit(now.minute()/10, 3);
	DisplayTimeDigit(now.minute()%10, 4);

	// blinking colon
//  if((mode == MODE_RUN) && (currentTime%1000 >= 500))
//		DisplayTimeDigit(CHAR_SPACE, 2);
//  else
		DisplayTimeDigit(CHAR_COLON, 2);
		
	// seconds
	DisplaySeconds();
		
	// month
	if (now.month() > 9)
		DisplayDateDigit(1, 0);
	else
		DisplayDateDigit(CHAR_SPACE, 0);

	DisplayDateDigit(now.month()%10, 1);

	// day
	if (now.day() > 9)
		DisplayDateDigit(now.day()/10, 3);
	else
		DisplayDateDigit(CHAR_SPACE, 3);

	DisplayDateDigit(now.day()%10, 4);

	// slash
	DisplayDateDigit(CHAR_SLASH, 2);
}

void DisplaySeconds()
{
	uint8_t temp;

	temp = now.second()%10;

	// Display 1s
	for (uint8_t x=0; x<10; x++)
	{
		if (temp == x)
			DisplaySecondsBar(1, x<<1, TIME_Y_OFFSET+8, 0xc0);
		else		
			DisplaySecondsBar(0, x<<1, TIME_Y_OFFSET+8, 0xc0);
	}
		
	// Display 10s
	temp = now.second()/10;
	
	for (uint8_t x=1; x<=5; x++)
	{
		if (temp == x)
			DisplaySecondsBar(1, (x-1)<<2, TIME_Y_OFFSET+6, 0x70);
		else		
			DisplaySecondsBar(0, (x-1)<<2, TIME_Y_OFFSET+6, 0x70);
	}
}

void DisplaySecondsBar(uint8_t digit, uint8_t xpos, uint8_t ypos, uint8_t bar)
{
	uint16_t widebar;
	
	widebar = uint16_t(bar)<<(8-(xpos%8));

	if (digit)
	{
		Display.framebuffer[(ypos<<2)+(xpos>>3)] |= (widebar>>8);
		Display.framebuffer[(ypos<<2)+(xpos>>3)+1] |= (widebar);
	}
	else
	{
		Display.framebuffer[(ypos<<2)+(xpos>>3)] &= ~(widebar>>8);
		Display.framebuffer[(ypos<<2)+(xpos>>3)+1] &= ~(widebar);
	}
}

void DisplayTimeDigit(uint8_t num, uint8_t pos)
{
	DisplayDigit(num, pos + TIME_X_OFFSET, TIME_Y_OFFSET);
}

void DisplayDateDigit(uint8_t num, uint8_t pos)
{
	DisplayDigit(num, pos + TIME_X_OFFSET, TIME_Y_OFFSET+10);
}

void DisplayDigit(uint8_t dig, uint8_t xpos, uint8_t ypos)
{
	for (int j=0; j<5; j++)
	{
		uint8_t temp;
		
		temp = Display.framebuffer[((j+ypos)<<2)+(xpos>>1)] & ((xpos&1)?(0xf0):(0x0f));
		Display.framebuffer[((j+ypos)<<2)+(xpos>>1)] = temp | ((xpos&1)?(digits[dig][j]>>5):(digits[dig][j]>>1));
	}
}

// Process button input

void PollButtons()
{
	uint32_t debouncetime;
	debouncetime = SafeMillis();
	
	if (debouncetime > (buttonDebounce + BUTTON_DEBOUNCE))
	{
		buttonDebounce = debouncetime;
		
		buttonPollState = BUTTONS_CURRENT;
		buttonPollStatePrev ^= buttonPollState;		// buttonPollStatePrev is nonzero if there has been a change.
	
		buttonsReleased = buttonPollStatePrev & buttonPollState;
		buttonsPressed = buttonPollStatePrev & ~buttonPollState;
	
		buttonPollStatePrev = buttonPollState;
	}
	else
	{
		buttonsReleased = 0;
		buttonsPressed = 0;
	}
}

uint32_t SafeMillis()
{
	uint32_t result;
	
	cli();
	result = millis();
	sei();

	return result;
}
