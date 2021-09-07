#include "stdafx.hpp"

// flash images
FlashImage::FlashImage(PBYTE data, DWORD size) {
    this->TotalSize = size;

    this->PageCount = this->TotalSize / PAGE_SIZE;
    this->FlashData = (PBYTE)malloc(this->PageCount * PAGE_DATA_SIZE);
    this->EccData = (PBYTE)malloc(this->PageCount * PAGE_ECC_SIZE);

    for(int read = 0; read < this->TotalSize; read += PAGE_SIZE) {
        memcpy(FlashData + this->FlashSize, data + read, PAGE_DATA_SIZE);
        memcpy(EccData + this->EccSize, data + (read + PAGE_DATA_SIZE), PAGE_ECC_SIZE);
        this->FlashSize += PAGE_DATA_SIZE;
        this->EccSize += PAGE_ECC_SIZE;
    }

    ParseFlashHeader();
    BL_HDR_WITH_NONCE cb_sb_2bl_hdr = ParseBootloaderHeader(this->FlashHdr.CbOffset);
}

FlashImage::FlashImage(FILE* f) {
    this->TotalSize = utils::GetFileSize(f);
    fseek(f, 0, SEEK_SET);

    this->PageCount = this->TotalSize / PAGE_SIZE;
    this->FlashData = (PBYTE)malloc(this->PageCount * PAGE_DATA_SIZE);
    this->EccData = (PBYTE)malloc(this->PageCount * PAGE_ECC_SIZE);

    for(int read = 0; read < this->TotalSize; read += PAGE_SIZE) {
        fread(FlashData + this->FlashSize, PAGE_DATA_SIZE, 1, f);
        fread(EccData + this->EccSize, PAGE_ECC_SIZE, 1, f);
        this->FlashSize += PAGE_DATA_SIZE;
        this->EccSize += PAGE_ECC_SIZE;
    }

    ParseFlashHeader();
    this->CbaSb2blHdr = ParseBootloaderHeader(this->FlashHdr.CbOffset);
    printf("0x%04X\n", this->CbaSb2blHdr.Magic);
    this->CbbSc3blHdr = ParseBootloaderHeader(this->FlashHdr.CbOffset + this->CbaSb2blHdr.Size);
    printf("0x%04X\n", this->CbbSc3blHdr.Magic);
    this->CdSd4blHdr = ParseBootloaderHeader(this->FlashHdr.CbOffset + this->CbaSb2blHdr.Size + this->CbbSc3blHdr.Size);
    printf("0x%04X\n", this->CdSd4blHdr.Magic);
    this->CeSe5blHdr = ParseBootloaderHeader(this->FlashHdr.CbOffset + this->CbaSb2blHdr.Size + this->CbbSc3blHdr.Size + this->CdSd4blHdr.Size);
    printf("0x%04X\n", this->CeSe5blHdr.Magic);
}

FlashImage::~FlashImage() {
    free(this->FlashData);
    free(this->EccData);
}

VOID FlashImage::ParseFlashHeader() {
	// I KNOW THIS IS AWFUL
	PBYTE pbFlash = this->FlashData;
	this->FlashHdr.Magic = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	FlashHdr.Build = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	FlashHdr.QFE = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	FlashHdr.Flags = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	FlashHdr.CbOffset = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.Sf1Offset = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	memcpy(&FlashHdr.Copyright, pbFlash, 0x40);
	pbFlash += 0x40;
	memcpy(&FlashHdr.Padding, pbFlash, 0x10);
	FlashHdr.KvLength = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.Sf2Offset = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.PatchSlots = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	FlashHdr.KvVersion = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	FlashHdr.Sf2Offset = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.KvOffset = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.PatchSlotSize = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.SmcConfigOffset = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.SmcLength = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	FlashHdr.SmcOffset = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
}

BL_HDR_WITH_NONCE FlashImage::ParseBootloaderHeader(DWORD offset) {
	PBYTE pbFlash = this->FlashData + offset;
	BL_HDR_WITH_NONCE hdr;
	hdr.Magic = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	hdr.Build = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	hdr.QFE = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	hdr.Flags = bswap16(*(PWORD)pbFlash);
	pbFlash += sizeof(WORD);
	hdr.EntryPoint = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
	hdr.Size = bswap32(*(PDWORD)pbFlash);
	pbFlash += sizeof(DWORD);
    memcpy(hdr.Nonce, pbFlash, 0x10);
    pbFlash += 0x10;
	return hdr;
}