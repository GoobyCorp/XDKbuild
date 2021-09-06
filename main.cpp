#include "stdafx.hpp"

PBYTE flash;
PBYTE ecc;
PBYTE boot_blk;
PBYTE ldr_d;
PBYTE ldr_e;
PBYTE ldr_c;
BYTE bl_key[0x10];
BYTE bl_string[0x20];
BYTE ldr_b_key[0x10];
BYTE zero_key[0x10];
BYTE ldr_bb_key[0x10];
BYTE ldr_d_key[0x10];
BYTE ldr_e_key[0x10];
BYTE ldr_c_key[0x10];
BYTE hmac[0x40];
BYTE sha[0x40];
BYTE ecc_hash[0x10];
BYTE new_ecc[0x10];


int i=0;
int f_sz;
int f_pgs;
int x=0;
int y=0;
int ldr_b_start;
int ldr_b_end;
int ldr_bb_start;
int ldr_bb_end;
int ldr_d_start;
int ldr_d_end;
int ldr_e_start;
int ldr_e_end;
int ldr_c_end;
int boot_blk_sz;
int sfc;

int char2int(char input)
{
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	// throw std::invalid_argument("Invalid input string");
}

int hex2data(PCHAR hex, PBYTE target)
{
	while(*hex && hex[1])
	{
		*(target++) = char2int(*hex) * 16 + char2int(hex[1]);
		hex += 2;
	}
	return 0;
}

/*
int hex2data(unsigned char *data, const unsigned char *hexstring, unsigned int len)
{
	unsigned const char *pos = hexstring;
	char *endptr;
	size_t count = 0;

	if ((hexstring[0] == '\0') || (strlen(hexstring) % 2))
	{
		//hexstring contains no data
		//or hexstring has an odd length
		return -1;
	}

	for(count = 0; count < len; count++)
	{
		char buf[5] = {'0', 'x', pos[0], pos[1], 0};
		data[count] = strtol(buf, &endptr, 0);
		pos += 2 * sizeof(char);

		if (endptr[0] != '\0')
		{
			//non-hexadecimal character encountered
			return -1;
		}

	}
	
	return 0;
}
*/

DWORD GetFileSize(FILE* fptr)
{
	int len;
	if(fptr == NULL)
	{
		return 0;
	}
	fseek(fptr, 0 , SEEK_END);
	len = ftell(fptr);
	rewind (fptr);
	return len;
}

void dump_buffer_hex(char* filename, void* buffer, int size)
{
	FILE* fptr;
	printf("writing 0x%x bytes to %s...", size, filename);
	if((buffer != NULL)&&(filename != NULL)&&(size != 0))
	{
		fptr = fopen(filename, "wb");
		if(fptr != NULL)
		{
			fwrite(buffer, size, 1, fptr);
			fclose(fptr);
		}
		else
		{
			printf("ERROR! Could not open file for writing!\n");
			return;
		}
	}
	else
	{
		printf("ERROR! Invalid args supplied to dump function!\n");
		return;
	}
	printf("done!\n");
}

PBYTE ReadFile(PCHAR fname, PDWORD pdwSize)
{
	FILE* fin;
	PBYTE buf = NULL;
	fin = fopen(fname, "rb");
	if(fin != NULL)
	{
		int sz = GetFileSize(fin);
		printf("loading file %s 0x%x bytes...", fname, sz);
		buf = (PBYTE)malloc(sz);
		if(buf != NULL)
		{
			fread(buf, sz, 1, fin);
			if(pdwSize != NULL)
				*pdwSize = sz;
			printf("done!\n");
		}
		else
			printf("failed to allocate 0x%x bytes!\n", sz);
		fclose(fin);
	}
	return buf;
}

void XeCryptSha(PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut)
{
	BYTE hash[0x14];
	sha1_context ctx;
	sha1_init(&ctx);
	sha1_starts(&ctx);
	sha1_update(&ctx, pbIn0, cbIn0);
	if(pbIn1 != NULL && cbIn1 != 0)
		sha1_update(&ctx, pbIn1, cbIn1);
	if(pbIn2 != NULL && cbIn2 != 0)
		sha1_update(&ctx, pbIn2, cbIn2);
	sha1_finish(&ctx, hash);
	sha1_free(&ctx);
	memcpy(pbOut, hash, cbOut);
}

