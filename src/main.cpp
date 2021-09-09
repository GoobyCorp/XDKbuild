#include "stdafx.hpp"

/* FLASH_HDR FlashHdr;
PBYTE flash;
PBYTE ecc;
PBYTE boot_blk;
PBYTE ldr_d;
PBYTE ldr_e;
PBYTE ldr_c;
BYTE bl_key[0x10];
BYTE zero_key[0x10];

BYTE ldr_cba_sb_2bl_key[0x10];
BYTE ldr_cbb_sc_3bl_key[0x10];
BYTE ldr_cd_sd_4bl_key[0x10];
BYTE ldr_ce_se_5bl_key[0x10];
BYTE ldr_sc_3bl_key[0x10];

BYTE hmac[0x40];
BYTE sha[0x40];
BYTE ecc_hash[0x10];
BYTE new_ecc[0x10];

int i = 0;
int f_sz;
int f_pgs;
int x = 0;
int y = 0;
int ldr_cba_sb_2bl_offs;
int ldr_cba_sb_2bl_size;
int ldr_cbb_sc_3bl_offs;
int ldr_cbb_sc_3bl_size;
int ldr_cd_sd_4bl_offs;
int ldr_cd_sd_4bl_size;
int ldr_ce_se_5bl_offs;
int ldr_ce_se_5bl_size;
// int ldr_c_end;
int boot_blk_sz;
SFC_TYPE sfc;

DWORD GetPageEcc(PBYTE datc, PBYTE spare)
{
	DWORD i = 0, val = 0, v = 0;
	PDWORD data = (PDWORD)datc;
	for (i = 0; i < 0x1066; i++)
	{
		if (!(i & 31))
		{
			if (i == 0x1000)
				data = (PDWORD)spare;
			v = ~*data++; // byte order: LE
		}
		val ^= v & 1;
		v >>= 1;
		if (val & 1)
			val ^= 0x6954559;
		val >>= 1;
	}
	return ~val;
}

VOID FixPageEcc(PBYTE datc, PBYTE spare)
{
	DWORD val = GetPageEcc(datc, spare);
	spare[12] = (spare[12] & 0x3F) + ((val << 6) & 0xC0);
	spare[13] = (val >> 2) & 0xFF;
	spare[14] = (val >> 10) & 0xFF;
	spare[15] = (val >> 18) & 0xFF;
}

VOID FixFuses()
{
	int patch_slot_start;
	int patch_slot_sz;
	int vfuse_start;
	int rows = 0;
	int rowplus = 0;
	int ctr = 0;
	QWORD testfuse;
	QWORD fuse;
   
	patch_slot_start = FlashHdr.Sf1Offset;
	patch_slot_sz = FlashHdr.PatchSlotSize;
	vfuse_start = patch_slot_start + patch_slot_sz;
	
	testfuse = bswap64(*(PQWORD)(flash + vfuse_start));
	if(testfuse != 0xC0FFFFFFFFFFFFFF && ctr != f_sz) {
		while(testfuse != 0xC0FFFFFFFFFFFFFF) {
			testfuse = bswap64(*(PQWORD)(flash + ctr));
			ctr += sizeof(QWORD);
		}
			
		printf("Virtual Fuses Found At 0x%llX\n:", ctr - sizeof(QWORD));
		ctr -= sizeof(QWORD);
		*(PDWORD)(flash + 0x70) = bswap32(ctr - patch_slot_start);
	}

	patch_slot_sz = FlashHdr.PatchSlotSize;
	vfuse_start = patch_slot_start + patch_slot_sz;
	
	memset(&flash[vfuse_start + 56], 0, 40);
	
	printf("Virtual Fuses Set To:\n");
	while(rows != 12) {
	   fuse = bswap64(*(PQWORD)(flash + vfuse_start + rowplus));
	   printf("Fuseset %02d: %016llX\n", rows, fuse);
	   rows++;
	   rowplus += sizeof(QWORD);
	}
}

VOID GetLdrHdrs()
{
	ldr_cba_sb_2bl_offs = FlashHdr.CbOffset;
	ldr_cba_sb_2bl_size = bswap32(*(PDWORD)(flash + ldr_cba_sb_2bl_offs + 0xC));
	if(bswap16(*(PWORD)(flash + ldr_cba_sb_2bl_offs)) != CB_CBA_2BL && bswap16(*(PWORD)(flash + ldr_cba_sb_2bl_offs)) != SB_2BL) {
		printf("fuck 1!\n");
	}
	
	ldr_cbb_sc_3bl_offs = ldr_cba_sb_2bl_offs + ldr_cba_sb_2bl_size;
	ldr_cbb_sc_3bl_size = bswap32(*(PDWORD)(flash + ldr_cbb_sc_3bl_offs + 0xC));
	if(bswap16(*(PWORD)(flash + ldr_cbb_sc_3bl_offs)) != CC_CBB_3BL && bswap16(*(PWORD)(flash + ldr_cbb_sc_3bl_offs)) != SC_3BL) {
		printf("fuck 2!\n");
	}

	ldr_cd_sd_4bl_offs = ldr_cbb_sc_3bl_offs + ldr_cbb_sc_3bl_size;
	ldr_cd_sd_4bl_size = bswap32(*(PDWORD)(flash + ldr_cd_sd_4bl_offs + 0xC));
	if(bswap16(*(PWORD)(flash + ldr_cd_sd_4bl_offs)) != CD_4BL && bswap16(*(PWORD)(flash + ldr_cd_sd_4bl_offs)) != SD_4BL) {
		printf("fuck 3!\n");
	}

	ldr_ce_se_5bl_offs = ldr_cd_sd_4bl_offs + ldr_cd_sd_4bl_size;
	ldr_ce_se_5bl_size = bswap32(*(PDWORD)(flash + ldr_ce_se_5bl_offs + 0xC));
	if(bswap16(*(PWORD)(flash + ldr_ce_se_5bl_offs)) != CE_5BL && bswap16(*(PWORD)(flash + ldr_ce_se_5bl_offs)) != SE_5BL) {
		printf("fuck 4!\n");
	}
}

VOID DecryptLdrs()
{
	Crypto::XeCryptRc4(ldr_cd_sd_4bl_key, 0x10, &flash[ldr_cd_sd_4bl_offs + 0x20], ldr_cd_sd_4bl_size-0x20, &flash[ldr_cd_sd_4bl_offs + 0x20]);
	Crypto::XeCryptRc4(ldr_ce_se_5bl_key, 0x10, &flash[ldr_ce_se_5bl_offs + 0x20], ldr_ce_se_5bl_size-0x20, &flash[ldr_ce_se_5bl_offs + 0x20]);
}

VOID GetLdrKeysRetail()
{
	memset(zero_key, 0, 0x10);
	Crypto::XeCryptHmacSha(bl_key, 0x10, &flash[ldr_cba_sb_2bl_offs + 0x10], 0x10, NULL, 0, NULL, 0, ldr_cba_sb_2bl_key, 0x10);
	Crypto::XeCryptHmacSha(ldr_cba_sb_2bl_key, 0x10, &flash[ldr_cbb_sc_3bl_offs + 0x10], 0x10, zero_key, 0x10, NULL, 0, ldr_cbb_sc_3bl_key, 0x10);
	Crypto::XeCryptHmacSha(ldr_cbb_sc_3bl_key, 0x10, &flash[ldr_cd_sd_4bl_offs + 0x10], 0x10, NULL, 0, NULL, 0, ldr_cd_sd_4bl_key, 0x10);
	Crypto::XeCryptHmacSha(ldr_cd_sd_4bl_key, 0x10, &flash[ldr_ce_se_5bl_offs + 0x10], 0x10, NULL, 0, NULL, 0, ldr_ce_se_5bl_key, 0x10);
}

VOID GetLdrKeysDevkit()
{
	memset(zero_key, 0, 0x10);
	Crypto::XeCryptHmacSha(zero_key, 0x10, &ldr_c[0x10], 0x10, NULL, 0, NULL, 0, ldr_sc_3bl_key, 0x10);
	Crypto::XeCryptHmacSha(ldr_sc_3bl_key, 0x10, &ldr_d[0x10], 0x10, NULL, 0, NULL, 0, ldr_cd_sd_4bl_key, 0x10);
	Crypto::XeCryptHmacSha(ldr_cd_sd_4bl_key, 0x10, &ldr_e[0x10], 0x10, NULL, 0, NULL, 0, ldr_ce_se_5bl_key, 0x10);
}

VOID EncryptLdrs()
{
	Crypto::XeCryptRc4(ldr_sc_3bl_key, 0x10, &ldr_c[0x20], ldr_cbb_sc_3bl_size - 0x20, &ldr_c[0x20]);
	Crypto::XeCryptRc4(ldr_cd_sd_4bl_key, 0x10, &ldr_d[0x20], ldr_cd_sd_4bl_size - 0x20, &ldr_d[0x20]);
	Crypto::XeCryptRc4(ldr_ce_se_5bl_key, 0x10, &ldr_e[0x20], ldr_ce_se_5bl_size - 0x20, &ldr_e[0x20]);
}

VOID BuildBootBlk()
{
	boot_blk_sz = ldr_cba_sb_2bl_size + ldr_cbb_sc_3bl_size + ldr_cd_sd_4bl_size + ldr_ce_se_5bl_size;
	boot_blk = (PBYTE)malloc(boot_blk_sz);
	memcpy(boot_blk, &flash[ldr_cba_sb_2bl_offs], ldr_cba_sb_2bl_size + ldr_cbb_sc_3bl_size);
	memcpy(&boot_blk[ldr_cba_sb_2bl_size + ldr_cbb_sc_3bl_size], ldr_c, ldr_cbb_sc_3bl_size);
	memcpy(&boot_blk[ldr_cba_sb_2bl_size + ldr_cbb_sc_3bl_size + ldr_cbb_sc_3bl_size], ldr_d, ldr_cd_sd_4bl_size);
	memcpy(&boot_blk[ldr_cba_sb_2bl_size + ldr_cbb_sc_3bl_size + ldr_cbb_sc_3bl_size + ldr_cd_sd_4bl_size], ldr_e, ldr_ce_se_5bl_size);
	memcpy(&flash[ldr_cba_sb_2bl_offs], boot_blk, boot_blk_sz);
}
*/

