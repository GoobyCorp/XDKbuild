#include "stdafx.hpp"

// flash images
FlashImage::FlashImage(FILE* f) {
	this->pFile = f;

	this->FileSize = Utils::GetFileSize(this->pFile);
	fseek(this->pFile, 0, SEEK_SET);

	// guess SFC type early
	this->GuessSfcTypeForFile();

	this->PageCount = this->FileSize / this->PageSize;
	this->pbFlashData = (PBYTE)malloc(this->PageCount * this->PageDataSize);
	if(this->SfcType != SFC_EMMC)
		this->pbEccData = (PBYTE)malloc(this->PageCount * PAGE_ECC_SIZE);

	// read the file and un-ecc the pages
	for(DWORD read = 0; read < this->FileSize; read += this->PageSize) {
		fread(this->pbFlashData + this->FlashSize, this->PageDataSize, 1, f);
		this->FlashSize += this->PageDataSize;
		if(this->SfcType != SFC_EMMC) {
			fread(this->pbEccData + this->EccSize, PAGE_ECC_SIZE, 1, f);
			this->EccSize += PAGE_ECC_SIZE;
		}
	}

	// parse the image
	this->ParseImage();
}

FlashImage::~FlashImage() {
	this->FreeImageMemory();
}

VOID FlashImage::GuessSfcTypeForFile() {
	BYTE tss, tsb;
	WORD tbb;
	fseek(this->pFile, 528 * 32 + 512, SEEK_SET);
	fread(&tss, 1, 1, this->pFile);
	fseek(this->pFile, 528 * 32 + 513, SEEK_SET);
	fread(&tsb, 1, 1, this->pFile);
	fseek(this->pFile, 528 * 0x100 + 512, SEEK_SET);
	fread(&tbb, sizeof(WORD), 1, this->pFile);
	es16(tbb);
	fseek(this->pFile, 0, SEEK_SET);

	if(tss == 0x01)
		this->SfcType = SFC_SMALL_ON_SMALL;
	else if(tsb == 0x01)
		this->SfcType = SFC_SMALL_ON_BIG;
	else if(tbb == 0xFF01)
		this->SfcType = SFC_BIG_ON_BIG;
	else if(this->FileSize == FLASH_4_GB) {
		this->SfcType = SFC_EMMC;
		this->PageSize = 512;
		this->PageDataSize = 512;
	}

	if(this->FileSize != FLASH_4_GB) {
		this->PageSize = 528;
		this->PageDataSize = 512;
	}
}

VOID FlashImage::GuessSfcTypeForBytes() {
	if(this->pbEccData[0x210] == 0x01)
		this->SfcType = SFC_SMALL_ON_SMALL;
	else if(this->pbEccData[0x211] == 0x01)
		this->SfcType = SFC_SMALL_ON_BIG;
	else if(bswap16(*(PWORD)(this->pbEccData + 0x1010)) == 0xFF01)
		this->SfcType = SFC_BIG_ON_BIG;
	else if(this->FileSize == FLASH_4_GB) {
		this->SfcType = SFC_EMMC;
		this->PageSize = 512;
		this->PageDataSize = 512;
	}

	if(this->FileSize != FLASH_4_GB) {
		this->PageSize = 528;
		this->PageDataSize = 512;
	}
}

