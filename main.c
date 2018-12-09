/*
 * main.c
 *
 * Copyright 2018  <>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <ctype.h>

#include "header.h"



/* FUNCTIONS */

char *removeEndingNewline(char *string_pointer) // changes newline characters to null characters - so only use it for fgets or similar
{
	for(int i = 0; i <= strlen(string_pointer); i++)
	{
		if(string_pointer[i] == '\n')
		{
			string_pointer[i] = '\0';
		}
	}
	return string_pointer;
}

int copyLogfileToArray(char parArray[][LOGFILE_ENTRY_LENGHT], int const parArraySize)
{
	FILE *logfilePointer = fopen(settings.batteryPercentagelogfilePath, "r");
	if(logfilePointer == NULL)
	{
		perror("Error opening Logfile");
		return 1;
	}
	char readString[LOGFILE_ENTRY_LENGHT];
	char bufferArray[parArraySize][LOGFILE_ENTRY_LENGHT];
	int line = 0;
	int lastArrayEntry = 0;
	char* rvcharptr;

	for(line = 0; fgets(readString, SIZEOF_ARRAY(readString), logfilePointer) != NULL; line++)
	{
		strcpy(bufferArray[line%parArraySize], removeEndingNewline(readString));
		if(settings.debugMode == 2) printf("Line %i: bufferArray[%i] = %s\n", line, line%parArraySize, bufferArray[line%parArraySize]);
		lastArrayEntry = line;
	}
	if(line == 0)
	{
		perror("Could not read from logfile");
		return 1;
	}
	if(settings.debugMode == 2) printf("LastArrayEntry: %i\n", lastArrayEntry);

	fclose(logfilePointer);

	// Sort the bufferArray and save it to the givin Array
	if( lastArrayEntry >= parArraySize )
	{
		for(int i = 0; i < parArraySize ; i++)	//SAVE: i+(lastArrayEntry-parArraySize) <= lastArrayEntry
		{
			strcpy(parArray[i], bufferArray[(i+(lastArrayEntry-parArraySize)+1)%parArraySize]);
			if(settings.debugMode == 2) printf("parArray[%i] = bufferArray[%i] = \"%s\"\n", i, (i+lastArrayEntry-parArraySize+1)%parArraySize, parArray[i]);
		}
	}

	return 0;
}

int getTerminalColumns()
{
	struct winsize windowsize;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowsize);

	return windowsize.ws_col;
}


int logfileArrayToStruct(struct logEntry parStruct[], char parArray[][LOGFILE_ENTRY_LENGHT], int parLenght)
{
	for(int i = 0; i < parLenght; i++)
	{
		char bufferString[5];

		// [YYYY-MM-DD-HH-MM] p or pp or ppp

		//copy year
		bufferString[0] = parArray[i][1];
		bufferString[1] = parArray[i][2];
		bufferString[2] = parArray[i][3];
		bufferString[3] = parArray[i][4];
		bufferString[4] = '\0';
		parStruct[i].year = (short)atoi(bufferString);

		//copy month
		bufferString[0] = parArray[i][6];
		bufferString[1] = parArray[i][7];
		bufferString[2] = '\0';
		parStruct[i].month = (unsigned char)atoi(bufferString);

		//copy day
		bufferString[0] = parArray[i][9];
		bufferString[1] = parArray[i][10];
		bufferString[2] = '\0';
		parStruct[i].day = (unsigned char)atoi(bufferString);

		//copy hour
		bufferString[0] = parArray[i][12];
		bufferString[1] = parArray[i][13];
		bufferString[2] = '\0';
		parStruct[i].hour = (unsigned char)atoi(bufferString);

		//copy minutes
		bufferString[0] = parArray[i][15];
		bufferString[1] = parArray[i][16];
		bufferString[2] = '\0';
		parStruct[i].minutes = (unsigned char)atoi(bufferString);

		//copy percentage
		if(parArray[i][20] == '\0')
		{
			bufferString[0] = parArray[i][19];
			bufferString[1] = '\0';
			parStruct[i].percentage = (unsigned char)atoi(bufferString);
			if(settings.debugMode == 2) printf("parArray[%i][20] == '\0' percentage=%hhi\n", i, parStruct[i].percentage);
		}
		else if(parArray[i][21] == '\0')
		{
			bufferString[0] = parArray[i][19];
			bufferString[1] = parArray[i][20];
			bufferString[2] = '\0';
			parStruct[i].percentage = (unsigned char)atoi(bufferString);
			if(settings.debugMode == 2) printf("parArray[%i][21] == \\0 percentage=%hhi\n", i, parStruct[i].percentage);
		}
		else if(parArray[i][22] == '\0')
		{
			bufferString[0] = parArray[i][19];
			bufferString[1] = parArray[i][20];
			bufferString[2] = parArray[i][21];
			bufferString[3] = '\0';
			parStruct[i].percentage = (unsigned char)atoi(bufferString);
			if(settings.debugMode == 2) printf("parArray[%i][22] == \\0 percentage=%hhi\n", i, parStruct[i].percentage);
		}
		else
		{
			parStruct[i].percentage = 0;
			if(settings.debugMode >= 1) printf("parArray[%i][20/21/22] = \\0 not found percentage=%hhi\n", parStruct[i].percentage);
		}

		//set InterpolatedFlag to 0
		parStruct[i].interpolatedFlag = (unsigned char)0;

	}

	return 0;
}

