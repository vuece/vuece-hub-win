#pragma once


#include <stdio.h>
#include <string>
#include <tchar.h>
#include <windows.h>
#include <Wincrypt.h>

using namespace std;

class VueceCrypt
{
public:
    VueceCrypt(void);
    virtual ~VueceCrypt(void) {
        if ( m_hKey )  CryptDestroyKey( m_hKey );
        if ( m_hHash ) CryptDestroyHash( m_hHash );
        if ( m_hProv ) CryptReleaseContext( m_hProv, 0);
    }
    BOOL SetKey( LPCSTR szKey= 0, LPCSTR pszSalt= 0 );

    BOOL EncryptDecrypt( BYTE* pData, DWORD* dwDataLen, LPCSTR pKey, BOOL fEncrypt );

    std::string EncryptStrToHex(   LPCSTR szText, LPCSTR pKey = 0, LPCSTR pszSalt = 0 );
    std::string DecryptStrFromHex( LPCSTR szHex,  LPCSTR pKey = 0, LPCSTR pszSalt = 0 );
    std::string EncodeToHex(   char* p, int nLen );
    int     DecodeFromHex( LPCSTR pSrc, BYTE* pDest, int nBufLen );

    int EncryptStrToBinary(const char* input,
    		char* result,
    		int resultBufSize,
    		LPCSTR pKey = 0,
    		LPCSTR pszSalt = 0
    		);

    int DecryptStrFromBinary( char* input, int input_len,  LPCSTR pKey = 0, LPCSTR pszSalt = 0 );

private:
    HCRYPTPROV  m_hProv;
    HCRYPTHASH  m_hHash;
    HCRYPTKEY   m_hKey;

    BOOL        m_fOK;
    DWORD       m_nLastErr;
    std::string     m_sErrMsg;
    char*       m_pszDefaultKeyRaw;
};
