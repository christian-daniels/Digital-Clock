/*
 * Project 2.c
 *
 * Created: 2/3/2021 11:35:18 AM
 * Author : Christian Daniels
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"
#define LED 0
#define PB 1
#define HALF_SEC 500
#define FEBLEAP 29
#define PM 1
#define AM 0
#define NO 0
#define YES 1

typedef struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int sec;	
	int meridiem; // 0 is AM, 1 is PM
	int militaryTime;
} DateTime;


int get_key();
int is_pressed(int r, int c);
void dt_init(DateTime *dt);
void dt_advance(DateTime *dt);
void dt_display(const DateTime *dt);
int getMHour(int hour, int meridiam);
void changeMilitarySwitch(DateTime *dt);
void setTimeLoop(DateTime *dt);
int retrieveNumber(int k);
void dt_displaySetMode(const DateTime *dt);
void runTutorial();
int verifyValidDate();
void updateDate(DateTime* newDateTime, int* userInput, int count);
void init_newDate(DateTime *newDateTime);
void displayPlease();
void displayInvalid();
int verifyValidDate(DateTime * toVerify);
// Global variables - Extra credit - Initially set to False

int SET_TIME = 0;
int dayCache[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	
int main(void)
{
	
    // testing if buttons work
	// SET_BIT(DDRB, LED); 
	int k;
	lcd_init();
	// Initialize DateTime data structure
	DateTime dt;
	dt_init(&dt);
	
	
    while (1) 
    {
		//start clock
		k = get_key();
		if (k == 16)
		{
			changeMilitarySwitch(&dt);
		}
		if (k == 13)
		{
			SET_TIME = 1;
			setTimeLoop(&dt);
			avr_wait(500);
			SET_TIME = 0;
		}
		if (!SET_TIME)
		{
			avr_wait(1000);
			dt_advance(&dt);
			dt_display(&dt);
		}
		
		
		/*
		k = get_key();
		for(i = 0; i < k; i++){
			SET_BIT(PORTB, LED);	// turn on LED
			avr_wait(250);				// Internal wait
			CLR_BIT(PORTB, LED);	// turn off LED
			avr_wait(250);
			
		} avr_wait(2000);
		*/
		
		
    }
}

int is_pressed(int r, int c)
{
	// Set DDRC to 0 - input mode
	DDRC = 0;
	// Set all pins to N/C
	PORTC = 0;
	CLR_BIT(DDRC, r);
	SET_BIT(PORTC, r);
	SET_BIT(DDRC, c + 4);
	CLR_BIT(PORTC, c + 4);
	
	// Check if PINC is 1 or not at c
	if (!GET_BIT(PINC, r))
	{
		return 1;
	}
	
	return 0;
}
int get_key(){
	int r,c;
	for(r = 0; r < 4; r++){
		for(c = 0; c < 4; c++){
			if (is_pressed(r, c))
			{
				return 1 +  (r*4 + c);
			}
		}
	}
	return 0;
}


void dt_init(DateTime *dt){
	// Set Date to my birthday
	dt->day = 31;
	dt->year = 1999;
	dt->month = 12;
	dt->hour = 11;
	dt->minute = 50;
	dt->sec = 50;
	dt->meridiem = PM;
	dt->militaryTime = NO;
}


// advances sec variable
void dt_advance(DateTime *dt){
	dt->sec++;
	if (60 == dt->sec){
		dt->sec = 0;
		dt->minute++;
		
		if (dt->minute == 60){
			dt->minute = 0;
			dt->hour++;
			
			
			/*
			Cases that change base on 12
				1. Day changes
				2. AM and PM change
			
			*/
			if(dt->hour == 13){
				dt->hour = 1;
			}
			if(dt->hour == 12){
				
				if (dt->meridiem == AM){
					dt->meridiem = PM;
				}
				else{
					dt->meridiem = AM;
					dt->day++;
					if (dt->day == dayCache[dt->month-1] + 1){
						dt->day = 1;
						dt->month++;
						if (dt->month == 13){
							dt->month = 1;
							dt->year++;
							if (dt->year == 10000){
								dt->year = 1;
							}
						}
					}
				}
					
			}
			
		}
	}
}

