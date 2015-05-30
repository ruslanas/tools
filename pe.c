/*
PE analyzer

@author Ruslanas Balciunas <ruslanas.com@gmail.com>
(c) 2015
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "types.h"

const char *subsystem[] = {
    "n/a",
    "native",
    "Windows/GUI",
    "Windows non-GUI",
    "n/a",
    "OS/2",
    "n/a",
    "POSIX"
};


#define MAX_SYMBOL_LENGTH 256

uint16_t print_import_address_table(FILE* ptr_file) {
    uint16_t hint;
    fread(&hint, sizeof(uint16_t), 1, ptr_file);
    char buff[MAX_SYMBOL_LENGTH];
    int i = 0;
    char c = 'A';

    do {
        c = fgetc(ptr_file);
        buff[i] = c;
        i++;
    } while(c != '\0' && i < sizeof(buff));

    if(i % 2 != 0) {
        fgetc(ptr_file);
    }
    printf("0x%08x %s\n", hint, buff);
    return hint;
}

int main(int argc, char* argv[]) {


    // file name must be provided
    if(argc < 2) {
        printf("%s\n", "Usage: pe <file>");
        return 1;
    }

    FILE *ptr_file; // pointer to file structure

    // open file for writing
    ptr_file = fopen(argv[1], "rb");

    if(!ptr_file) {
        printf("%s\n", "Could not open file");
        return 1;
    }


    SYSTEM_INFO si;
    GetSystemInfo(&si);
    printf("Memory page size: %ld\n", si.dwPageSize);
    printf("Processor type: %ld\n", si.dwProcessorType);
    printf("Processors: %ld\n", si.dwNumberOfProcessors);

    printf("\nDOS header\n");
    printf("---------------------------------------------------------\n");

    IMAGE_DOS_HEADER header;

    fread(&header, sizeof(struct _IMAGE_DOS_HEADER), 1, ptr_file);

    if(header.e_magic != 0x5a4d) {
        printf("%s", "Not MZ\n");
        return 1;
    }

    printf("Magic:\t\t\t\t 0x%04x\n", header.e_magic);
    printf("Bytes on last page:\t\t %d\n", header.e_cblp);
    printf("Pages:\t\t\t\t %d\n", header.e_cp);
    printf("Relocations:\t\t\t %d\n", header.e_crlc);
    printf("Size of header in paragraphs:\t %d\n", header.e_cparhdr);

    // ...

    printf("Initial SP:\t\t\t 0x%08x\n", header.e_sp);
    printf("Initial CS:\t\t\t 0x%08x\n", header.e_cs);

    printf("File address of new EXE header:\t %ld\n", header.e_lfanew);


    fseek(ptr_file, header.e_lfanew, SEEK_SET);


    printf("\nPE header\n");
    printf("---------------------------------------------------------\n");

    COFFHeader coff;

    fread(&coff, sizeof(coff), 1, ptr_file);

    printf("Signature:\t\t\t %s\n", coff.Signature);

    if(coff.Machine == 0x14c) {
        printf("Machine:\t\t\t Intel 386\n");
    } else {
        printf("Machine:\t\t\t 0x%04x\n", coff.Machine);
    }

    printf("Number of sections:\t\t %hu\n", coff.NumberOfSections);
    printf("TimeStamp:\t\t\t %ld\n", coff.TimeDateStamp);

    printf("Size of optional header:\t %hu\n", coff.SizeOfOptionalHeader);
    printf("Characteristics:\t\t 0x%04x\n", coff.Characteristics);


    printf("\nOptional header (standard fields)\n");
    printf("---------------------------------------------------------\n");

    PEOptionsHeader options;
    fread(&options, sizeof(options), 1, ptr_file);

    char* peFormat;

    if(options.signature == 0x10b) {
        peFormat = "PE32";
    } else {
        peFormat = "PE32+";
    }

    printf("Signature:\t\t\t %s\n", peFormat);
    printf("Address of entry point:\t\t 0x%lx\n", options.AddressOfEntryPoint);
    printf("Base of code:\t\t\t %ld\n", options.BaseOfCode);


    printf("\nOptional header (Windows-specific)\n");
    printf("---------------------------------------------------------\n");

    WindowsOptions windowsOptions;

    if(options.signature == 0x20b) {
        // PE32+
        PEOptionsHeaderWindowsPlus pe32Plus;
        fread(&pe32Plus, sizeof(PEOptionsHeaderWindowsPlus), 1, ptr_file);

        windowsOptions.OsVersion = pe32Plus.MajorOSVersion;
        windowsOptions.NumberOfRvaAndSizes = pe32Plus.NumberOfRvaAndSizes;
        windowsOptions.SectionAlignment = pe32Plus.SectionAlignment;
        windowsOptions.FileAlignment = pe32Plus.FileAlignment;
        // quick
        windowsOptions.ImageBase = (long)pe32Plus.ImageBase;
    } else {
        // PE32
        PEOptionsHeaderWindows pe32;
        fread(&pe32, sizeof(PEOptionsHeaderWindows), 1, ptr_file);

        windowsOptions.OsVersion = pe32.MajorOSVersion;
        windowsOptions.NumberOfRvaAndSizes = pe32.NumberOfRvaAndSizes;
        windowsOptions.SectionAlignment = pe32.SectionAlignment;
        windowsOptions.FileAlignment = pe32.FileAlignment;
        windowsOptions.ImageBase = pe32.ImageBase;
    }

    printf("Major OS version:\t\t %hu\n", windowsOptions.OsVersion);
    printf("Section alignment:\t\t 0x%08lx\n", windowsOptions.SectionAlignment);
    printf("File alignment:\t\t\t 0x%08lx\n", windowsOptions.FileAlignment);
    printf("Image base:\t\t\t 0x%08lx\n", windowsOptions.ImageBase);
    printf("Number of data directories:\t %ld\n", windowsOptions.NumberOfRvaAndSizes);


    printf("\nData directories\n");
    printf("---------------------------------------------------------\n");
    DataDirectory *directory = malloc(windowsOptions.NumberOfRvaAndSizes * sizeof(DataDirectory));
    fread(directory, sizeof(DataDirectory), windowsOptions.NumberOfRvaAndSizes, ptr_file);
    int i;
    printf("VA\t\tSize\n");
    printf("---------------------------------------------------------\n");

    for(i=0;i<windowsOptions.NumberOfRvaAndSizes;i++) {
        printf("0x%08lx\t%ld\n", directory[i].VirtualAddress, directory[i].Size);
    }

    printf("\nSections table\n");
    printf("---------------------------------");
    printf("---------------------------------\n");

    // allocate memory for NumberOfSections
    SectionDescriptor *descriptor = malloc(coff.NumberOfSections * sizeof(SectionDescriptor));

    fread(descriptor, sizeof(SectionDescriptor), coff.NumberOfSections, ptr_file);

    printf("Name\tSize\tVA\t\tRaw\tLocation\tFlags\n");
    printf("---------------------------------");
    printf("---------------------------------\n");

    long location = 0;



    for(i=0;i<coff.NumberOfSections;i++) {

        if(strcmp(descriptor[i].Name, ".idata") == 0) {
            location = descriptor[i].PhysicalLocation;
        }

        printf("%s\t%ld\t0x%08lx\t%ld\t0x%08lx\t0x%lx\n",
            descriptor[i].Name,
            descriptor[i].SizeInMemory,
            descriptor[i].LocationInMemory,
            descriptor[i].PhysicalSize,
            descriptor[i].PhysicalLocation,
            descriptor[i].Flags);
    }

    int importTables = 0;
    if(location != 0) {
        printf("\n.idata RAW location: 0x%08lx\n\n", location);
        fseek(ptr_file, location, SEEK_SET);
        ImportDirectoryTableEntry idtEntry;
        do {
            fread(&idtEntry, sizeof(idtEntry), 1, ptr_file);
            printf("Lookup Table RVA: 0x%08lx\n", idtEntry.UNION.LookupTableRva);
            printf("Time stamp: %ld\n", idtEntry.Stamp);
            printf("Forwarder index: %ld\n", idtEntry.ForwarderIndex);
            printf("Name RVA: 0x%08lx\n", idtEntry.NameRva);
            printf("Import Address Table RVA (Thunk table): 0x%08lx\n", idtEntry.AddressTableRva);

            printf("----\n");
            importTables++;
        } while(
            !((idtEntry.UNION.Characteristics == 0)
            && (idtEntry.UNION.LookupTableRva == 0)
            && (idtEntry.NameRva == 0)
            && (idtEntry.AddressTableRva == 0))
            );
    }


    // import lookup table

    if(options.signature == 0x10b) {
        long entry = 0;
        int cnt = 0;
        do {
            fread(&entry, sizeof(long), 1, ptr_file);
            //long h = entry & 0x7FFFFFFF;

            // anything as second comand argument serves
            // as debug option
            if(argc > 2) {
                printf("\n0x%08lx", entry);
            }

            if(entry == 0) {
                cnt++;
            }
        } while(entry != 0 || cnt <= importTables);
    } else {
        printf("\nCan't handle PE32+\n");
        return 1;
    }


    // name table
    uint16_t hint = 1;
    do {
      hint = print_import_address_table(ptr_file);
    } while(hint != 0);

    free(directory);
    free(descriptor);
    fclose(ptr_file);

    printf("\n");
    return 0;
}
