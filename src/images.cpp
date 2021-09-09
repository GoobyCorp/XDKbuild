#include "stdafx.hpp"

// flash images
FlashImage::FlashImage(PBYTE data, DWORD size) {
	this->TotalSize = size;

	this->PageCount = this->TotalSize / PAGE_SIZE;
	this->pbFlashData = (PBYTE)malloc(this->PageCount * PAGE_DATA_SIZE);
	this->pbEccData = (PBYTE)malloc(this->PageCount * PAGE_ECC_SIZE)  ;

	for(DWORD read = 0; read < this->TotalSize; read += PAGE_SIZE) {
		memcpy(this->pbFlashData + this->FlashSize, data + read, PAGE_DATA_SIZE);
		memcpy(this->pbEccData + this->EccSize, data + (read + PAGE_DATA_SIZE), PAGE_ECC_SIZE);
		this->FlashSize += PAGE_DATA_SIZE;
		this->EccSize += PAGE_ECC_SIZE;
	}

	//ParseFlashHeader();
	//ParseBootloaderHeader(this->FlashHdr.CbOffset);

	// @todo keep up-to-date with the FILE* constructor
}

FlashImage::FlashImage(FILE* f) {
	this->TotalSize = Utils::GetFileSize(f);
	fseek(f, 0, SEEK_SET);

	this->PageCount = this->TotalSize / PAGE_SIZE;
	this->pbFlashData = (PBYTE)malloc(this->PageCount * PAGE_DATA_SIZE);
	this->pbEccData = (PBYTE)malloc(this->PageCount * PAGE_ECC_SIZE);

	// read the file and un-ecc the pages
	for(DWORD read = 0; read < this->TotalSize; read += PAGE_SIZE) {
		fread(this->pbFlashData + this->FlashSize, PAGE_DATA_SIZE, 1, f);
		fread(this->pbEccData + this->EccSize, PAGE_ECC_SIZE, 1, f);
		this->FlashSize += PAGE_DATA_SIZE;
		this->EccSize += PAGE_ECC_SIZE;
	}

	// guess SFC type
	if(this->pbEccData[0x210] == 0x01)
		this->SfcType = SFC_SMALL_ON_SMALL;
	if(this->pbEccData[0x211] == 0x01)
		this->SfcType = SFC_SMALL_ON_BIG;
	if(this->pbEccData[0x1010] == 0xFF && this->pbEccData[0x1011] == 0x01)
		this->SfcType = SFC_BIG_ON_BIG;
	if(this->TotalSize == FLASH_4_GB)
		this->SfcType = SFC_EMMC;

	// flash header parsing
	this->pFlashHdr = (PFLASH_HDR)this->pbFlashData;
	EndianSwapFlashHeader();  // swap to LE

	// 2BL parsing
	DWORD offset = this->pFlashHdr->CbOffset;
	this->pCbaSb2blHdr = (PBL_HDR_WITH_NONCE)(this->pbFlashData + offset);
	EndianSwapBootloaderHeader(this->pCbaSb2blHdr);  // swap to LE
	this->pbCbaSb2blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCbaSb2blHdr->Magic == SB_2BL)  // devkit-specific
		Crypto::XeCryptHmacSha(Globals::_1BL_KEY, 0x10, this->pCbaSb2blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbaSb2blKey, 0x10);
	else if(this->pCbaSb2blHdr->Magic == CB_CBA_2BL) {  // retail-specific

	}
	Crypto::XeCryptRc4(this->CbaSb2blKey, 0x10, this->pbCbaSb2blData, this->pCbaSb2blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCbaSb2blData);

	// 3BL parsing
	offset += this->pCbaSb2blHdr->Size;
	this->pCbbSc3blHdr = (PBL_HDR_WITH_NONCE)(this->pbFlashData + offset);
	EndianSwapBootloaderHeader(this->pCbbSc3blHdr);  // swap to LE
	this->pbCbbSc3blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCbbSc3blHdr->Magic == SC_3BL)  // devkit-specific
		Crypto::XeCryptHmacSha((PBYTE)Globals::ZERO_KEY, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	else if(this->pCbbSc3blHdr->Magic == CC_CBB_3BL) {  // retail-specific
		Crypto::XeCryptHmacSha(this->CbaSb2blKey, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	}
	Crypto::XeCryptRc4(this->CbbSc3blKey, 0x10, this->pbCbbSc3blData, this->pCbbSc3blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCbbSc3blData);

	// 4BL parsing
	offset += this->pCbbSc3blHdr->Size;
	this->pCdSd4blHdr = (PBL_HDR_WITH_NONCE)(this->pbFlashData + offset);
	EndianSwapBootloaderHeader(this->pCdSd4blHdr);  // swap to LE
	this->pbCdSd4blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCdSd4blHdr->Magic == SD_4BL)  // devkit-specific
		Crypto::XeCryptHmacSha(this->CbbSc3blKey, 0x10, this->pCdSd4blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CdSd4blKey, 0x10);
	else if(this->pCdSd4blHdr->Magic == CD_4BL) {  // retail-specific

	}
	Crypto::XeCryptRc4(this->CdSd4blKey, 0x10, this->pbCdSd4blData, this->pCdSd4blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCdSd4blData);
	
	// 5BL parsing
	offset += this->pCdSd4blHdr->Size;
	this->pCeSe5blHdr = (PBL_HDR_WITH_NONCE)(this->pbFlashData + offset);
	EndianSwapBootloaderHeader(this->pCeSe5blHdr);  // swap to LE
	this->pbCeSe5blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCeSe5blHdr->Magic == SE_5BL)  // devkit-specific
		Crypto::XeCryptHmacSha(this->CdSd4blKey, 0x10, this->pCeSe5blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CeSe5blKey, 0x10);
	else if(this->pCeSe5blHdr->Magic == CE_5BL) {  // retail-specific
		
	}
	Crypto::XeCryptRc4(this->CeSe5blKey, 0x10, this->pbCeSe5blData, this->pCeSe5blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCeSe5blData);

	// calculate the total size of the bootloaders
	this->TotalBootloaderSize += this->pCbaSb2blHdr->Size;
	this->TotalBootloaderSize += this->pCbbSc3blHdr->Size;
	this->TotalBootloaderSize += this->pCdSd4blHdr->Size;
	this->TotalBootloaderSize += this->pCeSe5blHdr->Size;

	// null nonces
	memset(this->pCbaSb2blHdr->Nonce, 0, 0x10);
	memset(this->pCbbSc3blHdr->Nonce, 0, 0x10);
	memset(this->pCdSd4blHdr->Nonce, 0, 0x10);
	memset(this->pCeSe5blHdr->Nonce, 0, 0x10);

	Utils::PrintHex(this->CbaSb2blKey, 0x10);
	Utils::PrintHex(this->CbbSc3blKey, 0x10);
	Utils::PrintHex(this->CdSd4blKey, 0x10);
	Utils::PrintHex(this->CeSe5blKey, 0x10);
}

