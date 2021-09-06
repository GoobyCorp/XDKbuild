#include "stdafx.hpp"

VOID Crypto::XeCryptSha(PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut)
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

VOID Crypto::XeCryptHmacSha(PBYTE pbKey, DWORD cbKey, PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut) {
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

VOID Crypto::XeCryptRc4(PBYTE pbKey, DWORD cbKey, PBYTE pbIn, DWORD cbIn, PBYTE pbOut) {
	arc4_context ctx;
	arc4_init(&ctx);
	arc4_setup(&ctx, pbKey, cbKey);
	arc4_crypt(&ctx, cbIn, pbIn, pbOut);
	arc4_free(&ctx);
}