VOID FlashImage::ParseImage() {
	// flash header parsing
	this->pFlashHdr = (PFLASH_HDR)this->pbFlashData;
	this->EndianSwapFlashHeader(this->pFlashHdr);  // swap to LE

	// 2BL parsing
	PBYTE pbBlAddr = this->pbFlashData + this->pFlashHdr->EntryPoint;
	this->pCbaSb2blHdr = (PBL_HDR_WITH_NONCE)pbBlAddr;
	this->EndianSwapBootloaderHeader(this->pCbaSb2blHdr);  // swap to LE
	this->pbCbaSb2blData = pbBlAddr + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCbaSb2blHdr->Magic == SB_2BL)  // devkit-specific
		Crypto::XeCryptHmacSha(Globals::_1BL_KEY, 0x10, this->pCbaSb2blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbaSb2blKey, 0x10);
	else if(this->pCbaSb2blHdr->Magic == CB_CBA_2BL) {  // retail-specific

	}
	Crypto::XeCryptRc4(this->CbaSb2blKey, 0x10, this->pbCbaSb2blData, this->pCbaSb2blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCbaSb2blData);

	// 3BL parsing
	pbBlAddr += this->pCbaSb2blHdr->Size;
	this->pCbbSc3blHdr = (PBL_HDR_WITH_NONCE)pbBlAddr;
	this->EndianSwapBootloaderHeader(this->pCbbSc3blHdr);  // swap to LE
	this->pbCbbSc3blData = pbBlAddr + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCbbSc3blHdr->Magic == SC_3BL)  // devkit-specific
		Crypto::XeCryptHmacSha((PBYTE)Globals::ZERO_KEY, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	else if(this->pCbbSc3blHdr->Magic == CC_CBB_3BL) {  // retail-specific
		Crypto::XeCryptHmacSha(this->CbaSb2blKey, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	}
	Crypto::XeCryptRc4(this->CbbSc3blKey, 0x10, this->pbCbbSc3blData, this->pCbbSc3blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCbbSc3blData);

	// 4BL parsing
	pbBlAddr += this->pCbbSc3blHdr->Size;
	this->pCdSd4blHdr = (PBL_HDR_WITH_NONCE)pbBlAddr;
	this->EndianSwapBootloaderHeader(this->pCdSd4blHdr);  // swap to LE
	this->pbCdSd4blData = pbBlAddr + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCdSd4blHdr->Magic == SD_4BL)  // devkit-specific
		Crypto::XeCryptHmacSha(this->CbbSc3blKey, 0x10, this->pCdSd4blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CdSd4blKey, 0x10);
	else if(this->pCdSd4blHdr->Magic == CD_4BL) {  // retail-specific

	}
	Crypto::XeCryptRc4(this->CdSd4blKey, 0x10, this->pbCdSd4blData, this->pCdSd4blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCdSd4blData);
	
	// 5BL parsing
	pbBlAddr += this->pCdSd4blHdr->Size;
	this->pCeSe5blHdr = (PBL_HDR_WITH_NONCE)pbBlAddr;
	this->EndianSwapBootloaderHeader(this->pCeSe5blHdr);  // swap to LE
	this->pbCeSe5blData = pbBlAddr + sizeof(BL_HDR_WITH_NONCE);
	if(this->pCeSe5blHdr->Magic == SE_5BL)  // devkit-specific
		Crypto::XeCryptHmacSha(this->CdSd4blKey, 0x10, this->pCeSe5blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CeSe5blKey, 0x10);
	else if(this->pCeSe5blHdr->Magic == CE_5BL) {  // retail-specific
		
	}
	Crypto::XeCryptRc4(this->CeSe5blKey, 0x10, this->pbCeSe5blData, this->pCeSe5blHdr->Size - sizeof(BL_HDR_WITH_NONCE), this->pbCeSe5blData);

	// calculate the total size of the bootloaders
	this->TotalBootloaderSize = 0;
	this->TotalBootloaderSize += this->pCbaSb2blHdr->Size;
	this->TotalBootloaderSize += this->pCbbSc3blHdr->Size;
	this->TotalBootloaderSize += this->pCdSd4blHdr->Size;
	this->TotalBootloaderSize += this->pCeSe5blHdr->Size;

	// null the keys
	// this->NullKeys();
}

VOID FlashImage::FreeImageMemory() {
	if(this->pbFlashData) {
		free(this->pbFlashData);
		this->pbFlashData = 0;
	}
	if(this->pbEccData) {
		free(this->pbEccData);
		this->pbEccData = 0;
	}
}

VOID FlashImage::EndianSwapFlashHeader(PFLASH_HDR fhdr) {
	es16(fhdr->Magic);
	es16(fhdr->Build);
	es16(fhdr->QFE);
	es16(fhdr->Flags);
	es32(fhdr->EntryPoint);
	es32(fhdr->Size);
	// copyright
	// padding
	es32(fhdr->KvLength);
	es32(fhdr->PatchOffset);
	es16(fhdr->PatchSlots);
	es16(fhdr->KvVersion);
	es32(fhdr->KvOffset);
	es32(fhdr->PatchSlotSize);
	es32(fhdr->SmcConfigOffset);
	es32(fhdr->SmcLength);
	es32(fhdr->SmcOffset);
}

VOID FlashImage::EndianSwapBootloaderHeader(PBL_HDR bhdr) {
	es16(bhdr->Magic);
	es16(bhdr->Build);
	es16(bhdr->QFE);
	es16(bhdr->Flags);
	es32(bhdr->EntryPoint);
	es32(bhdr->Size);
	// nonce
}

