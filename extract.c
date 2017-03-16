#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAX_LENGTH 1024

int isAlphaNum(char);
int isSpace(char);
int end(char);

int main(int argc, char * argv[]) {
    unsigned char map[MAX_LENGTH] = "";
    unsigned long c;
    while((c=getopt(argc, argv, "m:"))!=-1) {
        switch(c) {
            case 'm':
                strcpy((char * restrict)map, optarg);
                printf("%s\n", map);
                break;
        }
    }

    if(strlen((const char *)map) == 0) {
        printf("Usage: extract -m <mapfile> <image>\n");
        exit(-1);
    }

    FILE * fp = fopen((const char *)map, "rb");
    if(!fp) {
        printf("Error opening file [errno: %d]\n", errno);
        exit(-1);
    }
    unsigned char buff[MAX_LENGTH];
    unsigned long len;
    len = fread(&buff, 1, sizeof(buff), fp);
    if(!len) {
        printf("Malformed memory map file\n");
        exit(-1);
    } 
    fclose(fp);

    unsigned char tbl[1024][32];
    int j = 0, p = 0, b = 0;
    for(int i=0;(!end(buff[i]) && (i<MAX_LENGTH));i++) {
        if((!isAlphaNum(buff[i]))
            || (isAlphaNum(buff[i]) && b == 0)) {

            if(isAlphaNum(buff[i])) {
                b = 1;
            } else {
                b = 0;
            }
            // ltrim
            while(isSpace(buff[p]) && (p<MAX_LENGTH)) {
                p++;
            }
            if(p>=i) {
                continue;
            }
            int l = i-p;
            // rtrim
            while(isSpace(buff[p+l-1]) && (l > 0)) {
                l--;
            }
            if(l <= 0) {
                continue;
            }
            strncpy((char *)tbl[j], (char *)buff + p, l);
            tbl[j][l] = '\0';
            j++;
            p = i;
        }
    }

    for(int i=0;i<j;i++) {
        printf("%s\n", tbl[i]);
    }
}

int isSpace(char c) {
    return (c < 33);
}

int isAlphaNum(char c) {
    return ((c > 47 && c < 58)
        || (c > 64 && c < 91)
        || (c > 96 && c < 123) || c == '_');
}

int end(char c) {
    return (c == '\0');
}