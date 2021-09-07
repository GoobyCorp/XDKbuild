#include "stdafx.hpp"

int utils::char2int(char input) {
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	throw invalid_argument("Invalid input character specified!");
}

int utils::HexDecode(PCHAR pcHex, PBYTE pbOut) {
	if(strlen(pcHex) % 2 != 0)
		return 0;

	while(*pbOut && pcHex[1])
	{
		*(pbOut++) = (char2int(*pcHex) * 16) + char2int(pcHex[1]);
		pcHex += 2;
	}
	return 0;
}

DWORD utils::GetFileSize(FILE* fptr) {
	int len;
	if(fptr == NULL)
	{
		return 0;
	}
	fseek(fptr, 0 , SEEK_END);
	len = ftell(fptr);
	rewind(fptr);
	return len;
}

VOID utils::DumpBufferHex(PCHAR filename, PVOID buffer, int size) {
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

PBYTE utils::ReadFile(PCHAR fileName, PDWORD pdwSize) {
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

BOOL utils::WriteFile(PCHAR fileName, PBYTE buffer, DWORD size) {
	FILE* fo = fopen(fileName, "wb");
	if(fo != NULL) {
		if(fwrite(buffer, size, 1, fo) == size)
			return TRUE;
	}
	return FALSE;
}