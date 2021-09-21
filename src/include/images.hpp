class FlashImage {
    public:
        FILE* pFile = 0;
        DWORD FileSize = 0;
        DWORD FlashSize = 0;
        DWORD EccSize = 0;

        DWORD PageSize = 0;
        DWORD PageDataSize = 0;

        FlashImage(FILE* f);
        ~FlashImage();

        BOOL ReplaceBootloader(PBYTE data, DWORD size);
        BOOL Patch2BL(PBYTE data);
        BOOL Patch3BL(PBYTE data);
        BOOL Patch4BL(PBYTE data);
        VOID Output(PCHAR fileName);
    private:
        // pointers
        PBYTE pbFlashData = 0;
        PBYTE pbEccData = 0;

        // variables
        SFC_TYPE SfcType = SFC_NONE;
        DWORD PageCount = 0;
        DWORD TotalBootloaderSize = 0;

        PFLASH_HDR pFlashHdr;

        // 2BL-specific attributes
        PBL_HDR_WITH_NONCE pCbaSb2blHdr;
        PBYTE pbCbaSb2blData = 0;
        BYTE CbaSb2blKey[0x10];

        // 3BL-specific attributes
        PBL_HDR_WITH_NONCE pCbbSc3blHdr;
        PBYTE pbCbbSc3blData = 0;
        BYTE CbbSc3blKey[0x10];

        // 4BL-specific attributes
        PBL_HDR_WITH_NONCE  pCdSd4blHdr;
        PBYTE pbCdSd4blData = 0;
        BYTE CdSd4blKey[0x10];

        // 5BL-specific attributes
        PBL_HDR_WITH_NONCE  pCeSe5blHdr;
        PBYTE pbCeSe5blData = 0;
        BYTE CeSe5blKey[0x10];

        VOID GuessSfcTypeForFile();
        VOID GuessSfcTypeForBytes();
        VOID ParseImage();
        VOID FreeImageMemory();
        VOID EndianSwapImageData();
        VOID EndianSwapFlashHeader(PFLASH_HDR fhdr);
        VOID EndianSwapBootloaderHeader(PBL_HDR bhdr);
        BOOL PatchBootloader(PATCH_BL bl, PBYTE data);
        VOID EndianSwapBootloaderHeader(PBL_HDR_WITH_NONCE bhdr);
        VOID GenerateKeys(BOOL devkit);
        BOOL RebuildImage(DWORD oldBlSize);
        VOID NullKeysAndNonces();
};