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
#include "functions.c"

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

    IMAGE_DOS_HEADER header;

    fread(&header, sizeof(struct _IMAGE_DOS_HEADER), 1, ptr_file);

    if(header.e_magic != 0x5a4d) {
        printf("%s", "Not MZ\n");
        return 1;
    }

    printf("\nDOS header\n");
    printf("---------------------------------------------------------\n");

    printDosHeader(&header);

    fseek(ptr_file, header.e_lfanew, SEEK_SET);

    COFFHeader coff;
    fread(&coff, sizeof(coff), 1, ptr_file);

    printf("\nPE header\n");
    printf("---------------------------------------------------------\n");

    printfCoffHeader(&coff);

    PEOptionsHeader options;
    fread(&options, sizeof(options), 1, ptr_file);

    printf("\nOptional header (standard fields)\n");
    printf("---------------------------------------------------------\n");

    printOptionsHeader(&options);

    printf("\nOptional header (Windows-specific)\n");
    printf("---------------------------------------------------------\n");

    if(options.signature == 0x20b) {
        // PE32+
        PEOptionsHeaderWindowsPlus pe32Plus;
        fread(&pe32Plus, sizeof(PEOptionsHeaderWindowsPlus), 1, ptr_file);
        printPe32PlusWindowsSpecificHeader(&pe32Plus);
        fclose(ptr_file);
        printf("Can't go further.\n");
        return 0;

    } else {
        // PE32
        PEOptionsHeaderWindows pe32;
        fread(&pe32, sizeof(PEOptionsHeaderWindows), 1, ptr_file);

        printPe32WindowsSpecificHeader(&pe32);

        printf("\nData directories\n");
        printf("---------------------------------------------------------\n");
        DataDirectory *directory = malloc(pe32.NumberOfRvaAndSizes * sizeof(DataDirectory));
        fread(directory, sizeof(DataDirectory), pe32.NumberOfRvaAndSizes, ptr_file);

        printf("VA\t\tSize\n");
        printf("---------------------------------------------------------\n");

        int i;
        for(i=0;i<pe32.NumberOfRvaAndSizes;i++) {
            printf("0x%08lx\t%ld\n", directory[i].VirtualAddress, directory[i].Size);
        }

        free(directory);
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


    int i;
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
            printIdtEntry(&idtEntry);
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

    free(descriptor);
    fclose(ptr_file);

    printf("\n");
    return 0;
}
