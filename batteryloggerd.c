//main.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>

#include "defines.h"
#include "settings.h"


char* removeEndingNewline(char *string_pointer) // changes newline characters to null characters - so only use it for fgets
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


int generateInputFilepathFromBatteryname(char *filepath, const char *batteryname)
{
	strcpy(filepath, "/sys/class/power_supply/");
	strcat(filepath, batteryname);
	strcat(filepath, "/capacity");

	return 0;
}


int generateOutputFilepathFromBatteryname(char *filepath, const char *batteryname)
{
	strcpy(filepath, settings.batterylogsPath);
	strcat(filepath, "batterylog-");
	strcat(filepath, batteryname);
	strcat(filepath, ".log");

	return 0;
}

int getBatteryPercentageString(char *outputString, int outputStringSize, const char *batteryname)
{
	char filepath[MAX_FILEPATH_STRING_LENGTH] = "";

	generateInputFilepathFromBatteryname(filepath, batteryname);

	FILE *fp = fopen(filepath, "r");

	if( fgets(outputString, outputStringSize, fp) == NULL)
	{
		return 1;
	}

	outputString = removeEndingNewline(outputString);

	if(settings.debugMode == 1) printf("Current battery percentage (%s): %s\n", batteryname, outputString);

	return 0;
}

int writeLogEntry(const char *percentage, const char *batteryname)
{
	char logfilePath[MAX_FILEPATH_STRING_LENGTH] = "";
	int rvint = 0;

	generateOutputFilepathFromBatteryname(logfilePath, batteryname);

	FILE *fp = fopen(logfilePath, "a");
	if(fp == NULL)
	{
		syslog(LOG_ERR, "Could not open logfile %s", logfilePath);
		return 1;
	}
	time_t rawtime;
	struct tm *timeinfo;
	char timestamp[18];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	rvint = strftime (timestamp, 18, "%Y-%m-%d-%H-%M", timeinfo);
	if(settings.debugMode == 1) printf("Debug: timestamp: %s\n", timestamp);
	if(rvint == 0)
	{
		syslog(LOG_ERR, "Converting time_t with strftime failed");
		return 1;
	}

	rvint = fprintf(fp, "[%s] %s\n", timestamp, percentage);
	if(rvint < 0)
	{
		fclose(fp);
		syslog(LOG_ERR, "Courld not write to logfile %s", logfilePath);
		return 1;
	}

	fclose(fp);
	return 0;

}


int get_current_minutes()
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	if(settings.debugMode == 1) printf("Debug: timeinfo->tm_min: %i\n", timeinfo->tm_min);

	return timeinfo->tm_min;
}


int fillBatteriesArray(char array[MAX_BATTERIES][MAX_BATTERY_NAME_LENGTH])
{
	char *rvcharp = NULL;
	char buffer[SIZEOF_ARRAY(settings.batteriesToLog)];
	int i;

	strcpy(buffer, settings.batteriesToLog);

	rvcharp = strtok(buffer, " ");
	for(i = 0; rvcharp != NULL && i < MAX_BATTERIES; i++)
	{
		strcpy(array[i], rvcharp);
		rvcharp = strtok(NULL, " ");
	}

	return i;	//the amount of batteries specified in the configfile string
}


int writeLogEntryForAllBatteries(const int amountOfBatteries, const char batteriesArray[MAX_BATTERIES][MAX_BATTERY_NAME_LENGTH])
{
	char batteryPercentageString[4];
	int rvint = 0;

	for(int i = 0; i < amountOfBatteries; i++)
	{
		rvint = getBatteryPercentageString(batteryPercentageString, SIZEOF_ARRAY(batteryPercentageString), batteriesArray[i]);
		if(rvint != 0)
		{
			syslog(LOG_ERR, "Courld not read battery percentage for %s", batteriesArray[i]);
			return 1;
		}

		rvint = writeLogEntry(batteryPercentageString, batteriesArray[i]);
		if(rvint != 0)
		{
			syslog(LOG_ERR, "Courld not write log entry to for %s", batteriesArray[i]);
			return 1;
		}
	}

	return 0;
}


int main(int argc, char **argv)
{
	int rvint = 0;
	char *rvcharp = NULL;

	char battery_percentage_string[4];
	char batteryPercentageString[4];
	int sleep_time;
	int currentMinutes = 0;
	char batteries[MAX_BATTERIES][MAX_BATTERY_NAME_LENGTH];
	int amountOfBatteries = 0;
	int onlyOnceFlag = 0;
	
	//get settings
	readSettingsFromConfigFile();	
	
	//fill batteries array and store amount
	amountOfBatteries = fillBatteriesArray(batteries);

	// parsing arguments
	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "--now") == 0)
			{
				onlyOnceFlag = 1;
			}
			else if(strcmp(argv[i], "--help") == 0)
			{
				printf("batteryloggerd [Option]\n --now    Immediately write a logentry and exit\n    --help    Show this message and exit\n\n");
				exit(0);
			}
			else if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0)
			{
				settings.debugMode = 1;
				printf("debug mode enabled");
			}
			else
			{
				fprintf(stderr, "Invalid Arguments. Try --help.\n");
				exit(1);
			}
		}
	}
	
	if(onlyOnceFlag == 1)
	{
		rvint = writeLogEntryForAllBatteries(amountOfBatteries, batteries);
		exit(rvint);
	}
	
	// enter daemon loop
	while(1)
	{
		currentMinutes = get_current_minutes();

		sleep_time = 60*( 10 - ( currentMinutes%10 ) );
		if(settings.debugMode == 1) printf("Sec until next log entry: %i (%i)min\n", sleep_time, sleep_time/60);

		sleep(sleep_time);	//wait until minutes end on 0 (e.g. 12:10 or 15:50)

		rvint = writeLogEntryForAllBatteries(amountOfBatteries, batteries);
		if(rvint != 0)
		{
			exit(1);
		}

	}
	return 0;
}

