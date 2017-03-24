/*
 * hx - Command line hex viewer
 *
 * Usage examples: hx -n0 file.exe
 *
 * @author Ruslanas Balciunas <ruslanas.com@gmail.com>
 * (c) 2015
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define READ_BUFF_SIZE 1024

int main(int argc, char* argv[]) {

	unsigned long num_lines = 31, c, page = 0;

	opterr = 0;
	char *end;
	long ln = 0;
	while((c=getopt(argc, argv, "l:p:n:"))!= -1) {
		switch(c) {
			case 'n':
				num_lines = strtol(optarg, &end, 10) - 1;
				break;
			case 'p':
				page = strtol(optarg, &end, 10);
				break;
			case 'l':
				// hx -l $((0x0A+0x0B)) file
				if(strstr(optarg, "0x") == NULL) {
					ln = strtol(optarg, &end, 10);
				} else {
					ln = strtol(optarg, &end, 16);
				}
				page = floor(ln / (32 * 16));
				break;

			case '?':
				if(optopt == 'n' || optopt == 'l')
					printf("Argument required for `-%c'\n", optopt);
				else
					printf("Unknown option `-%c'\n", optopt);
				return 1;
		}
	}


	FILE* fp;
	if(optind >= argc) {
		printf("Usage: hx [-n lines] [-p page] [-l address] <file>\n");
		return 1;
	}
	fp = fopen(argv[optind], "rb");

	if(!fp) {
		switch(errno) {
			case EACCES:
				printf("Permission denied.\n");
				break;
			default:
				printf("Could not read file `%s' [errno: %d]\n", argv[optind], errno);
		}
		return 1;
	}

	fseek(fp, page * 32 * 16, SEEK_SET);
	unsigned char buff[READ_BUFF_SIZE];

	unsigned long line = 0, k = 0, len;
	char str[16] = {'\0'};

	printf("File: %s\nPage: %ld\n\n", argv[optind], page);

	while(((len = fread(&buff, 1, sizeof(buff), fp)) > 0)
		&& ((num_lines == 0) || (line < num_lines))) {

		int i;

		for(i=0;i<len;i++) {

			if(i % 16 == 0) {
				printf("%08lx: ", (page * 32 + line) * 16);
			}

			unsigned char cc = buff[i];
			printf("%02X ", cc);

			if(isprint(cc)) {
				str[k % 16] = cc;

			} else {
				str[k % 16] = '.';
			}
			str[(k % 16) + 1] = '\0';
			k++;

			if((i+1) % 4 == 0) {
				printf(" ");
			}

			if((i+1) % 16 == 0) {
				line++;
				printf("%s\n", str);
				if(line % 4 == 0) {
					printf("\n");
				}
				if(line > num_lines) {
					break;
				}
			}
		}
	}

	len = strlen(str);
    if(len < 16) {
		int shift = 52 - len * 3 - floor(len / 4);
		while(shift > 0) {
			printf(" ");
			shift--;
		}
		printf("%s\n\n", str);
	}
	fclose(fp);
	return 0;
}
