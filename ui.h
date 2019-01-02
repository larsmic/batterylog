//ui.h

void printLeftOffset(int parOffset);

int printInterpolatedIndicators(const struct logEntry parStruct[], int parStructSize, int parNewlineFlag);

int printTable(struct logEntry parStruct[], int parStructLenght, int parTerminalColumns);

int printLogStruct(const struct logEntry *ptr);