/************************************************************************/
/* Prints out time in the form 
	MM/DD/YYYY
	00:00:00 AM                
	
	Creates a character buffer
	                                                     */
/************************************************************************/
void dt_display(const DateTime *dt){
	char date[17]; // 16 cells used for LCD, 1 cell used for terminating char
	sprintf(date, "%02d/%02d/%04d", dt->month, dt->day, dt->year);
	lcd_pos(0,0);
	lcd_puts(date);
	
	char time[17];
	
	if (dt->militaryTime)
	{
		int militaryHR = getMHour(dt->hour, dt->meridiem);
		if (dt->meridiem == PM){
			sprintf(time, "%02d:%02d:%02d PM", militaryHR, dt->minute, dt->sec);
		}
		else{
			sprintf(time, "%02d:%02d:%02d AM", militaryHR, dt->minute, dt->sec);
		}
	}
	else
	{
		if (dt->meridiem == PM){
			sprintf(time, "%02d:%02d:%02d PM", dt->hour, dt->minute, dt->sec);
		}
		else{
			sprintf(time, "%02d:%02d:%02d AM", dt->hour, dt->minute, dt->sec);
		}
	}
	
	lcd_pos(1,0);
	lcd_puts(time);
}
int getMHour(int hour, int meridiam){
	if (hour < 12){
		if (meridiam == PM ){
			return hour + 12;
		}
		else{
			return hour;
		}
	}
	else if (meridiam == PM){
		return hour;
	}
	else{
		return 0;
	}
}
void changeMilitarySwitch(DateTime *dt){
	if (dt->militaryTime == NO)
	{
		dt->militaryTime = YES;
	}
	else{
		dt->militaryTime = NO;
	}
}

/*
	SET Mode loop
	1. Retrieves user input
	2. Will add to time
	3. Will exit out of SET Mode
	
	I wanted to demonstrate that in my project the way I set the clock is directly 
	through the numbers on the key pad. On an actual clock it would only allow a 
	user to move up and down, but since we have a keypad I thought why not
	Because of this the user can input basically whatever numbers they want
	I handled this by making it so once a user inputs * the code will verify the 
	inputted date. If the date was wrong an error is displayed and the user must start
	over. If the date was valid the clock starts ticking.
*/
void setTimeLoop(DateTime *dt){
	int k;
	runTutorial();
	DateTime newDateTime;
	init_newDate(&newDateTime);
	newDateTime.meridiem = PM;
	
	int count = 0;
	int userInput[14] = {0};
	dt_displaySetMode(&newDateTime);
	avr_wait(1000);
	while (1)
	{
		k = get_key();
		// A number is being read - add it to new struct
		if (((k > 0 && k < 4) || (k > 4 && k < 8) || (k > 8 && k < 12) || k == 14 ) && count < 14){
			userInput[count]= retrieveNumber(k);
			count++;
			updateDate(&newDateTime , userInput, count);
			
			
		}
		// Exiting SET mode
		else if (k == 13)
		{
			if(verifyValidDate(&newDateTime)){
				char validline1[17];
				char validline2[17];
				
				sprintf(validline2, "---------------");
				sprintf(validline1, "--VALID INPUT--");
				lcd_pos(0,0);
				lcd_puts(validline1);
				lcd_pos(1,0);
				lcd_puts(validline2);
				avr_wait(2000);
				lcd_init();
				break;
			}
			// Invalid date was read in - clear screen for half sec (500)
			//		Display "Invalid Date Try Again"
			//		Set New Date
			else{
			
				for (int j = 0;  j < 14; j++)
				{
					userInput[j] = 0;
				}
				count = 0;
				init_newDate(&newDateTime);
				displayPlease();
			}
		}
		// Changing AM/PM
		else if (k == 15){
			if (newDateTime.meridiem == AM){
				newDateTime.meridiem = PM;
			}
			else{
				newDateTime.meridiem = AM;
			}
		}
		// Delete
		else if (k == 16){
			if (count > 0){
				userInput[count] = 0;
				count--;
				/*
				if (count == 0){
					userInput[count] = 0;
				}*/
				updateDate(&newDateTime , userInput, count);
			}
			if (count == 0)
			{
				userInput[count] = 0;
				init_newDate(&newDateTime);
			}
			updateDate(&newDateTime , userInput, count);
		}
		dt_displaySetMode(&newDateTime);
		avr_wait(250);
	}
	
	// Set user found 
	dt->day = newDateTime.day;
	dt->month = newDateTime.month;
	dt->year = newDateTime.year;
	dt->hour = newDateTime.hour;
	dt->minute = newDateTime.minute;
	dt->sec = newDateTime.sec;
	dt->meridiem = newDateTime.meridiem;
	dt->militaryTime = NO; // by default
}
int retrieveNumber(int k){
	
	if (k < 4 && k > 0)
	{
		return k;
	}
	else if (k > 4 && k < 8)
	{
		return k-1;
	}
	else if (k > 8 && k < 12)
	{
		return k - 2;
	}
	else{
		return 0;
	}
}

