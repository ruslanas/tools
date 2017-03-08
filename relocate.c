#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "elf.h"

int main(int argc, char * argv[]) {
    if(argc < 2) {
        return 1;
    }
    printf("%s\n", argv[1]);
    FILE * fp = fopen(argv[1], "rb");
    if(!fp) {
        printf("Error opening file\n");
        return 1;
    }

    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    printf("File size: %ld\n", sz);
    rewind(fp);

    unsigned char data[sz];
    if(fread(&data, 1, sizeof(data), fp) != sz) {
        return 1;
    }
    fclose(fp);

    Elf32_Ehdr * hdr = (Elf32_Ehdr *)data;

    printf("Section headers table offset: 0x%08x\n", hdr->e_shoff);

    Elf32_Shdr * shdr = (Elf32_Shdr *)&data[hdr->e_shoff];
    for(int i=0;i<hdr->e_shnum;i++) {
        if(shdr->sh_type == SHT_REL) {
            printf("%d %08x %08x (%08x) -> %08x\n",
                shdr->sh_size / shdr->sh_entsize, shdr->sh_offset, shdr->sh_size, shdr->sh_link, shdr->sh_addr);
        }
        shdr++;
    }

    return 0;
}
