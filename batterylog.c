//main.c

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <ctype.h>

#include "defines.h"
#include "settings.h"
#include "ui.h"



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

int copyLogfileToArray(char parArray[][LOGFILE_ENTRY_LENGTH], int const parArraySize, const char filepath[MAX_FILEPATH_STRING_LENGTH])
{
	FILE *logfilePointer = fopen(filepath, "r");
	if(logfilePointer == NULL)
	{
		perror("Error opening Logfile");
		exit(1);
	}
	char readString[LOGFILE_ENTRY_LENGTH];
	char bufferArray[parArraySize][LOGFILE_ENTRY_LENGTH];
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
		for(int i = 0; i < parArraySize ; i++)
		{
			strcpy(parArray[i], bufferArray[(i+(lastArrayEntry-parArraySize)+1)%parArraySize]);
			if(settings.debugMode == 2) printf("parArray[%i] = bufferArray[%i] = \"%s\"\n", i, (i+lastArrayEntry-parArraySize+1)%parArraySize, parArray[i]);
		}
	}
	else
	{
		//fill the dataless entries
		for(int i = 0; i < parArraySize-lastArrayEntry-1; i++)
		{
			strcpy(parArray[i],"\0");
			if(settings.debugMode == 2) printf("parArray[%i] = \"%s\"\n", i, parArray[i]);
		}
		//copy the data in the rest
		for(int i = parArraySize-lastArrayEntry-1, j = 0; i < parArraySize; i++, j++)
		{
			strcpy(parArray[i], bufferArray[j]);
			if(settings.debugMode == 2) printf("parArray[%i] = bufferArray[%i] = \"%s\"\n", i, j, parArray[i]);
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


int logfileArrayToStruct(struct logEntry parStruct[], char parArray[][LOGFILE_ENTRY_LENGTH], int parLenght)
{
	for(int i = 0; i < parLenght; i++)
	{
		char bufferString[5];

		if(parArray[i][0] == '\0')
		{
			//mark this element as empty, that the ui doesnt attempt to draw something
			parStruct[i].empty = 1;
		}
		else
		{
			parStruct[i].empty = 0;

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
				if(settings.debugMode == 2) printf("parArray[%i][20] == \\0 percentage=%hhi\n", i, parStruct[i].percentage);
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
				if(settings.debugMode >= 1) printf("parArray[%i][20/21/22] = \\0 not found percentage=%hhi\n", i, parStruct[i].percentage);
			}

			//set InterpolatedFlag to 0
			parStruct[i].interpolatedFlag = (unsigned char)0;
		}

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

				//calculate m
				interpolationm = ((double)(parInputStruct[inputi+1].percentage - parInputStruct[inputi].percentage)/(double)neededAdditions);
				if(settings.debugMode==2) printf("m=(%i - %i) / %i = %f\n", (int)parInputStruct[inputi+1].percentage, (int)parInputStruct[inputi].percentage, neededAdditions, interpolationm);

				//calculate y (the interpolated percentage)
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

				//set interpolation flag, but not for the last one, which is a real value
				if(i > 0)
				{
					parTimeAccurateStruct[outputi].interpolatedFlag = 1;
				}

				//take the empty flag with you
				parTimeAccurateStruct[outputi].empty = parInputStruct[inputi].empty;


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


int fillBatteriesArray(char array[MAX_BATTERIES][MAX_BATTERY_NAME_LENGTH], char *batteriesString)
{
	char *rvcharp = NULL;
	char buffer[SIZEOF_ARRAY(settings.batteriesToLog)];
	int i;

	strcpy(buffer, batteriesString);

	rvcharp = strtok(buffer, " ");
	for(i = 0; rvcharp != NULL && i < MAX_BATTERIES; i++)
	{
		strcpy(array[i], rvcharp);
		if(settings.debugMode >= 1) printf("Battery %i = \"%s\"\n", i, array[i]);
		rvcharp = strtok(NULL, " ");
	}
	if(settings.debugMode >= 1) printf("%i batteries specified\n", i);

	return i;	//the amount of batteries specified in the configfile string
}


int generateLogfilePathFromBatteryname(char *filepath, const char *batteryname)
{
	strcpy(filepath, settings.batterylogsPath);
	strcat(filepath, "batterylog-");
	strcat(filepath, batteryname);
	strcat(filepath, ".log");

	return 0;
}

int main(int argc, char **argv)
{
	int terminalColumns = getTerminalColumns();
	int tableColumns = terminalColumns - 4;
	char logEntryArray[tableColumns][LOGFILE_ENTRY_LENGTH];
	struct logEntry logEntries[tableColumns];
	char batteries[MAX_BATTERIES][MAX_BATTERY_NAME_LENGTH];
	int amountOfBatteries = 0;
	char logfilePath[MAX_FILEPATH_STRING_LENGTH] = "";

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
				printf("Usage: batterylog [OPTIONS]\n  -o        Output the logfile without interpolating\n  -d        debugging mode\n  -v        Verbose debugging mode\n  --help    Display this help message\n\n");
				printf("Style explanation:\n");
				printf("  %c = Value above is raw logged data\n", settings.graphUnderlineCharData);
				printf("  %s%c%s = Value above was interpolated between log entries\n", ANSI_COLOR_RED, settings.graphUnderlineCharInterpolated, ANSI_COLOR_RESET);
				printf("  %sx%s = No value can be displayed for this point in time\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
				exit(0);
			}
		}
	}

	//fill array with names of batteries
	amountOfBatteries = fillBatteriesArray(batteries, settings.batteriesToLog);


	if(settings.debugMode == 2)
	{
		printf("columns %d\n", terminalColumns);
		printf("tableColumns = %i\n", tableColumns);
		printf("SIZEOF_ARRAY(logEntryArray) = %i\n", SIZEOF_ARRAY(logEntryArray));
		printf("%i batteries specified to output graph for\n", amountOfBatteries);
	}


	for(int i = 0; i < amountOfBatteries; i++)
	{
		generateLogfilePathFromBatteryname(logfilePath, batteries[i]);

		//file -> array
		copyLogfileToArray(logEntryArray, tableColumns, logfilePath);

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
					printf("Struct[%03i] = %4hi-%2hhu-%2hhu-%2hhu-%2hhu: %3hhu ", i, logEntries[i].year, logEntries[i].month, logEntries[i].day, logEntries[i].hour, logEntries[i].minutes, logEntries[i].percentage);
					printf(" %s%s\n", (logEntries[i].interpolatedFlag == 1)?"i":" ", (logEntries[i].empty == 1)?"e":" ");
				}
			}
		}

		//draw UI
		printf("\n");
		printLeftOffset( 4+(terminalColumns-strlen(batteries[i]))/2 );
		printf("%s:\n", batteries[i]);

		printTable(logEntries, tableColumns, terminalColumns);
	}



	return 0;
}

