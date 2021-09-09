#include "stdafx.hpp"

int Utils::char2int(char input) {
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	throw invalid_argument("Invalid input character specified!");
}

VOID Utils::PrintHex(PBYTE data, DWORD size) {
	for(DWORD i = 0; i < size; i++) {
		printf("%02X", data[i]);
	}
	printf("\n");
}

int Utils::HexDecode(PCHAR pcHex, PBYTE pbOut) {
	if(strlen(pcHex) % 2 != 0)
		return 0;

	while(*pbOut && pcHex[1])
	{
		*(pbOut++) = (char2int(*pcHex) * 16) + char2int(pcHex[1]);
		pcHex += 2;
	}
	return 0;
}

DWORD Utils::GetFileSize(FILE* f) {
	int len;
	if(f == NULL)
	{
		return 0;
	}
	fseek(f, 0 , SEEK_END);
	len = ftell(f);
	rewind(f);
	return len;
}

VOID Utils::DumpBufferHex(PCHAR filename, PVOID buffer, int size) {
	FILE* fptr;
	printf("writing 0x%x bytes to %s...", size, filename);
	if((buffer != NULL) && (filename != NULL) && (size != 0))
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

PBYTE Utils::ReadFile(PCHAR fileName, PDWORD pdwSize) {
	FILE* fin;
	PBYTE buf = NULL;
	fin = fopen(fileName, "rb");
	if(fin != NULL)
	{
		int sz = GetFileSize(fin);
		printf("loading file %s 0x%x bytes...", fileName, sz);
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

BOOL Utils::WriteFile(PCHAR fileName, PBYTE buffer, DWORD size) {
	FILE* fo = fopen(fileName, "wb");
	if(fo != NULL) {
		if(fwrite(buffer, size, 1, fo) == size)
			return TRUE;
	}
	return FALSE;
}

DWORD Utils::GetPageEcc(PBYTE pbData, PBYTE pbSpare)
{
	DWORD i = 0, val = 0, v = 0;
	PDWORD data = (PDWORD)pbData;
	for (i = 0; i < 0x1066; i++)
	{
		if (!(i & 31))
		{
			if (i == 0x1000)
				data = (PDWORD)pbSpare;
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

VOID Utils::FixPageEcc(PBYTE pbData, PBYTE pbSpare)
{
	DWORD val = GetPageEcc(pbData, pbSpare);
	pbSpare[12] = (pbSpare[12] & 0x3F) + ((val << 6) & 0xC0);
	pbSpare[13] = (val >> 2) & 0xFF;
	pbSpare[14] = (val >> 10) & 0xFF;
	pbSpare[15] = (val >> 18) & 0xFF;
}