int generateTimeAccurateStruct(struct logEntry parTimeAccurateStruct[], struct logEntry parInputStruct[], const int parStructSize)
{
	struct tm timestructBuffer={0,0,0,0,0,0,0,0,0};
	struct tm *timestructPointer;
	double timediff = 0.0;
	int inputi = parStructSize-1;
	int outputi = parStructSize-1;
	int neededAdditions = 1;
	double interpolationm = 1.0;

	//first one by hand, because it is required for the next ones
	timestructBuffer.tm_year = (int)parInputStruct[inputi].year;
	timestructBuffer.tm_mon = (int)parInputStruct[inputi].month;
	timestructBuffer.tm_mday = (int)parInputStruct[inputi].day;
	timestructBuffer.tm_hour = (int)parInputStruct[inputi].hour;
	timestructBuffer.tm_min = (int)parInputStruct[inputi].minutes;
	parInputStruct[inputi].time = mktime(&timestructBuffer);
	if(parInputStruct[inputi].time == -1)
	{
		if(settings.debugMode >= 1) printf("Date conversion failed!\n");
	}
	parTimeAccurateStruct[outputi] = parInputStruct[inputi];

	if(settings.debugMode == 2) printf("accStruct[%i] = inputStruct[%i] = %hhu\n", outputi, inputi, parTimeAccurateStruct[outputi].percentage);

	inputi--;
	outputi--;


	while(outputi >= 0)
	{
		timestructBuffer.tm_year = (int)parInputStruct[inputi].year;
		timestructBuffer.tm_mon = (int)parInputStruct[inputi].month;
		timestructBuffer.tm_mday = (int)parInputStruct[inputi].day;
		timestructBuffer.tm_hour = (int)parInputStruct[inputi].hour;
		timestructBuffer.tm_min = (int)parInputStruct[inputi].minutes;
		parInputStruct[inputi].time = mktime(&timestructBuffer);
		if(parInputStruct[inputi].time == -1)
		{
			if(settings.debugMode >= 1) printf("==> Date conversion failed!\n");
		}

		//check if interpolation has to be done
		timediff = difftime(parInputStruct[inputi+1].time, parInputStruct[inputi].time);
		if(settings.debugMode == 2) printf("\n==> timediff = %li - %li = %f\n", parInputStruct[inputi+1].time, parInputStruct[inputi].time, timediff);
		if(timediff >= 1200.0)
		{
			//minimum one entry must be added to the accurate StructArray
			neededAdditions = (int)(timediff/600.0);
			if(settings.debugMode == 2) printf("==> neededAdditions=%i, spacesleft=%i, %i-%hhu-%hhu-%hhu-%hhu=%hhu(%li) -> %i-%hhu-%hhu-%hhu-%hhu=%hhu(%li)\n", neededAdditions, outputi+1, parInputStruct[inputi].year, parInputStruct[inputi].month, parInputStruct[inputi].day, parInputStruct[inputi].hour, parInputStruct[inputi].minutes, parInputStruct[inputi].percentage, parInputStruct[inputi].time, parInputStruct[inputi+1].year, parInputStruct[inputi+1].month, parInputStruct[inputi+1].day, parInputStruct[inputi+1].hour, parInputStruct[inputi+1].minutes, parInputStruct[inputi+1].percentage, parInputStruct[inputi+1].time);


			for(int i = neededAdditions; i >= 0 && outputi >= 0; i--)
			{
				//Interpolate the values between logentries
				//y=m*x+b	m=y2-y1/x2-x1 = y2-y1/neededActions
				//m=(parInputStruct[inputi].percentage - parInputStruct[inputi+1].percentage)/neededAdditions		b=parInputStruct[inputi].percentage
				//m=percentage(n+1)-percentage(n)/neededAdditions
				interpolationm = ((double)(parInputStruct[inputi+1].percentage - parInputStruct[inputi].percentage)/(double)neededAdditions);
				if(settings.debugMode==2) printf("m=(%i - %i) / %i = %f\n", (int)parInputStruct[inputi+1].percentage, (int)parInputStruct[inputi].percentage, neededAdditions, interpolationm);

				parTimeAccurateStruct[outputi].percentage = (unsigned char)(interpolationm * i + (double)parInputStruct[inputi].percentage);
				if(settings.debugMode==2) printf("p=%hhu = %f * %i + %i\n", parTimeAccurateStruct[outputi].percentage, interpolationm, i, (int)parInputStruct[inputi].percentage);

				//Interpolate the timestamps
				parTimeAccurateStruct[outputi].time = parInputStruct[inputi].time + i*(timediff/neededAdditions);
				timestructPointer = localtime(&parTimeAccurateStruct[outputi].time);
				parTimeAccurateStruct[outputi].year = (short)timestructPointer->tm_year;
				parTimeAccurateStruct[outputi].month = (unsigned char)timestructPointer->tm_mon;
				parTimeAccurateStruct[outputi].day = (unsigned char)timestructPointer->tm_mday;
				parTimeAccurateStruct[outputi].hour = (unsigned char)timestructPointer->tm_hour;
				parTimeAccurateStruct[outputi].minutes = (unsigned char)timestructPointer->tm_min;

				//set interpolation flag
				parTimeAccurateStruct[outputi].interpolatedFlag = 1;


				if(settings.debugMode == 2) printf("==> accStruct[%i] = inputStruct[%i] = %hhu (interpolated)\n", outputi, inputi, parTimeAccurateStruct[outputi].percentage);
				if(settings.debugMode == 2) printLogStruct(&parTimeAccurateStruct[outputi]);
				if(settings.debugMode == 2) printf("\n");

				outputi--;
			}
		}
		else
		{
			//no interpolation required just copying
			parTimeAccurateStruct[outputi] = parInputStruct[inputi];

			if(settings.debugMode == 2) printf("accStruct[%i] = inputStruct[%i] = %hhu\n", outputi, inputi, parTimeAccurateStruct[outputi].percentage);

			outputi--;
		}

		inputi--;
	}

	return 0;
}




