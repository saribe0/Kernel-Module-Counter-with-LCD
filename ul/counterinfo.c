/* Dare_Dev
 * Sambeau, Beachm, Wasimk95
 * EC 535, Spring 2016
 * Lab 5, 4/11/16
 * Source code for user level to interface with mygpio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

void printManPage(void);

int main(int argc, char **argv) {
	char line[128], *end;
	char sp, st[5], dir[5], br; 
	int pFile, N;
	int  value, period, start_hold, up_down, brightness, ii;

	// Open file and prepare for reading	
	pFile = open("/dev/mygpio", O_RDONLY);
	if (pFile < 0) {
		fputs("mygpio module isn't loaded\n",stderr);
		return -1;
	}

	// Get command line input
	if (argc == 2)
		N = strtol(argv[1], &end, 10);

	// Make sure input is valid
	if (argc == 2 && N != 0) 
	{
		// Print header
		printf("Value\tSpeed\tState\tDirection\tBrightness\n");

		// Until process is killed, read from file and print info
		while(1)
		{
			// Read from file and parse info
			read(pFile, line, 128);
			sscanf(line, "%d\n%d\n%d\n%d\n%d\n", &value, &period, &start_hold, &up_down, &brightness);
			if(period == 4000) sp = 'L';
			else if(period == 2000) sp ='M';
			else if(period == 500) sp = 'H';
			else sp = '?';
			if(start_hold) strcpy(st, "Run ");
			else strcpy(st, "Hold");
			if(!up_down) strcpy(dir, "Up  ");
			else strcpy(dir, "Down");
			if(brightness == 0) br = 'L';
			else if(brightness == 1) br ='M';
			else if(brightness == 2) br = 'H';
			else sp = '?';

			// Print info and sleep
			printf("%d\t%c\t%s\t%s\t\t%c\n", value, sp, st, dir, br);
			sleep(N);
			
			// After a few iterations, print header again
			ii++;
			if (ii == 10)
			{	
				ii = 0;
				printf("Value\tSpeed\tState\tDirection\tBrightness\n");
			}
		}
	}
	// Otherwise invalid
	else {
		printManPage();
	}

	close(pFile);
	return 0;
}

void printManPage() {
	printf("To use the counter:\n");
	printf("\tChange frequency by writing s0 for slow, s1 for medium, and s2 for fast\n");
	printf("\t\tto the /dev/mygpio file.\n");
	printf("\tChange brightness by writing b0 for dim, s1 for medium, and s2 for bright\n");
	printf("\t\tto the /dev/mygpio file.\n");
	printf("\tChange value by writing vN where N is the desired value in hex\n");
	printf("\t\tto the /dev/mygpio file.\n");
	printf("\tTo get the counter's info, run \"./counterinfo N\" where n is the time in seconds\n");
	printf("\t\tbetween counter updates. \n");
	printf("\tUse buttons to switch counter modes (Up/Down or Hold/Start) and brightness levels.\n");
}
