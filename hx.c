/*
 * hx
 *
 * @author Ruslanas Balciunas <ruslanas.com@gmail.com>
 * (c) 2015
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char* argv[]) {

	if(argc < 2) {
		printf("Usage: hx <file> [-n lines]\n");
		return 0;
	}

	long num_lines = 31, c;

	opterr = 0;
	char *end;
	while((c=getopt(argc, argv, "n:"))!= -1) {
		switch(c) {
			case 'n':
				num_lines = strtol(optarg, &end, 10) - 1;
				break;

			case '?':
				if(optopt == 'n')
					printf("Argument required for -n\n");
				else 
					printf("Unknown option `-%c'\n", optopt);
				return 1;
		}
	}


	FILE* fp;
	fp = fopen(argv[optind], "rb");

	if(!fp) {
		printf("Could not read file %s\n", argv[optind]);
		return 1;
	}

	unsigned char buff[1024];

	int line = 0, k = 0, len;
	char str[16];

	while(((len = fread(&buff, 1, sizeof(buff), fp)) > 0) && (line < num_lines)) {
		int i;

		for(i=0;i<len;i++) {
			
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
		printf("%s", str);
	}
	fclose(fp);
	return 0;
}