int main(int argc, char **argv)
{
	int terminalColumns = getTerminalColumns();
	int tableColumns = terminalColumns - 4;
	char logEntryArray[tableColumns][LOGFILE_ENTRY_LENGHT];
	struct logEntry logEntries[tableColumns];

	//load config before argument check to override configfile values by arguments
	readSettingsFromConfigFile();

	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			if( strcmp(argv[i], "-d") == 0 )
			{
					settings.debugMode = 1;
					printf("Debug mode activated.\n");
			}
			if( strcmp(argv[i], "-v") == 0 )
			{
					settings.debugMode = 2;
			}
			if(strcmp(argv[i], "-o") == 0)
			{
				settings.graphMode = 0;
			}
			if(strcmp(argv[i], "--help") == 0)
			{
				printf("Usage: batterylog [OPTIONS]\n\n  -o        Output the logfile without interpolating\n  -d        debugging mode\n  -v        Verbose debugging mode\n  --help    Display this help message\n");
				exit(0);
			}
		}
	}

	if(settings.debugMode == 2)
	{
		printf("columns %d\n", terminalColumns);
		printf("tableColumns = %i\n", tableColumns);
		printf("SIZEOF_ARRAY(logEntryArray) = %i\n", SIZEOF_ARRAY(logEntryArray));
	}



	//file -> array
	copyLogfileToArray(logEntryArray, tableColumns);

	if(settings.debugMode == 2)
	{
		for(int i = 0; i < tableColumns; i++)
		{
			printf("A[%i] = \"%s\"\n", i, logEntryArray[i]);
		}
	}

	//array -> struct
	logfileArrayToStruct(logEntries, logEntryArray, tableColumns);

	//convert to interpolated version for graphMode=1
	if(settings.graphMode == 1)
	{
		struct logEntry timeAccStruct[tableColumns];

		generateTimeAccurateStruct(timeAccStruct, logEntries, tableColumns);

		memcpy(&logEntries, &timeAccStruct, sizeof(logEntries));

		if(settings.debugMode >= 1)
		{
			for(int i = 0; i < tableColumns; i++)
			{
				printf("Struct[%i] = %hi-%hhu-%hhu-%hhu-%hhu: %hhu\n", i, logEntries[i].year, logEntries[i].month, logEntries[i].day, logEntries[i].hour, logEntries[i].minutes, logEntries[i].percentage);
			}
		}
	}

	//draw UI
	printTable(logEntries, tableColumns, terminalColumns);

	return 0;
}

