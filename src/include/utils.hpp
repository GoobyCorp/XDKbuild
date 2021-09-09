class Utils {
    public:
        static VOID PrintHex(PBYTE data, DWORD size);
        static int HexDecode(PCHAR pcHex, PBYTE pbOut);
        static DWORD GetFileSize(FILE* fptr);
        static VOID DumpBufferHex(PCHAR filename, PVOID buffer, int size);
        static PBYTE ReadFile(PCHAR fname, PDWORD pdwSize);
        static BOOL WriteFile(PCHAR fileName, PBYTE buffer, DWORD size);
        static DWORD GetPageEcc(PBYTE pbData, PBYTE pbSpare);
        static VOID FixPageEcc(PBYTE pbData, PBYTE pbSpare);
    private:
        static int char2int(char input);
};