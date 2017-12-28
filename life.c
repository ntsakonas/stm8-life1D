#include <string.h>
#include <stdio.h>
#include "stm8l.h"

// --- UART CODE ---
// write string to uart
int uart_write(const char *str) {
	char i;
	for(i = 0; i < strlen(str); i++) {
		while(!(USART1_SR & USART_SR_TXE));
		USART1_DR = str[i];
	}
	return(i); // Bytes sent
}

void uart_init()
{
	CLK_DIVR = 0x00; // Set the frequency to 16 MHz
	CLK_PCKENR1 = 0xFF; // Enable peripherals

	PC_DDR = 0x08; // Put TX line on
	PC_CR1 = 0x08;

	USART1_CR2 = USART_CR2_TEN; // Allow TX & RX
	USART1_CR3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
	USART1_BRR2 = 0x03; USART1_BRR1 = 0x68; // 9600 baud

}

// --- GAME ---
// using 16 cells and fixed initial pattern
// number of cells is linked to the number of bits used in the pattern
#define numOfCells  16
int cells[numOfCells+2];
// play for a few generations
int runGenerations = 20;

// set initial pattern
// maybe i should get it from the user through the serial port
// for now, I am setting an oscillating pattern

// 1F -> oscillating patter with period of 6
// manually center alligned: 11111 -> 0000 0011 1110 0000 ->03E0
unsigned int pattern = 0x03E0;

// 27 ->100111 -> 0000 0100 1110 0000 -> 04E0
//unsigned int pattern = 0x04E0;
// 15 -> 10101 -> 0000 0101 0100 0000 -> 0540
//unsigned int pattern = 0x0540;
// 5 -> 101 -> 0000 0010 1000 0000 -> 0280
//unsigned int pattern = 0x0280;

//unsigned int pattern = 0xf0ff;

void print_pattern(int pattern)
{
	char patternFormatted[10];
	sprintf(patternFormatted,"%0X\n",pattern);
	uart_write("Starting pattern ");
	uart_write(patternFormatted);
}

void game_init()
{
	unsigned int mask = 0x8000;
	int cellCounter =0;
	while (cellCounter < numOfCells)
	{
	  	if ((pattern & mask) == mask)
	  	{
	  		cells[cellCounter] = 1;
	  	}
	  	else
	  	{
			cells[cellCounter] = 0;
	  	}
	  	mask = mask >>1;
	  	cellCounter++;
	}
	cells[numOfCells] = 0;
	cells[numOfCells + 1] = 0;
 
}

void calculate_next_generation()
{
	int currentCell = 0;
	int t2  =0;
	int t1 = 0;
	do
	{
		int t0 = cells[currentCell];
		int surroundingCells = t1 + t2 + cells[currentCell+1] + cells[currentCell+2];
		if (t0 == 0 && (surroundingCells == 2 || surroundingCells == 3))
		{
			// cell is born
			cells[currentCell] = 1;	
		}
		else if (t0 == 1 && surroundingCells != 2 && surroundingCells != 4)
		{
			// cell dies
			cells[currentCell] = 0;
		}
		// continue to next cell
		t2 = t1;
		t1 = t0;
		currentCell++;	
	}while (currentCell<numOfCells);
}

void print_cells(int currentGeneration)
{
	int i;
	char generation[8];
	sprintf(generation,"%03d: |",currentGeneration);
	uart_write(generation);
  	for (i=0;i<numOfCells;i++)
  	{	
  		if (cells[i] == 0)
  		{
  			uart_write(" |");
  		}
  		else
  		{
 			uart_write("o|");
  		}
  	} 
  	uart_write("\n");
}

void game_run(int numOfGenerations)
{
	int currentGeneration = 0;
	print_cells(currentGeneration);
	while (currentGeneration < numOfGenerations)
	{
		calculate_next_generation();
		currentGeneration++;
		print_cells(currentGeneration);
	}
}

void main() {
	uart_init();
	uart_write("\nOne Dimensional Life Game (by J.K.Millen Phd, Byte Magazine,DEC 1978)\n");
	uart_write("Coded as a toy project for STM8 by N.Tsakonas (Dec 2017)\n");
	print_pattern(pattern);
	game_init();
	game_run(runGenerations);
	while(1);
}