void runTutorial(){
	char tutLine1[17];
	char tutLine2[17];
	sprintf(tutLine1, "* to Verify");
	sprintf(tutLine2, "D to Delete");
	
	lcd_pos(0,0);
	lcd_puts(tutLine1);
	lcd_pos(1,0);
	lcd_puts(tutLine2);
	avr_wait(2000);
	
	char tutLine3[17];
	char tutLine4[17];
	sprintf(tutLine3, "Month is 1-12 ");
	sprintf(tutLine4, "Day is 1-Month");
	lcd_pos(0,0);
	lcd_puts(tutLine3);
	lcd_pos(1,0);
	lcd_puts(tutLine4);
	avr_wait(2000);
	
	char tutLine5[17];
	char tutLine6[17];
	sprintf(tutLine5, "Year is 1-9999");
	sprintf(tutLine6, "Hr is 1-12    ");
	lcd_pos(0,0);
	lcd_puts(tutLine5);
	lcd_pos(1,0);
	lcd_puts(tutLine6);
	avr_wait(2000);
	
	char tutLine7[17];
	char tutLine8[17];
	sprintf(tutLine7, "Min is 1-59     ");
	sprintf(tutLine8, "Sec is 1-59     ");
	lcd_pos(0,0);
	lcd_puts(tutLine7);
	lcd_pos(1,0);
	lcd_puts(tutLine8);
	avr_wait(2000);
	
}

void dt_displaySetMode(const DateTime *dt){
	char date[17]; // 16 cells used for LCD, 1 cell used for terminating char
	sprintf(date, "%02d/%02d/%04d SET", dt->month, dt->day, dt->year);
	lcd_pos(0,0);
	lcd_puts(date);
	
	char time[17];
	
	if (dt->militaryTime)
	{
		int militaryHR = getMHour(dt->hour, dt->meridiem);
		if (dt->meridiem == PM){
			sprintf(time, "%02d:%02d:%02d PM", militaryHR, dt->minute, dt->sec);
		}
		else{
			sprintf(time, "%02d:%02d:%02d AM", militaryHR, dt->minute, dt->sec);
		}
	}
	else
	{
		if (dt->meridiem == PM){
			sprintf(time, "%02d:%02d:%02d PM <-#", dt->hour, dt->minute, dt->sec);
		}
		else{
			sprintf(time, "%02d:%02d:%02d AM <-#", dt->hour, dt->minute, dt->sec);
		}
	}
	
	lcd_pos(1,0);
	lcd_puts(time);
}
void init_newDate(DateTime *newDateTime){
	newDateTime->day = 0;
	newDateTime->month = 0;
	newDateTime->year = 0;
	newDateTime->hour = 0;
	newDateTime->minute = 0;
	newDateTime->sec = 0;
	newDateTime->militaryTime = NO;
	
}

