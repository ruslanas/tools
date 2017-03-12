#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "elf.h"

#define CMD_SYMTAB 1
#define CMD_HEADER 2
#define CMD_RELOCATE 4
#define CMD_REL 8
#define STT_LENGTH 16
unsigned char * strtab;

unsigned char symtypes[][STT_LENGTH] = {
    "NOTYPE",
    "OBJECT",
    "FUNC",
    "SECTION",
    "FILE"
};

int main(int argc, char * argv[]) {
    unsigned long c;

    int cmd = 0;
    while((c=getopt(argc, argv, "srhx")) != -1) {
        switch(c) {
            case 'h':
                cmd |= CMD_HEADER;
                break;
            case 's':
                cmd |= CMD_SYMTAB;
                break;
            case 'r':
                cmd |= CMD_RELOCATE;
                break;
            case 'x':
                cmd |= CMD_REL;
        }
    }

    if(optind >= argc) {
        printf("Usage: [-s] [-r] [-h] <infile> [<outfile>]\n");
        exit(-1);
    }

    printf("%s\n", argv[optind]);
    FILE * fp = fopen(argv[optind], "rb");
    if(!fp) {
        printf("Error opening file!\n");
        exit(-1);
    }

    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    unsigned char data[sz];
    printf("File size: %ld\n", sz);
    rewind(fp);

    if(fread(&data, 1, sizeof(data), fp) != sz) {
        printf("%s\n", argv[optind]);
        exit(-1);
    }

    fclose(fp);

    Elf32_Ehdr * hdr = (Elf32_Ehdr *)data;
    if(hdr->e_ident[0] != 0x7f) {
        printf("Not ELF\n");
        exit(-1);
    }

    if(hdr->e_ident[4] != 1) {
        printf("Only 32 bit ELF supported!\n");
        exit(-1);
    }

    if(cmd & CMD_REL) {
        int i = 0;
        Elf32_Shdr * reltab;
        while((reltab = get_nth_by_type(i, SHT_REL, data)) != NULL) {
            print_section_header(reltab);
            printf("Offset\t\tType\tSymbol\tValue\t\tName\n");
            Elf32_Shdr * symtab = get_by_ndx(reltab->sh_link, data);
            for(int j=0;j<reltab->sh_size / reltab->sh_entsize;j++) {
                Elf32_Rel * rel = (Elf32_Rel *)(data + reltab->sh_offset) + j;
                dump_rel(rel, symtab, data);
            }
            i++;
        }
    }

    if(cmd & CMD_SYMTAB) {
        Elf32_Shdr * symtab;
        int i=0;
        while((symtab = get_nth_by_type(i, SHT_SYMTAB, data)) != NULL) {
            print_section_header(symtab);
            display_symtab(symtab, data);
            i++;
        }
    }

    if(cmd & CMD_HEADER) {
        printf("Arch:\t\t\t\t%d (%s)\n", hdr->e_ident[4], hdr->e_ident[4] == 1 ? "32bit" : "Other");
        printf("Section headers table offset:\t0x%08x\n", hdr->e_shoff);
        printf("String table index:\t\t%d\n", hdr->e_shstrndx);
        printf("Number of sections:\t\t%d\n", hdr->e_shnum);
        printf("Type:\t\t\t\t%d (%s)\n", hdr->e_type, (hdr->e_type == 1) ? "Relocatable" : "Other");
        printf("Entry:\t\t\t\t%08x\n", hdr->e_entry);

    }

    if(cmd & CMD_RELOCATE) {

        if(hdr->e_type != 1) {
            printf("Not relocatable!\n");
            exit(-1);
        }

        Elf32_Shdr * reltab = get_by_ndx(0, data);
        for(int i=0;i<hdr->e_shnum;i++) {
            if(reltab->sh_type == SHT_REL) {
                relocate(reltab, data);
            }
            reltab++;
        }

        hdr->e_type = 2;
        if(optind + 1 < argc) {
            fp = fopen(argv[argc - 1], "wb");
            fwrite(data, 1, sizeof(data), fp);
            fclose(fp);
            printf("Done.\n");
        } else {
            printf("No output file specified!\n");
        }
    }

    return 0;
}

Elf32_Ehdr * get_elf_header(unsigned char * data) {
    return (Elf32_Ehdr *)data;
}

void dump_rel(Elf32_Rel * rel, Elf32_Shdr * symtab, unsigned char * data) {
    int ndx = get_symbol_ndx(rel);
    Elf32_Sym * sym = get_sym(ndx, symtab, data);
    unsigned char * name = symbol_name(sym, symtab, data);
    printf("%08x\t%d\t%d\t%08x\t%s\n",
        rel->r_offset, get_relocation_type(rel), ndx, sym->st_value, name);
}

Elf32_Shdr * get_by_ndx(uint32_t ndx, unsigned char * data) {
    return (Elf32_Shdr *)(data + get_elf_header(data)->e_shoff) + ndx;
}

Elf32_Rel * get_rel_at(uint32_t offset, unsigned char * data) {
    return (Elf32_Rel *)(data + offset);
}

void display_symtab(Elf32_Shdr * symtab, unsigned char * data) {
    int count = symtab->sh_size / symtab->sh_entsize;

    printf("Num\tType\tValue\t\tSize\tOther\tIndex\tName\n");

    // skip local
    for(int ndx=symtab->sh_info;ndx<count;ndx++) {
        Elf32_Sym * sym = get_sym(ndx, symtab, data);
        int tndx = ELF32_ST_TYPE(sym->st_info);
        unsigned char * typ = (tndx > 4) ? (unsigned char *)"?" : symtypes[tndx];
        printf("%d:\t%s\t%08x\t%d\t%d\t%d\t%s\n",
            ndx, typ, sym->st_value, sym->st_size, sym->st_other, sym->st_shndx, symbol_name(sym, symtab, data));
    }
}

