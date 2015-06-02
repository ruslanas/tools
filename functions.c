#define MAX_SYMBOL_LENGTH 256

void printOptionsHeader(PEOptionsHeader* options) {
    char* peFormat;

    if(options->signature == 0x10b) {
        peFormat = "PE32";
    } else {
        peFormat = "PE32+";
    }


    printf("Signature:\t\t\t %s\n", peFormat);
    printf("Address of entry point:\t\t 0x%lx\n", options->AddressOfEntryPoint);
    printf("Base of code:\t\t\t %ld\n", options->BaseOfCode);
}

void printDosHeader(IMAGE_DOS_HEADER* header) {
    printf("Magic:\t\t\t\t 0x%04x\n", header->e_magic);
    printf("Bytes on last page:\t\t %d\n", header->e_cblp);
    printf("Pages:\t\t\t\t %d\n", header->e_cp);
    printf("Relocations:\t\t\t %d\n", header->e_crlc);
    printf("Size of header in paragraphs:\t %d\n", header->e_cparhdr);

    // ...

    printf("Initial SP:\t\t\t 0x%08x\n", header->e_sp);
    printf("Initial CS:\t\t\t 0x%08x\n", header->e_cs);

    printf("File address of new EXE header:\t %ld\n", header->e_lfanew);
}

void printfCoffHeader(COFFHeader* coff) {
    printf("Signature:\t\t\t %s\n", coff->Signature);

    if(coff->Machine == 0x14c) {
        printf("Machine:\t\t\t Intel 386\n");
    } else {
        printf("Machine:\t\t\t 0x%04x\n", coff->Machine);
    }

    printf("Number of sections:\t\t %hu\n", coff->NumberOfSections);
    printf("TimeStamp:\t\t\t %ld\n", coff->TimeDateStamp);

    printf("Size of optional header:\t %hu\n", coff->SizeOfOptionalHeader);
    printf("Characteristics:\t\t 0x%04x\n", coff->Characteristics);

}

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

void printPe32WindowsSpecificHeader(PEOptionsHeaderWindows* pe32) {

    printf("Major OS version:\t\t %hu\n", pe32->MajorOSVersion);
    printf("Section alignment:\t\t 0x%08lx\n", pe32->SectionAlignment);
    printf("File alignment:\t\t\t 0x%08lx\n", pe32->FileAlignment);
    printf("Image base:\t\t\t 0x%08lx\n", pe32->ImageBase);
    printf("Number of data directories:\t %ld\n", pe32->NumberOfRvaAndSizes);
}

void printPe32PlusWindowsSpecificHeader(PEOptionsHeaderWindowsPlus* pe32) {

    printf("Major OS version:\t\t %hu\n", pe32->MajorOSVersion);
    printf("Section alignment:\t\t 0x%08lx\n", pe32->SectionAlignment);
    printf("File alignment:\t\t\t 0x%08lx\n", pe32->FileAlignment);
    printf("Image base:\t\t\t 0x%08lx\n", (long)pe32->ImageBase);
    printf("Number of data directories:\t %ld\n", pe32->NumberOfRvaAndSizes);
}
void printIdtEntry(ImportDirectoryTableEntry* idtEntry) {
    printf("Lookup Table RVA: 0x%08lx\n", idtEntry->UNION.LookupTableRva);
    printf("Time stamp: %ld\n", idtEntry->Stamp);
    printf("Forwarder index: %ld\n", idtEntry->ForwarderIndex);
    printf("Name RVA: 0x%08lx\n", idtEntry->NameRva);
    printf("Import Address Table RVA (Thunk table): 0x%08lx\n", idtEntry->AddressTableRva);
}
