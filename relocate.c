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

    Elf32_Shdr * shdr = get_by_ndx(0, data);
    int count;
    unsigned char * str;
    for(int i=0;i<hdr->e_shnum;i++) {
        switch(shdr->sh_type) {

            case SHT_REL:
                count = shdr->sh_size / shdr->sh_entsize;
                printf("%d %d %08x %08x (%08x) -> %08x [%08x]\n",
                    count, shdr->sh_type, shdr->sh_offset, shdr->sh_size, shdr->sh_link, shdr->sh_addr, shdr->sh_info);

                Elf32_Shdr * appl = get_by_ndx(shdr->sh_info, data);
                printf("Apply reloction to section type: %d\n", appl->sh_type);
                display_symtab(get_by_ndx(shdr->sh_link, data), data);

                printf("Offset\t\tType\tSymbol\n");
                Elf32_Rel * rel = get_rel_at(shdr->sh_offset, data);
                for(int j=0;j<count;j++) {
                    printf("%08x\t%d\t%d\n", rel->r_offset, ELF32_R_TYPE(rel->r_info), ELF32_R_SYM(rel->r_info));
                    rel++;
                }
                break;

            case SHT_STRTAB:
                str = (unsigned char *)&data[shdr->sh_offset+1];
                printf("%s\n", str);
                break;
        }
        shdr++;
    }

    return 0;
}
Elf32_Shdr * get_by_ndx(uint32_t ndx, unsigned char * data) {
    Elf32_Ehdr * hdr = (Elf32_Ehdr *)data;
    return (Elf32_Shdr *)&data[hdr->e_shoff] + ndx;
}
Elf32_Rel * get_rel_at(uint32_t offset, unsigned char * data) {
    return (Elf32_Rel *)&data[offset];
}
void display_symtab(Elf32_Shdr * shdr, unsigned char * data) {
    int count = shdr->sh_size / shdr->sh_entsize;
    printf("%d %d %08x %08x (%08x) -> %08x\n",
        count, shdr->sh_type, shdr->sh_offset, shdr->sh_size, shdr->sh_link, shdr->sh_addr);

    Elf32_Sym * sym = (Elf32_Sym *)&data[shdr->sh_offset];
    printf("Num\tName\tValue\t\tSize\tOther\tIndex\n");
    for(int j=0;j<count;j++) {
        printf("%d:\t%d\t%08x\t%d\t%d\t%d\n",
            j, sym->st_name, sym->st_value, sym->st_size, sym->st_other, sym->st_shndx);
        sym++;
    }
}