int sec_count(Elf32_Shdr * shdr) {
    if(shdr->sh_entsize == 0) {
        return 0; // undefined
    }
    return shdr->sh_size / shdr->sh_entsize;
}

void print_section_header(Elf32_Shdr * shdr) {
    int count = sec_count(shdr);
    printf("\nSection count:\t%d\n", count);
    printf("Type:\t\t%d\n", shdr->sh_type);
    printf("Offset:\t\t%08x\n", shdr->sh_offset);
    printf("Size:\t\t%08x\n", shdr->sh_size);
    printf("Address:\t%08x\n", shdr->sh_addr);
    printf("Link:\t\t%08x\n", shdr->sh_link);
    printf("Info:\t\t%08x\n\n", shdr->sh_info);
}

Elf32_Shdr * get_target(Elf32_Shdr * reltab, unsigned char * data) {
    return get_by_ndx(reltab->sh_info, data);
}

void relocation_header(Elf32_Shdr * reltab, Elf32_Shdr * target) {
    printf("Relocation target: NDX%d, @%08x (%d)\n", reltab->sh_link, target->sh_addr, target->sh_type);
}

Elf32_Shdr * get_symtab(Elf32_Shdr * reltab, unsigned char * data) {
    Elf32_Shdr * symtab = get_by_ndx(reltab->sh_link, data);
    if(symtab->sh_type != SHT_SYMTAB) {
        printf("Not a symbol table!");
        exit(-1);
    }
    return symtab;
}
Elf32_Rel * get_first_relocation(Elf32_Shdr * reltab, unsigned char * data) {
    if(reltab->sh_type != SHT_REL) {
        printf("Not a reloction table!");
        exit(-1);
    }
    return get_rel_at(reltab->sh_offset, data);
}
int get_relocation_type(Elf32_Rel * rel) {
    return ELF32_R_TYPE(rel->r_info);
}
int get_symbol_ndx(Elf32_Rel * rel) {
    return ELF32_R_SYM(rel->r_info);
}
void relocate(Elf32_Shdr * reltab, unsigned char * data) {
    if((reltab == NULL) || reltab->sh_type != SHT_REL) {
        printf("Not a relocation table!\n");
        exit(-1);
    }
    Elf32_Shdr * target = get_target(reltab, data);
    relocation_header(reltab, target);
    Elf32_Shdr * symtab = get_symtab(reltab, data);
    Elf32_Rel * rel = get_first_relocation(reltab, data);
    int count = sec_count(reltab);
    for(int j=0;j<count;j++) {

        if(get_symbol_ndx(rel) == SHN_UNDEF) {
            printf("Relocation error\n");
            exit(-1);
        }

        uint32_t * ref = (uint32_t *)(data + target->sh_offset + rel->r_offset);
        int old = *ref;

        Elf32_Sym * sym = get_sym(get_symbol_ndx(rel), symtab, data);
        Elf32_Shdr * section = get_by_ndx(sym->st_shndx, data);

        print_sym(sym, symtab, data);

        uint32_t val = get_symbol_value(sym, rel, data);

        switch(get_relocation_type(rel)) {

            case R_386_NONE:
                break;

            case R_386_32:
                *ref = section->sh_addr + sym->st_value;
                break;

            case R_386_PC32:
                *ref = section->sh_addr + sym->st_value;
                break;

            default:
                printf("Unsupported relocation type!\n");
                exit(0);
        }

        printf("%d [A(%08x) + S(%08x) - P(%08x) = %08x (%s)\n\n",
            get_relocation_type(rel), old, sym->st_value, rel->r_offset, *ref, symbol_name(sym, symtab, data));

        rel++;
    }


}

uint32_t get_symbol_value(Elf32_Sym * symbol, Elf32_Rel * rel, unsigned char *data) {
    return get_sym_val(symbol, data);
}

uint32_t get_sym_val(Elf32_Sym * sym, unsigned char * data) {
    if(sym->st_shndx == 0) {
        return 0;
    }
    return data[get_by_ndx(sym->st_shndx, data)->sh_offset + sym->st_value];
}

unsigned char * symbol_name(Elf32_Sym * symbol, Elf32_Shdr * symtab, unsigned char * data) {
    Elf32_Shdr * strtab = get_by_ndx(symtab->sh_link, data);
    if((strtab == NULL) || strtab->sh_type != SHT_STRTAB) {
        printf("Talbe string not found!\n");
        exit(-1);
    }
    return (data + strtab->sh_offset) + symbol->st_name;
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

Elf32_Sym * get_sym(uint32_t ndx, Elf32_Shdr * symtab, unsigned char * data) {
    if(symtab->sh_type != SHT_SYMTAB) {
        printf("Not a symbol table");
        exit(-1);
    }
    if(ndx >= sec_count(symtab)) {
        printf("Out or bounds!\n");
        exit(-1);
    }
    return (Elf32_Sym *)(data + symtab->sh_offset) + ndx;
}

void print_sym(Elf32_Sym * sym, Elf32_Shdr * symtab, unsigned char * data) {
    printf("Symbol:\t\t\t%s\n", symbol_name(sym, symtab, data));
    printf("Value:\t\t\t%08x (%08x)\n", sym->st_value, get_sym_val(sym, data));
    printf("Other: \t\t\t%d\n", sym->st_other);
    printf("Relevant section index:\t%08x\n", sym->st_shndx);
    printf("Binding:\t\t%d\n", ELF32_ST_BIND(sym->st_info));
    printf("Type:\t\t\t%d\n", ELF32_ST_TYPE(sym->st_info));
}
