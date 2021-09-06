#ifndef _TYPES_H
#define _TYPES_H
#define _ES16(val) \
	((u16)(((((u16)val) & 0xff00) >> 8) | \
	       ((((u16)val) & 0x00ff) << 8)))

#define bswap16(x) (((x&0xFF)<<8)+(((x&0xFF00)>>8)))
#define bswap32(x) ((((x&0xFF)<<24))+(((x&0xFF00)<<8))+(((x&0xFF0000)>>8))+(((x&0xFF000000)>>24)))
#define bswap64(x) (_byteswap_uint64(x))

// primitives
typedef unsigned char		u8, UCHAR, BYTE;
typedef unsigned short		u16, USHORT, WORD;
typedef unsigned int		u32, BOOL, UINT, DWORD;
typedef unsigned long long	u64, ULONG, QWORD;
typedef char			s8, CHAR;
typedef short			s16;
typedef int			s32;
typedef long long		s64;

// pointers
typedef CHAR*  PCHAR;
typedef BYTE*  PBYTE, PUCHAR;
typedef WORD*  PWORD;
typedef DWORD* PDWORD;
typedef QWORD* PQWORD;


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif


#endif // _TYPES_H