FlashImage::~FlashImage() {
	free(this->pbFlashData);
	free(this->pbEccData);
}

VOID FlashImage::EndianSwapFlashHeader() {
	es16(this->pFlashHdr->Magic);
	es16(this->pFlashHdr->Build);
	es16(this->pFlashHdr->QFE);
	es16(this->pFlashHdr->Flags);
	es32(this->pFlashHdr->CbOffset);
	es32(this->pFlashHdr->Sf1Offset);
	es32(this->pFlashHdr->KvLength);
	es32(this->pFlashHdr->Sf2Offset);
	es16(this->pFlashHdr->PatchSlots);
	es16(this->pFlashHdr->KvVersion);
	es32(this->pFlashHdr->Sf2Offset);
	es32(this->pFlashHdr->KvOffset);
	es32(this->pFlashHdr->PatchSlotSize);
	es32(this->pFlashHdr->SmcConfigOffset);
	es32(this->pFlashHdr->SmcLength);
	es32(this->pFlashHdr->SmcOffset);
}

VOID FlashImage::EndianSwapBootloaderHeader(PBL_HDR_WITH_NONCE bhdr) {
	es16(bhdr->Magic);
	es16(bhdr->Build);
	es16(bhdr->QFE);
	es16(bhdr->Flags);
	es32(bhdr->EntryPoint);
	es32(bhdr->Size);
}