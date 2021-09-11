#ifndef _TYPES_HPP
#define _TYPES_HPP

// defines
// es = endian swap
#define es16(x) x = bswap16(x)
#define es32(x) x = bswap32(x)
#define es64(x) x = bswap64(x)

// primitives
typedef void               VOID;
typedef unsigned char	   u8, UCHAR, BYTE;
typedef unsigned short	   u16, USHORT, WORD;
typedef unsigned int	   u32, BOOL, UINT, DWORD;
typedef unsigned long long u64, ULONG, QWORD;
typedef char			   s8, CHAR;
typedef short			   s16;
typedef int			       s32;
typedef long long		   s64;

// constants
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define SHA_DIGEST_SIZE 0x14  // 20

#define PAGE_SIZE 528
#define PAGE_DATA_SIZE 512
#define PAGE_ECC_SIZE 16

#define FLASH_16_MB      17301504
#define FLASH_256_512_MB 69206016
#define FLASH_4_GB       50331648

// structs
struct FLASH_HDR {
	WORD  Magic;
	WORD  Build;
	WORD  QFE;
	WORD  Flags;
	DWORD EntryPoint;
	DWORD Size;
	BYTE  Copyright[0x40];
	BYTE  Padding[0x10];
	DWORD KvLength;
	DWORD PatchOffset;
	WORD  PatchSlots;
	WORD  KvVersion;
	DWORD KvOffset;
	DWORD PatchSlotSize;
	DWORD SmcConfigOffset;
	DWORD SmcLength;
	DWORD SmcOffset;
};

struct BL_HDR {
	WORD Magic;
	WORD Build;
	WORD QFE;
	WORD Flags;
	DWORD EntryPoint;
	DWORD Size;
};

struct BL_HDR_WITH_NONCE {
	WORD Magic;
	WORD Build;
	WORD QFE;
	WORD Flags;
	DWORD EntryPoint;
	DWORD Size;
	BYTE Nonce[0x10];
};

#define HV_HDR         BL_HDR_WITH_NONCE
#define CBA_SB_2BL_HDR BL_HDR_WITH_NONCE
#define CBB_SC_3BL_HDR BL_HDR_WITH_NONCE
#define CD_SD_4BL_HDR  BL_HDR_WITH_NONCE
#define CE_SE_5BL_HDR  BL_HDR_WITH_NONCE
#define CF_SF_6BL_HDR  BL_HDR_WITH_NONCE

// bootloader magic
enum BL_MAGIC {
	CA_1BL = 0x0342,
	CB_CBA_2BL = 0x4342,
	CC_CBB_3BL = 0x4343,
	CD_4BL = 0x4344,
	CE_5BL = 0x4345,
	CF_6BL = 0x4346,
	CG_7BL = 0x4347,
	SB_2BL = 0x5342,
	SC_3BL = 0x5343,
	SD_4BL = 0x5344,
	SE_5BL = 0x5345,
	SF_6BL = 0x5346,
	SG_7BL = 0x5347
};

// errors
enum {
	ERR_NONE = 0,
	ERR_NOT_ENOUGH_ARGS,
	ERR_CANT_OPEN_INPUT_FILE,
	ERR_INVALID_IMAGE_SIZE,
	ERR_CANT_OPEN_SB_FILE,
	ERR_CANT_OPEN_SC_FILE,
	ERR_CANT_OPEN_SD_FILE,
	ERR_CANT_OPEN_SE_FILE,
	ERR_CANT_OPEN_OUTPUT_FILE
};

// System Flash Controller (SFC) types
enum SFC_TYPE {
	SFC_SMALL_ON_SMALL = 0,
	SFC_SMALL_ON_BIG,
	SFC_BIG_ON_BIG,
	SFC_EMMC
};

// pointers
typedef FLASH_HDR* PFLASH_HDR;
typedef BL_HDR* PBL_HDR;
typedef BL_HDR_WITH_NONCE* PBL_HDR_WITH_NONCE;
typedef void*  PVOID, VOIDP;
typedef CHAR*  PCHAR;
typedef BYTE*  PBYTE, PUCHAR;
typedef WORD*  PWORD;
typedef DWORD* PDWORD;
typedef QWORD* PQWORD;

#endif // _TYPES_H
