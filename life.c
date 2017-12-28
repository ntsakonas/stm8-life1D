// This is a toy project for exploring the STM8 mcu.
// the code was tested on a minimal STM8S103F3 board
// the program output is availble via serial port (9600,8,N,1)
// modify the game variables (pattern/generations) to play.
//
// Author: Nick Tsakonas (2017)
//
// One dimensional game of life (by J.K.Millen)
// --------------------------------------------
// This is a one dimensional variation of Conway's Game Of Life, 
// that appearred on Byte Magazine,vol3, No.12 p68-p74 , DEC 1978.
// Cells live in a single row and next generation relies on the +2/-2
// surrounding cells according to the following rules:
// Rule 1: Birth. cells that are OFF but have either two or three neighbors
//		   will go ON.
// Rule 2: Survival.cells that are ON and have two or four neighbors, stay ON.
//         Those with zero or one neighbors ON, die of loneliness.
//         Those with three ON, die from overcrowding.  	
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <string.h>
#include <stdio.h>
#include "stm8l.h"

// --- UART CODE ---
// slightly modified version from sdcc samples
// see https://github.com/vdudouyt/sdcc-examples-stm8
void uart_write(const char *str) {
	char i;
	for(i = 0; i < strlen(str); i++) {
		while(!(USART1_SR & USART_SR_TXE));
		USART1_DR = str[i];
	}
}

void uart_init()
{
	// changing the internal clock does not work this way,
	// so the CPU runs at the default clock of 2MHZ
	// CLK_DIVR = 0x00; // Set the frequency to 16 MHz
	CLK_PCKENR1 = 0xFF; // Enable peripherals

	PC_DDR = 0x08; // Put TX line on
	PC_CR1 = 0x08;

	USART1_CR2 = USART_CR2_TEN; // Allow TX & RX
	USART1_CR3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
	// due to clock mismatch, the initial plan to use 9600 actually configures
	// the UART to 1200, so new calculations are in order using CPU clock = 2Mhz
	// USART1_BRR2 = 0x03; USART1_BRR1 = 0x68; // 9600 baud if it was 16MHZ, but it is actually 1200
	// calculations based on 2MHZ clock
	// USART1_BRR2 = 0x00; USART1_BRR1 = 0x1a; // 4800 baud
	USART1_BRR2 = 0x00; USART1_BRR1 = 0x0d; // 9600 baud
}

// --- GAME ---
// using 16 cells and a selection of initial pattern
#define numOfCells  16
int cells[numOfCells+2];
// GAME VARIABLES
// play up to 20 generations
int maxGenerations = 20;

// set initial pattern
// known patterns from the original article, manually center alligned
// 1F -> oscillating patter with period of 6
// 11111 -> 0000 0011 1110 0000 ->03E0
unsigned int pattern = 0x03E0;

// 15 -> Oscillates,fourth form of number 1F  
// 10101 -> 0000 0101 0100 0000 -> 0540
//unsigned int pattern = 0x0540;

// 17 -> glider with period 1  
// 10111 ->  1011 1000 0000 0000-> b800
// unsigned int pattern = 0xb800;

// 5 -> dies after generation 2 
// 101 -> 0000 0010 1000 0000 -> 0280
//unsigned int pattern = 0x0280;

// 27 -> becomes numbers 3F,DB,2B5,FF,37B,A05,201,0
// 100111 -> 0000 0100 1110 0000 -> 04E0
//unsigned int pattern = 0x04E0;

void print_pattern()
{
	char patternFormatted[20];
	sprintf(patternFormatted,"Using pattern %04X\n",pattern);
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
	  	++cellCounter;
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
		++currentCell;	
	}while (currentCell<numOfCells);
}

void print_cells(int currentGeneration)
{
	int i;
	char generation[8];
	sprintf(generation,"%03d: |",currentGeneration);
	uart_write(generation);
  	for (i=0;i<numOfCells;++i)
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

int colonyIsAlive()
{
	int cellCounter;
	int deadCells = 0;
	for (cellCounter = 0;cellCounter < numOfCells;++cellCounter)
	{
		if (cells[cellCounter] == 0)
		{
			++deadCells;
		}
	}
	return deadCells != numOfCells;
}

void game_run()
{
	int currentGeneration = 0;
	print_cells(currentGeneration);
	while (currentGeneration < maxGenerations && colonyIsAlive())
	{
		calculate_next_generation();
		++currentGeneration;
		print_cells(currentGeneration);
	}
}

void main() {
	uart_init();
	uart_write("\nOne Dimensional Life Game (by J.K.Millen Phd, Byte Magazine,DEC 1978)\n");
	uart_write("Coded as a toy project for STM8 by N.Tsakonas (Dec 2017)\n");
	print_pattern();
	game_init();
	game_run();
	while(1);
}