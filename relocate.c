#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "elf.h"

unsigned char * strtab;

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
    Elf32_Shdr * shdr = get_by_ndx(0, data);
    int count;

    printf("Section headers table offset: 0x%08x\n", hdr->e_shoff);
    printf("String table index: %d\n", hdr->e_shstrndx);
    printf("Number of sections: %d\n", hdr->e_shnum);
    printf("EI_CLASS: %s\n", hdr->e_ident[4] == 1 ? "32bit" : "64bit");
    if(hdr->e_ident[4] != 1) {
        // not ready yet for 64
        exit(0);
    }

    for(int i=0;i<hdr->e_shnum;i++) {

        switch(shdr->sh_type) {

            case SHT_REL:

                print_section_header(shdr);

                Elf32_Shdr * target = get_by_ndx(shdr->sh_info, data);
                display_symtab(get_by_ndx(shdr->sh_link, data), data);

                count = count_rels(shdr);
                Elf32_Rel * rel = get_rel_at(shdr->sh_offset, data);

                for(int j=0;j<count;j++) {
                    printf("Apply relocation to section (%d) %08x\n", target->sh_type, target->sh_addr);
                    relocate(target, rel, get_by_ndx(shdr->sh_link, data), data);
                    rel++;
                }
                break;

            default:
                printf("SHDR: Addr:%08x Type:%d, Flags:%d\n",
                    shdr->sh_addr, shdr->sh_type, shdr->sh_flags);
        }
        shdr++;
    }

    if(argc > 2) {
        fp = fopen(argv[2], "wb");
        fwrite(data, 1, sizeof(data), fp);
        fclose(fp);
    }

    return 0;
}

Elf32_Ehdr * get_elf_header(unsigned char * data) {
    return (Elf32_Ehdr *)data;
}

Elf32_Shdr * get_by_ndx(uint32_t ndx, unsigned char * data) {
    Elf32_Ehdr * hdr = get_elf_header(data);
    return (Elf32_Shdr *)(data + hdr->e_shoff) + ndx;
}
Elf32_Rel * get_rel_at(uint32_t offset, unsigned char * data) {
    return (Elf32_Rel *)(data + offset);
}
void display_symtab(Elf32_Shdr * shdr, unsigned char * data) {
    int count = shdr->sh_size / shdr->sh_entsize;
    printf("%d %d %08x %08x (%08x) -> %08x\n",
        count, shdr->sh_type, shdr->sh_offset, shdr->sh_size, shdr->sh_link, shdr->sh_addr);

    Elf32_Sym * sym = (Elf32_Sym *)(data + shdr->sh_offset);
    printf("Num\tName\t\tValue\t\tSize\tOther\tIndex\n");
    for(int j=0;j<count;j++) {
        printf("%d:\t%s\t\t%08x\t%d\t%d\t%d\n",
            j, get_sym_name(sym, data), sym->st_value, sym->st_size, sym->st_other, sym->st_shndx);
        sym++;
    }
}
int count_rels(Elf32_Shdr * shdr) {
    if(shdr->sh_entsize == 0) {
        return 0;
    }
    return shdr->sh_size / shdr->sh_entsize;
}

void print_section_header(Elf32_Shdr * shdr) {
    int count = count_rels(shdr);
    printf("Count:%d Type:%d Offset:%08x Size:%08x (Link:%08x) -> Addr:%08x [Info:%08x]\n",
        count, shdr->sh_type, shdr->sh_offset, shdr->sh_size, shdr->sh_link, shdr->sh_addr, shdr->sh_info);
}

void relocate(Elf32_Shdr * target, Elf32_Rel * rel, Elf32_Shdr * symtab, unsigned char * data) {

    int * ref = (int *)&data + target->sh_offset + rel->r_offset;
    Elf32_Sym * sym = get_sym(ELF32_R_SYM(rel->r_info), symtab, data);
    Elf32_Shdr * section = get_by_ndx(sym->st_shndx, data);

    print_section_header(section);
    print_sym(sym, data);

    switch(ELF32_R_TYPE(rel->r_info)) {
        case R_386_PC32:
            printf(": %08x\n\n", get_sym_val(sym, data));
            break;

        case R_386_32:
            // bar = data[section->sh_offset + rel->r_offset];
            // printf("=:%08x\n\n", bar);
            break;
    }

}

int get_sym_val(Elf32_Sym * sym, unsigned char * data) {
    return data[get_by_ndx(sym->st_shndx, data)->sh_offset + sym->st_value];
}

unsigned char * get_sym_name(Elf32_Sym * sym, unsigned char * data) {
    Elf32_Shdr * sh = get_nth_by_type(1, SHT_STRTAB, data);

    if(sh != NULL) {
        return data + sh->sh_offset + sym->st_name;
    }

    return malloc(sizeof(unsigned char));
}

Elf32_Shdr * get_nth_by_type(int ndx, int t, unsigned char * data) {
    Elf32_Ehdr * hdr = get_elf_header(data);
    Elf32_Shdr * shdr = get_by_ndx(0, data);
    int n = 0;
    for(int i=0;i<hdr->e_shnum;i++) {
        if(shdr->sh_type == t) {
            if(n == ndx) {
                return shdr;
            }
            n++;
        }
        shdr++;
    }
    return NULL;
}

unsigned char * get_strtab(unsigned char * data) {
    return (unsigned char *)(data + get_by_ndx(get_elf_header(data)->e_shstrndx, data)->sh_offset);
}

Elf32_Sym * get_sym(uint32_t offset, Elf32_Shdr * symtab, unsigned char * data) {
    Elf32_Sym * sym = (Elf32_Sym *)(data + symtab->sh_offset) + offset;
    return sym;
}

void print_sym(Elf32_Sym * sym, unsigned char * data) {
    if(sym->st_name == 0) {
        return;
    }
    printf("sym->st_name: %08x\n", sym->st_name);
    printf("sym->st_value: %08x\n", sym->st_value);
    printf("sym->st_other: %08x\n", sym->st_other);
    printf("sym->st_shndx: %08x\n", sym->st_shndx);
    printf("Sym binding: %d\n", ELF32_ST_BIND(sym->st_info));
    printf("ELF32_ST_TYPE: %d\n", ELF32_ST_TYPE(sym->st_info));
    printf("Value :%08x\n", get_sym_val(sym, data));
    printf("Name: %s\n", get_sym_name(sym, data));
}
