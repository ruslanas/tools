#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char) (i))
#define ELF32_R_INFO(s,t) (((s)<<8) + (unsigned char)(t))

#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

# define DO_386_PC32(S, A, P)	((S) + (A) - (P))


#define STN_UNDEF 0

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_LOOS 10
#define STB_HIOS 12
#define STB_LOPROC 13
#define STB_HIPROC 15

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_SHLIB 8
#define SHT_REL 9
#define EI_NIDENT 16

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_LOPROC 13
#define STT_HIPROC 15

#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000

#define R_386_NONE 0
#define R_386_32 1   // S + A
#define R_386_PC32 2 // S + A - P

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} Elf32_Shdr;

typedef struct {
    uint32_t r_offset; // byte offset from section start to affected unit
    uint32_t r_info;
} Elf32_Rel;

typedef struct {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_shndx;
} Elf32_Sym;

Elf32_Ehdr * get_elf_header(unsigned char *);
Elf32_Shdr * get_by_ndx(uint32_t, unsigned char *);
Elf32_Rel * get_rel_at(uint32_t, unsigned char *);
unsigned char * get_strtab(unsigned char *);
Elf32_Sym * get_sym(uint32_t, Elf32_Shdr *, unsigned char *);
uint32_t get_sym_val(Elf32_Sym *, unsigned char *);
Elf32_Shdr * get_nth_by_type(int, int, unsigned char *);
int get_symbol_ndx(Elf32_Rel *);
int get_relocation_type(Elf32_Rel *);

void display_symtab(Elf32_Shdr *, unsigned char *);
void print_section_header(Elf32_Shdr *);
int sec_count(Elf32_Shdr *);
void print_sym(Elf32_Sym *, Elf32_Shdr * symtab, unsigned char *);
void dump_rel(Elf32_Rel *, Elf32_Shdr *, unsigned char *);
void relocate(Elf32_Shdr *, unsigned char *);
unsigned char * symbol_name(Elf32_Sym *, Elf32_Shdr *, unsigned char *);
uint32_t get_symbol_value(Elf32_Sym *, Elf32_Rel *, unsigned char *);
