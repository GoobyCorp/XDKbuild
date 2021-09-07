class utils {
    public:
        static int HexDecode(PCHAR pcHex, PBYTE pbOut);
        static DWORD GetFileSize(FILE* fptr);
        static VOID DumpBufferHex(PCHAR filename, PVOID buffer, int size);
        static PBYTE ReadFile(PCHAR fname, PDWORD pdwSize);
        static BOOL WriteFile(PCHAR fileName, PBYTE buffer, DWORD size);
    private:
        static int char2int(char input);
};