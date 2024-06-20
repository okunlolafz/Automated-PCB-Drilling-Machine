#include <avr/io.h>
#include <util/delay.h>

// INITIALIZE ALL VARIABLES
int speed = 0;
int no_of_steps = 0;
char steps_list[3];
int max_step = 1000000;

void initialize_instructions()
{
	//INITIALIZE INSTRUCTIONS FOR INSTRUCTION REGISTER
	load_instruction(0X38);		// Set LCD to 16x2 Format
	load_instruction(0X06);		// Auto-Increment Cursor to Right
	load_instruction(0X0C);		// Display ON, Cursor OFF
	load_instruction(0X01);		// Clear screen
}

// LOAD DATA TO LCD DATA REGISTER
void load_data(unsigned char data)
{
	PORTD = data;													// pass received data to PORTB
	PORTC |= (1<<PINC1) | (1<<PINC0);		// RS=1, EN=1
	_delay_ms(1); PORTC &= (~(1<<PINC0));							// EN=0
}

// PRINT STRING TO LCD
void print(const unsigned char *str, unsigned char length)
{
	char i=0;
	for (i=0;i<length;i++) { load_data(str[i]);}
}

// LOAD INSTRUCTION TO LCD INSTRUCTION REGISTER
void load_instruction(unsigned char instruct)
{
	PORTD = instruct;												// pass received instruction to PortD
	PORTC &= (~(1<<PINC1));	PORTC |= (1<<PINC0);	// RS=0, EN=1
	_delay_ms(1); PORTC &= (~(1<<PINC0));							// EN=0
}

// MAIN PROGRAM
int main(void)
{
	// INITIALIZE PORTS
	DDRB = 0X0F;					// Set first 4 bits as OUTPUT
	DDRC = 0X03;					// Set first 2 bits as OUTPUT & the rest as INPUT
	DDRD = 0XFF;					// Set all as OUTPUT
	initialize_instructions();
	
	while (1)
	{
		// IF EXTREME END IS NOT REACHED	MACHINE CAN CONTINUE WORK
		if ((no_of_steps>-1) & (no_of_steps<max_step+1))
		{
			//IF START BUTT0N IS ON
			if (PINC & (1<<PINC2))
			{
				// PRINT START TO SCREEN
				print_start();
				
				// IF SET TO CLOCKWISE, START MACHINE CLOCKWISE
				if (PINC & (1<<PINC3))	{clockwise();}
				
				// IF SET TO ANTI-CLOCKWISE, START MACHINE ANTI-CLOCKWISE
				else   {antclockwise();}
			}
			
			//IF START BUTTON IS OFF, PRINT STOP TO SCREEN & STOP THE MACHINE
			else    {print_stop();	PORTB = 0X00;}
		}
		
		// IF EXTREME END IS REACHED
		else
		{
			// STOP MACHINE IMMEDIATELY
			PORTB = 0X00; print_step();
			
			// CHECK IF DIRECTION HAS BEEN REVERSED AND ONLY RESTART THE MACHINE IN THE REVERSED DIRECTION
			if ((no_of_steps>50) & (~(PINC & (1<<PINC3))))	{antclockwise();}
			if ((no_of_steps<0) & ((PINC & (1<<PINC3))))	{clockwise();}
		}
	}
}


// SUB-FUNCTIONS
void clockwise()
{
	load_instruction(0xC0); print("clockwise     ",14);
	
	// IF STILL CLOCKWISE				 MOVE BY +1 STEP, INCREMENT NO OF STEPS, CHECK FOR EXTREME ENDS, CHECK SPEED
	if (PINC & (1<<PINC3))		{PORTB = 0X09; inc_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
	if (PINC & (1<<PINC3))		{PORTB = 0X0C; inc_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
	if (PINC & (1<<PINC3))		{PORTB = 0X06; inc_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
	if (PINC & (1<<PINC3))		{PORTB = 0X03; inc_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
}

void antclockwise()
{
	load_instruction(0xC0);	print("anti-clockwise",14);
	
	// IF STILL ANTI-CLOCKWISE			MOVE BY -1 STEP, DECREMENT NO OF STEPS, CHECK FOR EXTREME ENDS, CHECK SPEED
	if (~(PINC & (1<<PINC3)))	{PORTB = 0X03; dec_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
	if (~(PINC & (1<<PINC3)))	{PORTB = 0X06; dec_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
	if (~(PINC & (1<<PINC3)))	{PORTB = 0X0C; dec_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
	if (~(PINC & (1<<PINC3)))	{PORTB = 0X09; dec_step(); speedometer(); _delay_ms(125);	if (speed == 0) {_delay_ms(250);}}
}

void speedometer()
{
	// IF SPEED BTN IS PRESSED				SET SPEED
	if (PINC & (1<<PINC4))			{speed = 1;} else {speed = 0;}
}

void print_start()
{
	// PRINT START TO SCREEN
	load_instruction(0X80);	print("START",5); print_step();
}

void print_stop()
{
	// PRINT STOP TO SCREEN AND CLEAR SECOND LINE
	load_instruction(0x80);	print("STOP ",5);	load_instruction(0xC0);	print("                ",16); print_step();
}

void print_step()
{
	// CONVERT INTEGER TO STRING					PRINT IT TO LEFT SIDE OF THE SCREEN
	sprintf(steps_list, "%d", no_of_steps);		load_instruction(0X89);	print(steps_list, 7);
}

void inc_step()
{
	// CHECK RESET BUTTON
	if (~(PINB & (1<<PINB5)))
	{
		// IF MAXIMUM STEP ISNT REACHED
		if (no_of_steps<max_step)
		{
			// IF EXTREME (RIGHT) IS REACHED STOP MACHINE
			if (PINC & (1<<PINC5)) {PORTB = 0X00;}
			
			// IF EXTREME (RIGHT) ISNT REACHED, CONITNUE MACHINE
			else {no_of_steps++; print_step();}
		}
		
		// IF MAXIMUM STEP IS REACHED, STOP MACHINE
		else {PORTB = 0X00;}
	}
	else {reset();}
	
}

void dec_step()
{
	// CHECK RESET BUTTON
	if (~(PINB & (1<<PINB5)))
	{
		// IF MINIMUM STEP ISNT REACHED
		if (no_of_steps>0)
		{
			// IF EXTREME (LEFT) IS REACHED STOP MACHINE
			if (PINB & (1<<PINB4)) {PORTB = 0X00;}
			
			// IF EXTREME (LEFT) ISNT REACHED, CONITNUE MACHINE
			else {no_of_steps--; print_step();}
		}
		
		// IF MINIMUM STEP IS REACHED, STOP MACHINE
		else {PORTB = 0X00;}
	}
	else {reset();}
	
}

void reset()
{
	speed = 0;
	no_of_steps = 0;
	initialize_instructions();
}