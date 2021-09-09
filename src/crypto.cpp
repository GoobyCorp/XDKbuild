#include "stdafx.hpp"

VOID Crypto::XeCryptSha(PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut) {
	BYTE hash[SHA1::DIGESTSIZE];

	SHA1 h;
	h.Update(pbIn0, cbIn0);
	if(pbIn1 != NULL && cbIn1 > 0)
		h.Update(pbIn1, cbIn1);
	if(pbIn2 != NULL && cbIn2 > 0)
		h.Update(pbIn2, cbIn2);
	h.Final(hash);
	memcpy(pbOut, hash, cbOut);
}

VOID Crypto::XeCryptHmacSha(PBYTE pbKey, DWORD cbKey, PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut) {
	BYTE hash[HMAC<SHA1>::DIGESTSIZE];

	HMAC<SHA1> h(pbKey, cbKey);
	h.Update(pbIn0, cbIn0);
	if(pbIn1 != NULL && cbIn1 > 0)
		h.Update(pbIn1, cbIn1);
	if(pbIn2 != NULL && cbIn2 > 0)
		h.Update(pbIn2, cbIn2);
	h.Final(hash);
	memcpy(pbOut, hash, cbOut);
}

VOID Crypto::XeCryptRc4(PBYTE pbKey, DWORD cbKey, PBYTE pbIn, DWORD cbIn, PBYTE pbOut) {
	ARC4::Encryption enc;
    enc.SetKey(pbKey, cbKey);
	enc.ProcessData(pbOut, pbIn, cbIn);
}