void XeCryptHmacSha(PBYTE pbKey, DWORD cbKey, PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut) {
	BYTE hash[0x14];
	sha1_context ctx;
	sha1_init(&ctx);
	sha1_hmac_starts(&ctx, pbKey, cbKey);
	sha1_hmac_update(&ctx, pbIn0, cbIn0);
	if(pbIn1 != NULL && cbIn1 != 0)
		sha1_hmac_update(&ctx, pbIn1, cbIn1);
	if(pbIn2 != NULL && cbIn2 != 0)
		sha1_hmac_update(&ctx, pbIn2, cbIn2);
	sha1_hmac_finish(&ctx, hash);
	sha1_free(&ctx);
	memcpy(pbOut, hash, cbOut);
}

void XeCryptRc4(PBYTE pbKey, DWORD cbKey, PBYTE pbIn, DWORD cbIn, PBYTE pbOut) {
	arc4_context ctx;
	arc4_init(&ctx);
	arc4_setup(&ctx, pbKey, cbKey);
	arc4_crypt(&ctx, cbIn, pbIn, pbOut);
	arc4_free(&ctx);
}

unsigned int getPageEcc(unsigned char* datc, unsigned char* spare)
{
	unsigned int i=0, val=0, v=0;
	unsigned int* data = (unsigned int*) datc;
	for (i = 0; i < 0x1066; i++)
	{
		if (!(i & 31))
		{
			if (i == 0x1000)
				data = (unsigned int*)spare;
			v = ~*data++; // byte order: LE 
		}
		val ^= v & 1;
		v>>=1;
		if (val & 1)
			val ^= 0x6954559;
		val >>= 1;
	}
	return ~val;
}

void fixPageEcc(unsigned char* datc, unsigned char* spare)
{
	unsigned int val=getPageEcc(datc, spare);
	spare[12] = (spare[12]&0x3F)+((val << 6) & 0xC0);
	spare[13] = (val >> 2) & 0xFF;
	spare[14] = (val >> 10) & 0xFF;
	spare[15] = (val >> 18) & 0xFF;
	return;
}

void fixFuses()
{
	int patch_slot_start;
	int patch_slot_sz;
	int vfuse_start;
	int rows=0;
	int rowplus=0;
	int ctr=0;	
	unsigned char new_addr[4];
	unsigned long long testfuse;
	unsigned long long fuse;
   
	patch_slot_start=bswap_32(*(PDWORD)(flash + 0xC));
	patch_slot_sz=bswap_32(*(PDWORD)(flash + 0x70));
	vfuse_start=patch_slot_start+patch_slot_sz;
	
	testfuse=bswap_64(*(PDWORD)(flash + vfuse_start));
	if(testfuse != 0xc0ffffffffffffff && ctr != f_sz){
		while(testfuse != 0xc0ffffffffffffff){
			testfuse=bswap_64(*(PQWORD)(flash + ctr));
			ctr=ctr+4;}
			
		printf("Virtaul Fuses Found At 0x%X\n:", ctr-4);
		ctr=ctr-4;
		new_addr[0]=ctr-patch_slot_start >>24;
		new_addr[1]=ctr-patch_slot_start <<8 >>24;
		new_addr[2]=ctr-patch_slot_start <<16 >>24;
		new_addr[3]=ctr-patch_slot_start <<24 >>24;
		memcpy(&flash[0x70], new_addr, 4);}

	//	else{
	//		printf("Could not Find Virtual Fuses Make Sure You Built a Gitch2m Image");
	//		}
		patch_slot_sz=bswap_32(*(PDWORD)(flash + 0x70));
		vfuse_start=patch_slot_start+patch_slot_sz;
	
	memset(&flash[vfuse_start+56], 0, 40);
	
	printf("Virtual Fuses Set To:\n");    
	while(rows != 12)
	{
	   fuse=bswap_64(*(PQWORD)(flash + vfuse_start + rowplus));
	   printf("Fuseset %02d: %016llX\n", rows, fuse);
	   rows++;
	   rowplus += 8;         
	}  
	return;  
}

