//defines.h

#define MAX_FILEPATH_STRING_LENGTH 4097
#define MAX_BATTERIES 10
#define MAX_BATTERY_NAME_LENGTH 100

#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof(a[0] ))

#define LOGFILE_ENTRY_LENGTH 30
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* global variables */

struct settingsStruct
{
	/*
	* Specifications:
	* - maximal length of varnames of settings is 100 chars without '\0'
	* - maximal length of values is 4096 without '\0'
	*
	* If you add an option to the settings struct, also include suppurt for it in:
	* - the updateSettingsValue function (if config file support is logical)
	* - the global definition of default values
	* If config file support is not logical, note it as comment at the definition of the variable.
	*/
	char batterylogsPath[4097];	//maximal characters allowed for filepaths in ext4, plus one for \0
	char graphLineCharData;
	char graphLineCharInterpolated;
	char graphPillarChar;
	char graphUnderlineCharData;
	char graphUnderlineCharInterpolated;
	int xAxisDescriptionMode;	//0=HH:MM	1=time difference to current time	TODO:implement 1
	int graphMode;	//0=just display logentries    1=time accurate mode, which interpolated missing entries
	int debugMode;	//0=no debug displayed	1=normal amount debug	2=verbose debug //no configfile support
	char batteriesToLog[MAX_BATTERIES * MAX_BATTERY_NAME_LENGTH + MAX_BATTERIES-1 + 1]; //batteries to display a graph for
};
extern struct settingsStruct settings;


struct logEntry
{
	short year;				//output format %hi
	unsigned char month;	//output format %hhu
	unsigned char day;
	unsigned char hour;
	unsigned char minutes;
	unsigned char percentage;
	time_t time;					//output format %li
	unsigned char interpolatedFlag;
	unsigned char empty;
};
