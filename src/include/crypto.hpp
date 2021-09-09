#ifndef _CRYPTO_HPP
#define _CRYPTO_HPP

class Crypto {
    public:
        static VOID XeCryptSha(PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut);
        static VOID XeCryptHmacSha(PBYTE pbKey, DWORD cbKey, PBYTE pbIn0, DWORD cbIn0, PBYTE pbIn1, DWORD cbIn1, PBYTE pbIn2, DWORD cbIn2, PBYTE pbOut, DWORD cbOut);
        static VOID XeCryptRc4(PBYTE pbKey, DWORD cbKey, PBYTE pbIn, DWORD cbIn, PBYTE pbOut);
};
#endif