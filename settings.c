
/*
 *
 * settings.c
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <ctype.h>

#include "header.h"

// initialisiation with default values
struct settingsStruct settings =
{
	"/var/log/",				//filepath for input logfiles
	'#',						//line char
	'#',						//line char interpolated
	'#',						//pillar char
	'-',						//underline char
	'-',						//underline char interpolated
	0,							//xAxis description
	1,							//graph mode
	0,							//debug mode
	"BAT0"		//batteries to display graph for
};


int updateSettingsValue(char *parName, char *parValue)
{
	if(strcmp(parName, "batterylogs_path") == 0)
	{
		strcpy(settings.inputLogfilePath, parValue);
	}
	else if(strcmp(parName, "graph_line_char_data") == 0)
	{
		settings.graphLineCharData = parValue[0];
	}
	else if(strcmp(parName, "graph_line_char_interpolated") == 0)
	{
		settings.graphLineCharInterpolated = parValue[0];
	}
	else if(strcmp(parName, "graph_pillar_char") == 0)
	{
		settings.graphPillarChar = parValue[0];
	}
	else if(strcmp(parName, "graph_underline_char_data") == 0)
	{
		settings.graphUnderlineCharData = parValue[0];
	}
	else if(strcmp(parName, "graph_underline_char_interpolated") == 0)
	{
		settings.graphUnderlineCharInterpolated = parValue[0];
	}
	else if(strcmp(parName, "x_axis_description_mode") == 0)
	{
		settings.xAxisDescriptionMode = atoi(parValue);
	}
	else if(strcmp(parName, "graph_mode") == 0)
	{
		settings.graphMode = atoi(parValue);
	}
	else if(strcmp(parName, "batteries") == 0)
	{
		strcpy(settings.batteriesToLog, parValue);
	}
	else
	{
		return 1;
	}

	return 0;
}

int readSettingsFromConfigFile()
{
	char *rvcharptr;
	int rvint = 0;
	char readString[4198] = "";		//4198 = 100(varname) + 1(=) + 4096(largest size of value) + 1(\0)
	char varName[101] = "";
	char varValue[4097] = "";
	int indexReadString = 0;
	int skipFlag = 0;
	int i = 0;

	if(settings.debugMode >= 1) printf("Load config file:\n");

	FILE *fp = fopen("/etc/batterylog.conf", "r");
	if(fp != NULL)
	{
		rvcharptr = fgets(readString, sizeof(readString), fp);
		for(int line = 1; rvcharptr != NULL; line++)
		{
			//check if line has been commented out
			if(readString[0] != '#' && isspace(readString[0]) == 0)
			{
				indexReadString = 0;
				skipFlag = 0;	//if set to 1, the line has an error and should therefore be skipped

				//skip whitespaces and set skipFlag if nothing else comes till the end of String
				while(isspace(readString[indexReadString]) && indexReadString < SIZEOF_ARRAY(readString))
				{
					indexReadString++;
				}
				if(readString[indexReadString] == '\0')
				{
					skipFlag = 1;
					if(settings.debugMode >= 1) printf("Nothing detected in line %i\n", line);
				}

				//start with copying the varName until you have a whitespace. Set skipFlag, if \0 comes
				for(i = 0; readString[indexReadString] != '=' && !(isspace(readString[indexReadString])) && skipFlag == 0 && i < (SIZEOF_ARRAY(varName) - 1); i++)
				{
					if(readString[indexReadString] != '\0')
					{
						varName[i] = readString[indexReadString];
						indexReadString++;
					}
					else
					{
						skipFlag = 1;
						break;
					}
				}
				varName[i] = '\0';

				//skip whitespaces and set skipFlag if nothing else comes till the end of String
				while(isspace(readString[indexReadString]) && indexReadString < SIZEOF_ARRAY(readString))
				{
					indexReadString++;
				}
				if(readString[indexReadString] == '\0')
				{
					skipFlag = 1;
					if(settings.debugMode >= 1) printf("Nothig after name detected in line %i\n", line);
				}

				//skip the =, which should be there. If not set skipFlag
				if(readString[indexReadString] == '=')
				{
					indexReadString++;
				}
				else
				{
					skipFlag = 1;
					if(settings.debugMode >= 1) printf("No '=' detected in line %i\n", line);
				}

				//skip whitespaces and set skipFlag if nothing else comes till the end of String
				while(isspace(readString[indexReadString]) && indexReadString < SIZEOF_ARRAY(readString))
				{
					indexReadString++;
				}
				if(readString[indexReadString] == '\0')
				{
					skipFlag = 1;
					if(settings.debugMode >= 1) printf("No value detected in line %i\n", line);
				}

				//copy the Value
				for(i = 0; skipFlag == 0 && readString[indexReadString] != '\n' && readString[indexReadString] != '\0' && i < (SIZEOF_ARRAY(varValue) - 1); i++)
				{
						varValue[i] = readString[indexReadString];
						indexReadString++;
				}
				varValue[i] = '\0';

				if(skipFlag == 0)
				{
					if(settings.debugMode >= 1) printf("Attempting to set %s to %s... ", varName, varValue);

					//update config parameters to non default value read from config file
					rvint = updateSettingsValue(varName, varValue);
					if(settings.debugMode >= 1)
					{
						if(rvint == 0)
						{
							printf("Successful\n");
						}
						else
						{
							printf("%sFAILED%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
						}
					}
				}
				else
				{
					if(settings.debugMode >= 1) printf("WARNING: Failed to read line %i of config file\n", line);
				}
			}

			//get new line from config file
			rvcharptr = fgets(readString, sizeof(readString), fp);
		}
	}
	else
	{
		if(settings.debugMode >= 1) printf("Failed to open config file...\n");
		return 1;
	}

	fclose(fp);
	return 0;
}
