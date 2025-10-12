#include <stdlib.h>
#include <string.h>
#include "TimeParser.h"

// time format: HHMMSS (6 characters)
int time_parse(char *time) {

	// how many seconds, default returns error
	int seconds = TIME_LEN_ERROR;

	// Check that string is not null
	if(time == nullptr) {
		return TIME_ARRAY_ERROR;
	}
	// Parse values from time string
	// For example: 124033 -> 12hour 40min 33sec
    int values[3];
    char hour[3] = {0}, min[3] = {0}, sec[3] = {0};
    strncpy(hour, time, 2);
    strncpy(min, time+2, 2);
    strncpy(sec, time+4, 2);
    values[0] = atoi(hour); 	// values[0] hour
    values[1] = atoi(min);		// values[1] minute
    values[2] = atoi(sec);		// values[2] second

	// Add boundary check time values: below zero or above limit not allowed
	// limits are 59 for minutes, 23 for hours, etc
	if (values[0] < 0 || values[0] > 23 || values[1] < 0 || values[1] > 59 || values[2] < 0 || values[2] > 59)
	{
		return TIME_VALUE_ERROR;
	}

	// String lenght check
	if (strlen(time) != 6) {
		return TIME_LEN_ERROR;
	}

	// Calculate return value from the parsed minutes and seconds
	seconds = values[0] * 3600 + values[1] * 60 + values[2];
	
	return seconds;
}