void getLdrHdrs()
{
	ldr_b_start=bswap_32(*(PDWORD)(flash + 0x8));
	ldr_b_end=bswap_32(*(PDWORD)(flash + ldr_b_start + 0xC));
	
	ldr_bb_start=ldr_b_start+ldr_b_end;
	ldr_bb_end=bswap_32(*(PDWORD)(flash + ldr_bb_start + 0xC)); 

	ldr_d_start=ldr_bb_start+ldr_bb_end;
	ldr_d_end=bswap_32(*(PDWORD)(flash + ldr_d_start + 0xC)); 

	ldr_e_start=ldr_d_start+ldr_d_end;
	ldr_e_end=bswap_32(*(PDWORD)(flash + ldr_e_start + 0xC));

	return;
}

void getLdrKeysRetail()
{
	XeCryptHmacSha(bl_key, 0x10, &flash[ldr_b_start+0x10], 0x10, NULL, 0, NULL, 0, ldr_b_key, 0x10);

	memset(zero_key, 0, 0x10);
	XeCryptHmacSha(ldr_b_key, 0x10, &flash[ldr_bb_start+0x10], 0x10, NULL, 0, NULL, 0, ldr_bb_key, 0x10);

	XeCryptHmacSha(ldr_bb_key, 0x10, &flash[ldr_d_start+0x10], 0x10, NULL, 0, NULL, 0, ldr_d_key, 0x10);

	XeCryptHmacSha(ldr_d_key, 0x10, &flash[ldr_e_start+0x10], 0x10, NULL, 0, NULL, 0, ldr_e_key, 0x10);
}

void decryptLdrs()
{
	//crypt_ldr(ldr_b_key, 0x10, ldr_b_end-0x20, &flash[ldr_b_start+0x20], &flash[ldr_b_start+0x20]);
	//crypt_ldr(ldr_bb_key, 0x10, ldr_bb_end-0x20, &flash[ldr_bb_start+0x20], &flash[ldr_bb_start+0x20]);
	XeCryptRc4(ldr_d_key, 0x10, &flash[ldr_d_start+0x20], ldr_d_end-0x20, &flash[ldr_d_start+0x20]);
	XeCryptRc4(ldr_e_key, 0x10, &flash[ldr_e_start+0x20], ldr_e_end-0x20, &flash[ldr_e_start+0x20]);
}

void getLdrKeysDevkit()
{
	XeCryptHmacSha(zero_key, 0x10, &ldr_c[0x10], 0x10, NULL, 0, NULL, 0, ldr_c_key, 0x10);

	XeCryptHmacSha(ldr_c_key, 0x10, &ldr_d[0x10], 0x10, NULL, 0, NULL, 0, ldr_d_key, 0x10);

	XeCryptHmacSha(ldr_d_key, 0x10, &ldr_e[0x10], 0x10, NULL, 0, NULL, 0, ldr_e_key, 0x10);
}

void encryptLdrs()
{
  XeCryptRc4(ldr_c_key, 0x10, &ldr_c[0x20], ldr_c_end-0x20, &ldr_c[0x20]);
  XeCryptRc4(ldr_d_key, 0x10, &ldr_d[0x20], ldr_d_end-0x20, &ldr_d[0x20]);
  XeCryptRc4(ldr_e_key, 0x10, &ldr_e[0x20], ldr_e_end-0x20, &ldr_e[0x20]);
}

