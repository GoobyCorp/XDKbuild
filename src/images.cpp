#include "stdafx.hpp"

// flash images
FlashImage::FlashImage(PBYTE data, DWORD size) {
	this->TotalSize = size;

	this->PageCount = this->TotalSize / PAGE_SIZE;
	this->pbFlashData = (PBYTE)malloc(this->PageCount * PAGE_DATA_SIZE);
	this->pbEccData = (PBYTE)malloc(this->PageCount * PAGE_ECC_SIZE);

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
	this->TotalSize = utils::GetFileSize(f);
	fseek(f, 0, SEEK_SET);

	this->PageCount = this->TotalSize / PAGE_SIZE;
	this->pbFlashData = (PBYTE)malloc(this->PageCount * PAGE_DATA_SIZE);
	this->pbEccData = (PBYTE)malloc(this->PageCount * PAGE_ECC_SIZE);

	for(DWORD read = 0; read < this->TotalSize; read += PAGE_SIZE) {
		fread(this->pbFlashData + this->FlashSize, PAGE_DATA_SIZE, 1, f);
		fread(this->pbEccData + this->EccSize, PAGE_ECC_SIZE, 1, f);
		this->FlashSize += PAGE_DATA_SIZE;
		this->EccSize += PAGE_ECC_SIZE;
	}

	// flash header parsing
	this->pFlashHdr = ParseFlashHeader();

	// 2BL parsing
	DWORD offset = this->pFlashHdr->CbOffset;
	this->pCbaSb2blHdr = ParseBootloaderHeader(offset);
	this->pbCbaSb2blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	//memcpy(this->pbCbaSb2blData, this->pbFlashData + offset, this->pCbaSb2blHdr->Size);
	if(this->pCbaSb2blHdr->Magic == SB_2BL) {  // devkit-specific
		// Crypto::XeCryptHmacSha(globals::_1BL_KEY, 0x10, this->pCbaSb2blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbaSb2blKey, 0x10);
	} else if(this->pCbaSb2blHdr->Magic == CB_CBA_2BL) {  // retail-specific

	}
	// Crypto::XeCryptRc4(this->CbaSb2blKey, 0x10, this->pbCbaSb2blData, this->pCbaSb2blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCbaSb2blData);

	// 3BL parsing
	offset += this->pCbaSb2blHdr->Size;
	this->pCbbSc3blHdr = ParseBootloaderHeader(offset);
	this->pbCbbSc3blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	//memcpy(this->pbCbbSc3blData, this->pbFlashData + offset, this->pCbbSc3blHdr->Size);
	if(this->pCbbSc3blHdr->Magic == SC_3BL) {  // devkit-specific
		// Crypto::XeCryptHmacSha((PBYTE)globals::ZERO_KEY, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	} else if(this->pCbbSc3blHdr->Magic == CC_CBB_3BL) {  // retail-specific
		// Crypto::XeCryptHmacSha(this->CbaSb2blKey, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	}
	// Crypto::XeCryptRc4(this->CbbSc3blKey, 0x10, this->pbCbbSc3blData, this->pCbbSc3blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCbbSc3blData);

	// 4BL parsing
	offset += this->pCbbSc3blHdr->Size;
	this->pCdSd4blHdr = ParseBootloaderHeader(offset);
	this->pbCdSd4blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	//memcpy(this->pbCdSd4blData, this->pbFlashData + offset, this->pCdSd4blHdr->Size);
	if(this->pCdSd4blHdr->Magic == SD_4BL) {  // devkit-specific
		// Crypto::XeCryptHmacSha(this->CbbSc3blKey, 0x10, this->pCdSd4blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CdSd4blKey, 0x10);
	} else if(this->pCdSd4blHdr->Magic == CD_4BL) {  // retail-specific

	}
	// Crypto::XeCryptRc4(this->CdSd4blKey, 0x10, this->pbCdSd4blData, this->pCdSd4blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCdSd4blData);
	
	// 5BL parsing
	offset += this->pCdSd4blHdr->Size;
	this->pCeSe5blHdr = ParseBootloaderHeader(offset);
	this->pbCeSe5blData = this->pbFlashData + offset + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCeSe5blHdr->Magic == SE_5BL) {  // devkit-specific
		// Crypto::XeCryptHmacSha(this->CdSd4blKey, 0x10, this->pCeSe5blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CeSe5blKey, 0x10);
	} else if(this->pCeSe5blHdr->Magic == CE_5BL) {  // retail-specific
		
	}
	// Crypto::XeCryptRc4(this->CeSe5blKey, 0x10, this->pbCeSe5blData, this->pCeSe5blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCeSe5blData);


	utils::PrintHex(this->CbaSb2blKey, 0x10);
	utils::PrintHex(this->CbbSc3blKey, 0x10);
	utils::PrintHex(this->CdSd4blKey, 0x10);
	utils::PrintHex(this->CeSe5blKey, 0x10);
}

FlashImage::~FlashImage() {
	free(this->pbFlashData);
	free(this->pbEccData);
}

PFLASH_HDR FlashImage::ParseFlashHeader() {
	PFLASH_HDR fhdr = (PFLASH_HDR)this->pbFlashData;

	es16(fhdr->Magic);
	es16(fhdr->Build);
	es16(fhdr->QFE);
	es16(fhdr->Flags);
	es32(fhdr->CbOffset);
	es32(fhdr->Sf1Offset);
	es32(fhdr->KvLength);
	es32(fhdr->Sf2Offset);
	es16(fhdr->PatchSlots);
	es16(fhdr->KvVersion);
	es32(fhdr->Sf2Offset);
	es32(fhdr->KvOffset);
	es32(fhdr->PatchSlotSize);
	es32(fhdr->SmcConfigOffset);
	es32(fhdr->SmcLength);
	es32(fhdr->SmcOffset);

	return fhdr;
}

PBL_HDR_WITH_NONCE FlashImage::ParseBootloaderHeader(DWORD offset) {
	PBL_HDR_WITH_NONCE bhdr = (PBL_HDR_WITH_NONCE)(this->pbFlashData + offset);

	es16(bhdr->Magic);
	es16(bhdr->Build);
	es16(bhdr->QFE);
	es16(bhdr->Flags);
	es32(bhdr->EntryPoint);
	es32(bhdr->Size);
	
	return bhdr;
}