int main(int argc, char* argv[])
{
	// usage
	if(argc != 4){
		printf("Usage: XDKbuild v0.06b [input image file] [1bl_key] [sc_file]\n");
		printf("By Xvistaman2005\n");
		return ERR_NOT_ENOUGH_ARGS;
	}

	// flash file vars
	printf("XDKbuild v0.06b By Xvistaman2005\n");

	// parse 1BL key argument
	utils::HexDecode(argv[2], globals::_1BL_KEY);
	
	//open the flash file and unecc image
	FILE* f = fopen(argv[1], "rb");
	
	if(f == NULL) {
		printf("Could not open input image file\n");
		return ERR_CANT_OPEN_INPUT_FILE;
	}

	FlashImage fi(f);

	fclose(f);

	/*
	f_sz = utils::GetFileSize(f);

	f_pgs = f_sz / 528;
	flash = (PBYTE)malloc(f_pgs * 512);
	ecc = (PBYTE)malloc(f_pgs * 16);
	memset(ecc, 0x0, f_pgs * 16);

	if(f_sz == FLASH_4_GB)
		fread(flash, f_sz, 0x01, f);
	else if (f_sz == FLASH_16_MB || f_sz == FLASH_256_512_MB) {
		while(i != f_sz) {
			fread(&flash[x], 512, 0x01, f);
			fread(&ecc[y], 16, 0x01, f);
			i += 528;
			x += 512;
			y += 16;
		}
	} else {
		printf("Image doesn't appear to be a valid Xbox 360 Image\n");
		printf("Valid Sizes Are 17301504 = 16mb nand, 69206016==Big Block Image 256mb/512mb, 50331648 = Corona 4Gb eMMC\n");
		return ERR_INVALID_IMAGE_SIZE;
	}
	
	fclose(f);

	printf("Reading Image File %s\n", argv[1]);
	
	// test ecc bytes for v1 or v2	
	if(ecc[0x210] == 0x01) {
		printf("Image Type: Small Block On Small Block Controller\n\n");
		sfc = SFC_SMALL_ON_SMALL;
	}
	if(ecc[0x211] == 0x01) {
		printf("Image Type: Small Block On Big Block Controller\n");
		sfc = SFC_SMALL_ON_BIG;
	}
	if(ecc[0x1010] == 0xFF && ecc[0x1011] == 0x01) {
		printf("Image Type: Big Block On Big Block Controller\n");
		sfc = SFC_BIG_ON_BIG;
	}
	if(f_sz == 50331648) {
		printf("Image Type: eMMC Controller\n");
		sfc = SFC_EMMC;
	}

	//get keys from command line and conver then to hex data
	sscanf(argv[2], "%s", bl_string);
	utils::HexDecode((PCHAR)bl_key, bl_string);
	printf("Setting 1BL key as: %s\n", argv[2]);

	//get loction of all ldrs in flash
	GetLdrHdrs();
	printf("Locating Bootloaders\n");

	//setup buffers for decrypted ldrs
	ldr_d = (PBYTE)malloc(ldr_cd_sd_4bl_size);
	ldr_e = (PBYTE)malloc(ldr_ce_se_5bl_size);

	// create all ldr keys to crpyt ldrs with   
	// GetLdrKeysRetail();
	// printf("Calculating Retail Decryption Keys\n");
	
	//read in the sc file from the file name on the cmd line
	FILE* scf = fopen(argv[3], "rb");

	if(scf == NULL) {
		printf("Could not open SC bootloader file\n");
		return ERR_CANT_OPEN_SC_FILE;
	}

	ldr_cbb_sc_3bl_size = utils::GetFileSize(scf);

	//setup sc buffer
	ldr_c = (PBYTE)malloc(ldr_cbb_sc_3bl_size);
	fread(ldr_c, ldr_cbb_sc_3bl_size, 0x01, scf);
	fclose(scf);
	printf("Reading SC Bootloader File: %s\n", argv[3]);

	//decrypt the ldrs
	DecryptLdrs();
	printf("Decrypting Bootloaders\n");

	//copy the decrpyted SD SE to there own buffers for crypto
	memcpy(ldr_d, &flash[ldr_cd_sd_4bl_offs], ldr_cd_sd_4bl_size);
	memcpy(ldr_e, &flash[ldr_ce_se_5bl_offs], ldr_ce_se_5bl_size);

	//setup devkit crypto keys for SB SC SD SE ldrs
	GetLdrKeysDevkit();
	printf("Calculating Devkit Encryption Keys\n");

	//encrypt the ldrs with devkit crypto keys
	EncryptLdrs();
	printf("Encrypting Bootloaders\n");

	//create new ldr chain    
	BuildBootBlk();
	printf("Building New Devkit Bootchain\n");


	//set vufses cf ldv to 0x0 and print the vfuses
	FixFuses();
	//	dump_buffer_hex("ecc.bin", ecc, y);
	//fix up the ecc bytes ad readd to image

	int ii = 0;
	int xx = 0;
	int yy = 16;
	DWORD sha_exp = 0x9D0AC37B;
	DWORD sha_calc;
	DWORD blk_num_a = 0;
	DWORD blk_num_b = 0;
	DWORD pg_ctr = 0;
	DWORD blk_ctr_b = 0; 
	DWORD out_sz = 0;

	FILE* ff = fopen(argv[1], "r+b");
   
	if(f == NULL) {
		printf("Could not open output image file\n");
		return ERR_CANT_OPEN_OUTPUT_FILE;
	}

	if(sfc == SFC_EMMC)
		fwrite(&flash[xx], f_sz, 1, ff);
	else {
		while(out_sz != 1310720) {
			Crypto::XeCryptSha(&flash[xx], 512, NULL, 0, NULL, 0, ecc_hash, 0x10);
			// memcpy(ecc_hash, sha, 0x10); 
			sha_calc = bswap32(*(PDWORD)ecc_hash);
			//   blk_num = getBeU32(&ecc[yy]);
			//    blk_num = blk_num>>16;
			if(sha_calc != sha_exp) {
				//    printf("Checking ECC Spare At Block Nmber %X\n",blk_num_a);
				memset(new_ecc, 0x0, 16);
			
				if(sfc == SFC_SMALL_ON_SMALL || sfc == SFC_SMALL_ON_BIG) {
					if(pg_ctr == 32) {
						blk_num_a++;
						pg_ctr = 0;
					}
					if(blk_num_a == 0xff) {
						blk_num_b++;
						blk_num_a = 0;
					}
				}
				if(sfc == SFC_BIG_ON_BIG) {
					if(pg_ctr == 256) {
						blk_num_a++;
						pg_ctr = 0;
					}
				}
				if(sfc == SFC_SMALL_ON_SMALL) {
					new_ecc[0] = blk_num_a;
					new_ecc[1] = blk_num_b;
					new_ecc[5] = 0xFF;
				}
				if(sfc == SFC_SMALL_ON_BIG) {
					new_ecc[1] = blk_num_a;
					new_ecc[2] = blk_num_b;
					new_ecc[5] = 0xFF;
				}
				if(sfc == SFC_BIG_ON_BIG) {
					new_ecc[0] = 0xFF;
					new_ecc[1] = blk_num_a;
				}
					
				FixPageEcc(&flash[xx], new_ecc);
				fwrite(&flash[xx], 512, 1, ff);
				fwrite(&new_ecc, 16, 1, ff);
				pg_ctr++;
				xx += 512;
				yy += 16;
				out_sz += 512;
			} else {
				//    printf("Empty Page Found In Image Skipping ECC Check at Block Number\n", blk_num_a);
				fwrite(&flash[xx], 512, 1, ff);
				memset(new_ecc, 0xFF, 16);
				fwrite(new_ecc, 16, 1, ff);
				
				if(sfc == SFC_SMALL_ON_SMALL || sfc == SFC_SMALL_ON_BIG) {
					if(pg_ctr == 32) {
						blk_num_a++;
						pg_ctr = 0;
					}
					
					if(blk_num_a == 0xFF) {
						blk_num_b++;
						blk_num_a = 0;
					}
				}	
				
				if(sfc == SFC_BIG_ON_BIG) {
					if(pg_ctr==256) {
						blk_num_a++;
						pg_ctr = 0;
					}
				}
				
				pg_ctr++;
				xx += 512;
				yy += 16;
				out_sz += 512;
			}
		}
	}

	printf("Writing Final Image To File: %s\n", argv[1]);

	fclose(ff);
	
	free(flash);
	free(ecc);
	free(boot_blk);
	free(ldr_c);
	free(ldr_d);
	free(ldr_e);
	*/

	// free(flash);
	// free(ecc);
		
	return ERR_NONE;
}