void updateDate(DateTime* newDateTime, int *userInput, int count)
{
	// Retrieve month
	int i;
	
	for (i = 0; i < 2; i++){
		if (i == 0 ){
			newDateTime->month = userInput[i] * 10;
		}
		else{
			newDateTime->month += userInput[i];
		}
		
	}
	// Retrieve day
	for (i = 2; i < 4 ; i++){
		if (i == 2 ){
			newDateTime->day = userInput[i] * 10;
		}
		else{
			newDateTime->day += userInput[i];
		}
	}
	
	// Retrieve year
	for (i = 4; i < 8 ; i++){
		if (i == 4 ){
			newDateTime->year = userInput[i] * 1000;
		}
		else if (i == 5){
			newDateTime->year += userInput[i] * 100;
		}
		else if (i == 6)
		{
			newDateTime->year += userInput[i] * 10;
		}
		else{
			newDateTime->year += userInput[i];
		}
	}
	// Retrieve Hour
	for(i = 8; i < 10 ; i++){
		if (i == 8 ){
			newDateTime->hour = userInput[i] * 10;
		}
		else{
			newDateTime->hour += userInput[i];
		}
	}
	
	// Retrieve Min
	for(i = 10; i < 12 ; i++){
		if (i == 10){
			newDateTime->minute = userInput[i] * 10;
		}
		else{
			newDateTime->minute += userInput[i];
		}
	}
	
	// Retrieve Sec
	for(i = 12; i < 14 ; i++){
		if (i == 12){
			newDateTime->sec = userInput[i] * 10;
		}
		else{
			newDateTime->sec += userInput[i];
		}
	}
}

int verifyValidDate(DateTime * toVerify){
	/*
		To have a valid date what do you need
		-You need to make sure the day is set within 1 and month's day limit
		-Hour needs to be less than 13 and greater than 0
		-month needs to be between 1 and 12
		-year needs to be between 1 and 9999
		-hour needs to b
	*/
	
	if (1 <= toVerify->month && toVerify->month <= 12)
	{
		// passes continues to next code block
	}
	else{
		displayInvalid();
		char invalidMonth[17];
		sprintf(invalidMonth, "Month was wrong");
		lcd_pos(1,0);
		lcd_puts(invalidMonth);
		avr_wait(1000);
		return 0;
	}
	if (1 <= toVerify->day && toVerify->day <= dayCache[toVerify->month - 1])
	{
		// passes
	}
	else{
		displayInvalid();
		char invalidDay[17];
		sprintf(invalidDay, "Day was wrong");
		lcd_pos(1,0);
		lcd_puts(invalidDay);
		avr_wait(1000);
		return 0;
	}
	if (1 <= toVerify->year && toVerify->year <= 9999)
	{
	}
	else{
		displayInvalid();
		char invalidYear[17];
		sprintf(invalidYear, "Year was wrong");
		lcd_pos(1,0);
		lcd_puts(invalidYear);
		avr_wait(1000);
		
		return 0;
	}
	if (1 <= toVerify->hour && toVerify->hour <= 12)
	{
		
	}
	else{
		displayInvalid();
		char invalidHour[17];
		sprintf(invalidHour, "Hour was wrong");
		lcd_pos(1,0);
		lcd_puts(invalidHour);
		avr_wait(1000);
		return 0;
	}
	if (1 <= toVerify->minute && toVerify->minute <= 59)
	{
		
	}
	else{
		displayInvalid();
		char invalidMinute[17];
		sprintf(invalidMinute, "Min was wrong");
		lcd_pos(1,0);
		lcd_puts(invalidMinute);
		avr_wait(1000);
		return 0;
	}
	if (1 <= toVerify->sec && toVerify->sec <= 59)
	{
	}
	else{
		displayInvalid();
		char invalidSec[17];
		sprintf(invalidSec, "Sec was wrong");
		lcd_pos(1,0);
		lcd_puts(invalidSec);
		avr_wait(1000);
		return 0;
	}
	return 1;
}

void displayInvalid(){
	char invalidLine1[17];
	sprintf(invalidLine1, "INVALID INPUT");
	lcd_pos(0,0);
	lcd_puts(invalidLine1);
	
}

void displayPlease(){
	char pleaseLine1[17];
	char pleaseLine2[17];
	sprintf(pleaseLine1, "   Please      ");
	sprintf(pleaseLine2, "  Try Again    ");
	lcd_pos(0,0);
	lcd_puts(pleaseLine1);
	lcd_pos (1,0);
	lcd_puts(pleaseLine2);
	avr_wait(1000);
}