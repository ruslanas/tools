#include <stdint.h>
#include <inttypes.h>

typedef struct COFFHeader {
    char Signature[4];
    short Machine;
    short NumberOfSections;
    long TimeDateStamp;
    long PointerToSymbolTable;
    long NumberOfSymbols;
    short SizeOfOptionalHeader;
    short Characteristics;
} COFFHeader;

typedef struct WindowsOptions {
    short OsVersion;
    long SectionAlignment;
    long ImageBase;
    long FileAlignment;
    long NumberOfRvaAndSizes;
} WindowsOptions;

typedef struct DataDirectory {
    long VirtualAddress;
    long Size;
 } DataDirectory;

typedef struct PEOptionsHeader {
    short signature; //decimal number 267 for 32 bit, and 523 for 64 bit.
    char MajorLinkerVersion;
    char MinorLinkerVersion;
    long SizeOfCode;
    long SizeOfInitializedData;
    long SizeOfUninitializedData;
    long AddressOfEntryPoint;  //The RVA of the code entry point
    long BaseOfCode;
} PEOptionsHeader;

typedef struct PEOptionsHeaderWindows {
    long BaseOfData;
    long ImageBase;
    long SectionAlignment;
    long FileAlignment;
    short MajorOSVersion;
    short MinorOSVersion;
    short MajorImageVersion;
    short MinorImageVersion;
    short MajorSubsystemVersion;
    short MinorSubsystemVersion;
    long Reserved;
    long SizeOfImage;
    long SizeOfHeaders;
    long Checksum;
    short Subsystem;
    short DLLCharacteristics;
    long SizeOfStackReserve;
    long SizeOfStackCommit;
    long SizeOfHeapReserve;
    long SizeOfHeapCommit;
    long LoaderFlags;
    long NumberOfRvaAndSizes;
} PEOptionsHeaderWindows;

typedef struct PEOptionsHeaderWindowsPlus {
    char ImageBase[8];
    long SectionAlignment;
    long FileAlignment;
    short MajorOSVersion;
    short MinorOSVersion;
    short MajorImageVersion;
    short MinorImageVersion;
    short MajorSubsystemVersion;
    short MinorSubsystemVersion;
    long Reserved;
    long SizeOfImage;
    long SizeOfHeaders;
    long Checksum;
    short Subsystem;
    short DLLCharacteristics;

    char SizeOfStackReserve[8];
    char SizeOfStackCommit[8];
    char SizeOfHeapReserve[8];
    char SizeOfHeapCommit[8];

    long LoaderFlags;
    long NumberOfRvaAndSizes;
} PEOptionsHeaderWindowsPlus;

typedef struct SectionDescriptor {
    char Name[8];
    long SizeInMemory;
    long LocationInMemory;
    long PhysicalSize;
    long PhysicalLocation; // from start of disk image
    char Reserved[12];
    long Flags;
} SectionDescriptor;

// IMAGE_IMPORT_DESCRIPTOR
typedef struct ImportDirectoryTableEntry {
    union {
        long Characteristics;
        long LookupTableRva;
    } UNION;
    long Stamp;
    long ForwarderIndex;
    long NameRva;
    long AddressTableRva;
} ImportDirectoryTableEntry;
