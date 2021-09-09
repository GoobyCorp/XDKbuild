class FlashImage {
    public:
        FlashImage(PBYTE data, DWORD size);
        FlashImage(FILE* f);
        ~FlashImage();
    private:
        // pointers
        PBYTE pbFlashData;
        PBYTE pbEccData;

        // variables
        DWORD FlashSize = 0;
        DWORD EccSize = 0;
        DWORD TotalSize = 0;
        DWORD PageCount = 0;

        PFLASH_HDR pFlashHdr;

        // 2BL-specific attributes
        PBL_HDR_WITH_NONCE pCbaSb2blHdr;
        PBYTE pbCbaSb2blData;
        BYTE CbaSb2blKey[0x10];

        // 3BL-specific attributes
        PBL_HDR_WITH_NONCE pCbbSc3blHdr;
        PBYTE pbCbbSc3blData;
        BYTE CbbSc3blKey[0x10];

        // 4BL-specific attributes
        PBL_HDR_WITH_NONCE  pCdSd4blHdr;
        PBYTE pbCdSd4blData;
        BYTE CdSd4blKey[0x10];

        // 5BL-specific attributes
        PBL_HDR_WITH_NONCE  pCeSe5blHdr;
        PBYTE pbCeSe5blData;
        BYTE CeSe5blKey[0x10];

        PFLASH_HDR ParseFlashHeader();
        PBL_HDR_WITH_NONCE ParseBootloaderHeader(DWORD offset);
};