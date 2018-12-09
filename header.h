
/*
 *
 *	header.h
 *
 */

/* DEFINES */
#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof(a[0] ))
#define LOGFILE_ENTRY_LENGHT 30
#define CHART_CHAR '#'
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* GLOBALS */
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
};

struct settingsStruct
{
	/*
	* Specifications:
	* - maximal length of varnames of settings is 100 chars without '\0'
	* - maximal length of values is 4096 without '\0'
	*
	* If you add an option to the settings struct, also include suppurt for it in:
	* - the updateSettingsValue function (if config file suppurt is logical)
	* - the global definition of default values
	* If config file support is not logical, please note it as comment at the definition of the variable.
	*/
	char batteryPercentagelogfilePath[4097];	//maximal characters allowed for filepaths in ext4, plus one for \0
	char graphLineCharData;
	char graphLineCharInterpolated;
	char graphPillarChar;
	char graphUnderlineCharData;
	char graphUnderlineCharInterpolated;
	int xAxisDescriptionMode;	//0=HH:MM	1=time difference to current time	TODO:implement 1
	int graphMode;	//0=just display logentries    1=time accurate mode, which interpolated missing entries
	int debugMode;	//0=no debug displayed	1=normal amount debug	2=verbose debug
};

extern struct settingsStruct settings;


/* FUNCTION DECLARATIONS */

//ui.c
int printLine(int parTerminalColumns, const struct logEntry parStruct[], int parStructSize, int parInterpolationFlag);

void printLeftOffset(int parOffset);

int printMoreThanTenMinutesDifferenceIndicators(struct logEntry parStruct[], int parStructLenght, int parLeftOffset);

int printInterpolatedIndicators(const struct logEntry parStruct[], int parStructSize, int parNewlineFlag);

int printTable(struct logEntry parStruct[], int parStructLenght, int parTerminalColumns);

int printLogStruct(const struct logEntry *ptr);


//settings.c
int updateSettingsValue(char *parName, char *parValue);

int readSettingsFromConfigFile(void);


//main.c
char *removeEndingNewline(char *string_pointer);

int copyLogfileToArray(char parArray[][LOGFILE_ENTRY_LENGHT], int const parArraySize);

int getTerminalColumns(void);

int logfileArrayToStruct(struct logEntry parStruct[], char parArray[][LOGFILE_ENTRY_LENGHT], int parLenght);

int generateTimeAccurateStruct(struct logEntry parTimeAccurateStruct[], struct logEntry parInputStruct[], const int parStructSize);