VOID FlashImage::EndianSwapBootloaderHeader(PBL_HDR_WITH_NONCE bhdr) {
	es16(bhdr->Magic);
	es16(bhdr->Build);
	es16(bhdr->QFE);
	es16(bhdr->Flags);
	es32(bhdr->EntryPoint);
	es32(bhdr->Size);
	// nonce
}

VOID FlashImage::NullKeysAndNonces() {
	// null keys
	memset(this->CbaSb2blKey, 0, 0x10);
	memset(this->CbbSc3blKey, 0, 0x10);
	memset(this->CdSd4blKey, 0, 0x10);
	memset(this->CeSe5blKey, 0, 0x10);

	// null nonces
	memset(this->pCbaSb2blHdr->Nonce, 0, 0x10);
	memset(this->pCbbSc3blHdr->Nonce, 0, 0x10);
	memset(this->pCdSd4blHdr->Nonce, 0, 0x10);
	memset(this->pCeSe5blHdr->Nonce, 0, 0x10);
}

VOID FlashImage::GenerateKeys(BOOL devkit) {
	// generate nonces
	Crypto::XeCryptRandom(this->pCbaSb2blHdr->Nonce, 0x10);
	Crypto::XeCryptRandom(this->pCbbSc3blHdr->Nonce, 0x10);
	Crypto::XeCryptRandom(this->pCdSd4blHdr->Nonce, 0x10);
	Crypto::XeCryptRandom(this->pCeSe5blHdr->Nonce, 0x10);

	// generate keys
	Crypto::XeCryptHmacSha(Globals::_1BL_KEY, 0x10, this->pCbaSb2blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbaSb2blKey, 0x10);
	if(devkit == TRUE)
		Crypto::XeCryptHmacSha((PBYTE)Globals::ZERO_KEY, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	else
		Crypto::XeCryptHmacSha(this->CbaSb2blKey, 0x10, this->pCbbSc3blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CbbSc3blKey, 0x10);
	Crypto::XeCryptHmacSha(this->CbbSc3blKey, 0x10, this->pCdSd4blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CdSd4blKey, 0x10);
	Crypto::XeCryptHmacSha(this->CdSd4blKey, 0x10, this->pCeSe5blHdr->Nonce, 0x10, NULL, 0, NULL, 0, this->CeSe5blKey, 0x10);
}

BOOL FlashImage::RebuildImage(DWORD oldBlSize) {
	// this function extracts the parts of the original image and pieces them back together

	PBYTE pbNewFlashData = (PBYTE)malloc(this->FlashSize);
	// expensive!
	memcpy(pbNewFlashData, this->pbFlashData, this->FlashSize);

	PBYTE pbNewEccData;
	if(this->SfcType != SFC_EMMC) {
		pbNewEccData = (PBYTE)malloc(this->EccSize);
		memcpy(pbNewEccData, this->pbEccData, this->EccSize);
	}

	PFLASH_HDR pNewFlashHdr = (PFLASH_HDR)pbNewFlashData;

	// clean up image for placing new loaders
	if(oldBlSize > this->TotalBootloaderSize)
		memset(pbNewFlashData + pNewFlashHdr->EntryPoint, 0xFF, oldBlSize);
	else if(oldBlSize < this->TotalBootloaderSize)
		memset(pbNewFlashData + pNewFlashHdr->EntryPoint, 0xFF, this->TotalBootloaderSize);
	else
		memset(pbNewFlashData + pNewFlashHdr->EntryPoint, 0xFF, this->TotalBootloaderSize);

	// 2BL
	PBYTE pbBlAddr = pbNewFlashData + pNewFlashHdr->EntryPoint;
	// copy 2BL header
	memcpy(pbBlAddr, this->pCbaSb2blHdr, sizeof(BL_HDR_WITH_NONCE));
	// fix the bootloader header
	this->EndianSwapBootloaderHeader((PBL_HDR_WITH_NONCE)pbBlAddr);  // swap to BE
	// copy 2BL data
	memcpy(pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pbCbaSb2blData, this->pCbaSb2blHdr->Size - sizeof(BL_HDR_WITH_NONCE));
	// encrypt 2BL data
	Crypto::XeCryptRc4(this->CbaSb2blKey, 0x10, pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pCbaSb2blHdr->Size - sizeof(BL_HDR_WITH_NONCE), pbBlAddr + sizeof(BL_HDR_WITH_NONCE));

	// 3BL
	pbBlAddr += this->pCbaSb2blHdr->Size;
	// copy 3BL header
	memcpy(pbBlAddr, this->pCbbSc3blHdr, sizeof(BL_HDR_WITH_NONCE));
	// fix the bootloader header
	this->EndianSwapBootloaderHeader((PBL_HDR_WITH_NONCE)pbBlAddr);  // swap to BE
	// copy 3BL data
	memcpy(pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pbCbbSc3blData, this->pCbbSc3blHdr->Size - sizeof(BL_HDR_WITH_NONCE));
	// encrypt 3BL data
	Crypto::XeCryptRc4(this->CbbSc3blKey, 0x10, pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pCbbSc3blHdr->Size - sizeof(BL_HDR_WITH_NONCE), pbBlAddr + sizeof(BL_HDR_WITH_NONCE));

	// 4BL
	pbBlAddr += this->pCbbSc3blHdr->Size;
	// copy 4BL header
	memcpy(pbBlAddr, this->pCdSd4blHdr, sizeof(BL_HDR_WITH_NONCE));
	// fix the bootloader header
	this->EndianSwapBootloaderHeader((PBL_HDR_WITH_NONCE)pbBlAddr);  // swap to BE
	// copy 4BL data
	memcpy(pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pbCdSd4blData, this->pCdSd4blHdr->Size - sizeof(BL_HDR_WITH_NONCE));
	// encrypt 4BL data
	Crypto::XeCryptRc4(this->CdSd4blKey, 0x10, pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pCdSd4blHdr->Size - sizeof(BL_HDR_WITH_NONCE), pbBlAddr + sizeof(BL_HDR_WITH_NONCE));

	// 5BL
	pbBlAddr += this->pCdSd4blHdr->Size;
	// copy 5BL header
	memcpy(pbBlAddr, this->pCeSe5blHdr, sizeof(BL_HDR_WITH_NONCE));
	// fix the bootloader header
	this->EndianSwapBootloaderHeader((PBL_HDR_WITH_NONCE)pbBlAddr);  // swap to BE
	// copy 5BL data
	memcpy(pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pbCeSe5blData, this->pCeSe5blHdr->Size - sizeof(BL_HDR_WITH_NONCE));
	// encrypt 5BL data
	Crypto::XeCryptRc4(this->CeSe5blKey, 0x10, pbBlAddr + sizeof(BL_HDR_WITH_NONCE), this->pCeSe5blHdr->Size - sizeof(BL_HDR_WITH_NONCE), pbBlAddr + sizeof(BL_HDR_WITH_NONCE));

	// fix flash header
	this->EndianSwapFlashHeader(pNewFlashHdr);  // swap to BE

	// free original image memory
	this->FreeImageMemory();

	// remap the new image
	this->pbFlashData = pbNewFlashData;
	this->FileSize = this->FlashSize;
	if(this->SfcType != SFC_EMMC) {
		this->pbEccData = pbNewEccData;
	} else
		this->FileSize += this->EccSize;
	this->PageCount = this->FileSize / this->PageSize;

	// check the SFC type
	// this->GuessSfcTypeForBytes();
	
	// parse the new image
	this->ParseImage();

	return TRUE;
}

VOID FlashImage::EndianSwapImageData() {
	// image must be parsed first!

	// flash header
	EndianSwapFlashHeader(this->pFlashHdr);

	// 2BL
	EndianSwapBootloaderHeader(this->pCbaSb2blHdr);

	// 3BL
	EndianSwapBootloaderHeader(this->pCbbSc3blHdr);

	// 4BL
	EndianSwapBootloaderHeader(this->pCdSd4blHdr);

	// 5BL
	EndianSwapBootloaderHeader(this->pCeSe5blHdr);
}

BOOL FlashImage::ReplaceBootloader(PBYTE data, DWORD size) {
	PBL_HDR_WITH_NONCE bhdr = (PBL_HDR_WITH_NONCE)data;
	this->EndianSwapBootloaderHeader(bhdr);  // swap to LE

	// fix size mismatch
	if(bhdr->Size != size)
		bhdr->Size = size;

	BOOL devkit = TRUE;
	DWORD oldSize = this->TotalBootloaderSize;
	switch(bhdr->Magic) {
		// 2BL
		case SB_2BL: {
			this->TotalBootloaderSize -= this->pCbaSb2blHdr->Size;
			this->pCbaSb2blHdr = bhdr;
			this->pbCbaSb2blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		case CB_CBA_2BL: {
			devkit = FALSE;
			this->TotalBootloaderSize -= this->pCbaSb2blHdr->Size;
			this->pCbaSb2blHdr = bhdr;
			this->pbCbaSb2blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		// 3BL
		case SC_3BL: {
			this->TotalBootloaderSize -= this->pCbbSc3blHdr->Size;
			this->pCbbSc3blHdr = bhdr;
			this->pbCbbSc3blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		case CC_CBB_3BL: {
			devkit = FALSE;
			this->TotalBootloaderSize -= this->pCbbSc3blHdr->Size;
			this->pCbbSc3blHdr = bhdr;
			this->pbCbbSc3blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		// 4BL
		case SD_4BL: {
			this->TotalBootloaderSize -= this->pCdSd4blHdr->Size;
			this->pCdSd4blHdr = bhdr;
			this->pbCdSd4blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		case CD_4BL: {
			devkit = FALSE;
			this->TotalBootloaderSize -= this->pCdSd4blHdr->Size;
			this->pCdSd4blHdr = bhdr;
			this->pbCdSd4blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		// 5BL
		case SE_5BL: {
			this->TotalBootloaderSize -= this->pCeSe5blHdr->Size;
			this->pCeSe5blHdr = bhdr;
			this->pbCeSe5blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		case CE_5BL: {
			devkit = FALSE;
			this->TotalBootloaderSize -= this->pCeSe5blHdr->Size;
			this->pCeSe5blHdr = bhdr;
			this->pbCeSe5blData = data + sizeof(BL_HDR_WITH_NONCE);
			this->TotalBootloaderSize += size;
			break;
		}
		default: {
			return FALSE;
		}
	}

	this->GenerateKeys(devkit);
	this->RebuildImage(oldSize);

	return TRUE;
}

BOOL FlashImage::PatchBootloader(PATCH_BL target, PBYTE data) {
	PBYTE pbBlData;
	switch(target) {
		case PATCH_2BL: {
			pbBlData = this->pbCbaSb2blData;
			break;
		}
		case PATCH_3BL: {
			pbBlData = this->pbCbbSc3blData;
			break;
		}
		case PATCH_4BL: {
			pbBlData = this->pbCdSd4blData;
			break;
		}
		default: {
			return FALSE;
		}
	}

	// read xebuild patches
	PBYTE pbData = data;
	while(true) {
		DWORD addr = bswap32(*(PDWORD)pbData);
		if(addr == 0xFFFFFFFF)
			break;
		pbData += sizeof(DWORD);
		DWORD sizeInts = bswap32(*(PDWORD)(pbData));
		DWORD sizeBytes = sizeInts * sizeof(DWORD);
		pbData += sizeof(DWORD);
		memcpy(pbBlData + addr, pbData, sizeBytes);
		pbData += sizeBytes;
	}

	return TRUE;
}

BOOL FlashImage::Patch2BL(PBYTE data) {
	return this->PatchBootloader(PATCH_2BL, data);
}

BOOL FlashImage::Patch3BL(PBYTE data) {
	return this->PatchBootloader(PATCH_3BL, data);
}

BOOL FlashImage::Patch4BL(PBYTE data) {
	return this->PatchBootloader(PATCH_4BL, data);
}

VOID FlashImage::Output(PCHAR fileName) {
	PBYTE pbOutData = (PBYTE)malloc(this->FileSize);
	PBYTE pbOutTmp = pbOutData;
	this->EndianSwapImageData();  // swap to BE
	for(DWORD pageNum = 0; pageNum < this->PageCount; pageNum++) {
		// recalculate ecc bits
		if(this->SfcType != SFC_EMMC)
			Utils::FixPageEcc(this->pbFlashData + (pageNum * this->PageDataSize), this->pbEccData + (pageNum * PAGE_ECC_SIZE));
		memcpy(pbOutTmp, this->pbFlashData + (pageNum * this->PageDataSize), this->PageDataSize);
		pbOutTmp += this->PageDataSize;
		if(this->SfcType != SFC_EMMC) {
			memcpy(pbOutTmp, this->pbEccData + (pageNum * PAGE_ECC_SIZE), PAGE_ECC_SIZE);
			pbOutTmp += PAGE_ECC_SIZE;
		}
	}
	Utils::WriteFile(fileName, pbOutData, this->FileSize);
	free(pbOutData);
	this->EndianSwapImageData();  // swap to LE
}