class FlashImage {
    public:
        FlashImage(PBYTE data, DWORD size);
        FlashImage(FILE* f);
        ~FlashImage();
    private:
        PBYTE FlashData;
        PBYTE EccData;
        DWORD FlashSize;
        DWORD EccSize;
        DWORD TotalSize;
        DWORD PageCount;
        FLASH_HDR FlashHdr;
        CBA_SB_2BL_HDR CbaSb2blHdr;
        CBB_SC_3BL_HDR CbbSc3blHdr;
        CD_SD_4BL_HDR  CdSd4blHdr;
        CE_SE_5BL_HDR  CeSe5blHdr;

        VOID ParseFlashHeader();
        BL_HDR_WITH_NONCE ParseBootloaderHeader(DWORD offset);
};