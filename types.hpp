#ifndef _TYPES_H
#define _TYPES_H

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
