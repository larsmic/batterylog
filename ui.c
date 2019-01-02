
/*
 *
 *	ui.c
 *
 */

 #include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <ctype.h>

#include "defines.h"
#include "settings.h"


int printLine(int parTerminalColumns, const struct logEntry parStruct[], int parStructSize, int parInterpolationFlag)
{
	int leftOffset = parTerminalColumns-parStructSize;

	for(int i = 0; i < parTerminalColumns; i++)
	{
		if(parStruct[i-leftOffset].empty == 1)
		{
			printf("%sx%s", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		}
		else if(parInterpolationFlag == 1 && i >= leftOffset && parStruct[i-leftOffset].interpolatedFlag == 1)
		{
			printf("%s%c%s", ANSI_COLOR_RED, settings.graphUnderlineCharInterpolated, ANSI_COLOR_RESET);
		}
		else
		{
			printf("%c", settings.graphUnderlineCharData);
		}
	}
	printf("\n");
	return 0;
}

void printLeftOffset(int parOffset)
{
	for(int i = 0; i < parOffset; i++)
	{
		printf(" ");
	}
	return;
}

int printMoreThanTenMinutesDifferenceIndicators(struct logEntry parStruct[], int parStructLenght, int parLeftOffset)
{
	printLeftOffset(parLeftOffset);

	for(int i = 0; i < parStructLenght; i++)
	{
		if( ((parStruct[i].year-parStruct[i-1].year) == 0 || ( (parStruct[i].year-parStruct[i-1].year) == 1 && (parStruct[i].month-parStruct[i-1].month) == -11)) &&
			((parStruct[i].month-parStruct[i-1].month) == 0 || ( (parStruct[i].month-parStruct[i-1].month) == 1 && (parStruct[i].day-parStruct[i-1].day) < -25)) &&
			((parStruct[i].day-parStruct[i-1].day) == 0 || ( (parStruct[i].day-parStruct[i-1].day) == 1 && (parStruct[i].hour-parStruct[i-1].hour) == -23)) &&
			((parStruct[i].hour-parStruct[i-1].hour) == 0 || ( (parStruct[i].hour-parStruct[i-1].hour) == 1 && (parStruct[i].minutes-parStruct[i-1].minutes) >= -59 && (parStruct[i].minutes-parStruct[i-1].minutes) <= -50)) &&
			((parStruct[i].minutes-parStruct[i-1].minutes) > 0 && (parStruct[i].minutes-parStruct[i-1].minutes) <= 10)	)	//gist of it: if every timestamp ist only 0 to 10min after the last one
		{
			printf(" ");
		}
		else
		{
			printf("^", parStruct[i].hour, parStruct[i].minutes);
		}
	}
	return 0;
}

int printInterpolatedIndicators(const struct logEntry parStruct[], int parStructSize, int parNewlineFlag)
{
	for(int i = 0; i < parStructSize; i++)
	{
		if(parStruct[i].interpolatedFlag == 1)
		{
			printf("^");
		}
		else
		{
			printf("*");
		}
	}

	if(parNewlineFlag == 1)
	{
		printf("\n");
	}

	return 0;
}

int printTable(struct logEntry parStruct[], int parStructLenght, int parTerminalColumns)
{
	int minValue;
	int rvint = 0;

	//loop for  the lines
	for(int i = 0; i < 20; i++)
	{
		minValue = 100-i*5;

		if(i%2 == 0)
		{
			printf("%3i ", minValue);
		}
		else
		{
			printLeftOffset(parTerminalColumns-parStructLenght);
		}

		//loop for the columns
		for(int j = 0; j < parStructLenght; j++)
		{
			if(parStruct[j].empty == 1)
			{
				printf(" ");
			}
			else
			{
				if((parStruct[j].percentage >= minValue) && (parStruct[j].percentage < 100-(i-1)*5))
				{
					if(parStruct[j].interpolatedFlag == 0)
					{
						printf("%c", settings.graphLineCharData);
					}
					else
					{
						printf("%c", settings.graphLineCharInterpolated);
					}
				}
				else if(parStruct[j].percentage > minValue && j%6 == 0)
				{
					printf("%c", settings.graphPillarChar);
				}
				else
				{
					printf(" ");
				}
			}
		}
		printf("\n");
	}

	printLine(parTerminalColumns, parStruct, parStructLenght, 1);

	//print time every hour
	printLeftOffset(4);
	for(int i = 0; i < parStructLenght/6; i++)
	{
		if(parStruct[i*6].empty == 0)
		{
			printf("%02hhu:%02hhu ", parStruct[i*6].hour, parStruct[i*6].minutes);
		}
		else
		{
			printLeftOffset(6);
		}
	}
	printf("\n");


	//printLeftOffset(parTerminalColumns-parStructLenght);
	//printInterpolatedIndicators(parStruct, parStructLenght, 1);

	return 0;
}

int printLogStruct(const struct logEntry *ptr)
{
	int rvint = 0;

	rvint = printf("  year=%hi month=%hhu day=%hhu hour=%hhu min=%hhu time=%li interpolatedFlag=%hhu\n", ptr->year, ptr->month, ptr->day, ptr->hour, ptr->minutes, ptr->time, ptr->interpolatedFlag);
	if(rvint < 0)
	{
		return 1;
	}
	return 0;
}