void buildBootBlk()
{
	boot_blk_sz = ldr_b_end + ldr_bb_end + ldr_c_end + ldr_d_end + ldr_e_end;    
	boot_blk = (PBYTE)malloc(boot_blk_sz);
	memcpy(boot_blk, &flash[ldr_b_start], ldr_b_end + ldr_bb_end);
	memcpy(&boot_blk[ldr_b_end+ldr_bb_end], ldr_c, ldr_c_end);
	memcpy(&boot_blk[ldr_b_end+ldr_bb_end+ldr_c_end], ldr_d, ldr_d_end);
	memcpy(&boot_blk[ldr_b_end+ldr_bb_end+ldr_c_end+ldr_d_end], ldr_e, ldr_e_end);
	memcpy(&flash[ldr_b_start], boot_blk, boot_blk_sz);
}    


int main (int argc, char** argv)
{
	// usage
	if(argc != 4){
		printf("Usage: XDKbuild v0.05b [input image file] [1bl_key] [sc_file]\n");
		printf("By Xvistaman2005\n");
		exit(0);
	}
	// flash file vars
	printf("XDKbuild v0.05b By Xvistaman2005\n");
	
	//open the flash file and unecc image
	FILE* f = fopen(argv[1], "rb");
	
	if(f==NULL) {
		printf("Could not open input image file\n");
		exit(0);
	}    

	f_sz = GetFileSize(f);

	f_pgs = f_sz / 528;
	flash = (PBYTE)malloc(f_sz);
	ecc = (PBYTE)malloc(f_sz);
	memset(ecc, 0x0, f_sz);

	if(f_sz == 50331648)
		fread(flash, f_sz, 0x01, f);

	else if (f_sz==17301504||f_sz==69206016){
		while(i != f_sz) {
			fread(&flash[x], 512, 0x01, f);
			fread(&ecc[y], 16, 0x01, f);
			i += 528;
			x += 512;
			y += 16;
		}
	} else {
		printf("Image Does Not Seem To Be A Valid Size For A Xbox360 Image\n");
		printf("Valid Sizes Are 17301504=16mb nand, 69206016==Big Block Image 256mb/512mb, 50331648=Corona 4Gb eMMC\n");
		exit(0);
	}
	
	fclose(f);
	printf("Reading Image File %s\n", argv[1]);
	
	// test ecc bytes for v1 or v2	
	if(ecc[0x210] == 0x01){
		printf("Image Type: Small Block On Small Block Controller\n\n");
		sfc=1;}

	if(ecc[0x211] == 0x01){
		printf("Image Type: Small Block On Big Block Controller\n");
		sfc=2;}
		
	if(ecc[0x1010]==0xff&& ecc[0x1011]==0x01 ){
		printf("Image Type: Big Block On Big Block Controller\n");
		sfc=3;}
			
	if(f_sz==50331648){
		printf("Image Type: eMMC Controller\n");
		sfc=4;
	}		
		

	//get keys from command line and conver then to hex data
	sscanf(argv[2], "%s", bl_string);
	hex2data((PCHAR)bl_key, bl_string);
	printf("Setting 1BL key as: %s\n", argv[2]);

	//get loction of all ldrs in flash
	getLdrHdrs();
	printf("Locating Bootloaders\n");

	//setup buffers for decrypted ldrs
	ldr_d=(PBYTE)malloc(ldr_d_end);
	ldr_e=(PBYTE)malloc(ldr_e_end);

	// create all ldr keys to crpyt ldrs with   
	getLdrKeysRetail();
	printf("Calculating Retail Decryption Keys\n");
	
	//read in the sc file from the file name on the cmd line    
	FILE* scf;
	scf=fopen(argv[3], "rb");

	if(scf==NULL) {
	printf("Could not open SC bootloader file\n");
	exit(0);} 

	fseek(scf, 0, SEEK_END);
	ldr_c_end= ftell(scf);
	rewind(scf);

	//setup sc buffer
	ldr_c=(PBYTE)malloc(ldr_c_end);
	fread(ldr_c, ldr_c_end, 0x01, scf);
	fclose(scf);
	printf("Reading SC Bootloader File: %s\n", argv[3]);

	//decrypt the ldrs
	decryptLdrs();
	printf("Decrypting Bootloaders\n");

	//copy the decrpyted SD SE to there own buffers for crypto
	memcpy(ldr_d, &flash[ldr_d_start], ldr_d_end);
	memcpy(ldr_e, &flash[ldr_e_start], ldr_e_end); 

	//setup devkit crpyto keys for SC SD SE ldrs
	getLdrKeysDevkit();
	printf("Calculating Devkit Encryption Keys\n");	

	//encrypt the ldrs with devkit crypto keys
	encryptLdrs();
	printf("Encrypting Bootloaders\n");	

	//create new ldr chain    
	buildBootBlk();
	printf("Building New Devkit Bootchain\n");	


	//set vufses cf ldv to 0x0 and print the vfuses
	fixFuses();
	//	dump_buffer_hex("ecc.bin", ecc, y);
	//fix up the ecc bytes ad readd to image

	int ii=0;
	int xx=0;
	int yy=16;
	unsigned int sha_exp=0x9D0AC37B;
	unsigned int sha_calc;
	unsigned int blk_num_a=0;
	unsigned int blk_num_b=0;
	unsigned int pg_ctr=0;
	unsigned int blk_ctr_b=0; 
	unsigned int out_sz=0;

	FILE* ff = fopen(argv[1], "r+b");
   
	if(f == NULL) {
		printf("Could not open output image file\n");
		exit(0);
	}

	if(sfc == 4)
		fwrite(&flash[xx], f_sz, 1, ff);
	else {
		while(out_sz != 1310720) {
			XeCryptSha(&flash[xx], 512, NULL, 0, NULL, 0, ecc_hash, 0x10);
			// memcpy(ecc_hash, sha, 0x10); 
			sha_calc=bswap_32(*(PDWORD)ecc_hash);
			//   blk_num=getBeU32(&ecc[yy]);
			//    blk_num=blk_num>>16;
			if(sha_calc != sha_exp) {
				//    printf("Checking ECC Spare At Block Nmber %X\n",blk_num_a);
				memset(new_ecc, 0x0, 16);
			
				if(sfc==1||sfc==2){
				
				if(pg_ctr==32){
					blk_num_a++;
					pg_ctr=0;}
				
				if(blk_num_a==0xff){
					blk_num_b++;
					blk_num_a=0;}
				}
				
				if(sfc==3){
				
				if(pg_ctr==256){
					blk_num_a++;
					pg_ctr=0;}
				}
				
				if(sfc==1){
				new_ecc[0]=blk_num_a;
				new_ecc[1]=blk_num_b;
				new_ecc[5]=0xff;}
				
				if(sfc==2){
				new_ecc[1]=blk_num_a;
				new_ecc[2]=blk_num_b;
				new_ecc[5]=0xff;}
				
				if(sfc==3){
				new_ecc[0]=0xff;
				new_ecc[1]=blk_num_a;}
					
				fixPageEcc(&flash[xx], new_ecc);
				fwrite(&flash[xx], 512, 1, ff);
				fwrite(&new_ecc, 16, 1, ff);
				pg_ctr++;
				xx=xx+512;
				yy=yy+16;
				out_sz=out_sz+512;
			} else {
				//    printf("Empty Page Found In Image Skipping ECC Check at Block Number\n", blk_num_a);    
				fwrite(&flash[xx], 512, 1, ff);
				memset(new_ecc, 0xff, 16);
				fwrite(new_ecc, 16, 1, ff);
				
				if(sfc==1||sfc==2){
				
				if(pg_ctr==32){
					blk_num_a++;
					pg_ctr=0;}
				
				if(blk_num_a==0xff){
					blk_num_b++;
					blk_num_a=0;}
				}	
				
				if(sfc==3){
				
				if(pg_ctr==256){
					blk_num_a++;
					pg_ctr=0;}
				}
				
				pg_ctr++;
				xx=xx+512;
				yy=yy+16;
				out_sz=out_sz+512;
			}
		}
	}

	printf("Writing Final Image To File: %s\n", argv[1]);

	fclose(ff);
	
	free (flash);
	free (ecc);
	free (boot_blk);
	free (ldr_d);
	free (ldr_e);
	free (ldr_c);
		
	return 0; 
}
