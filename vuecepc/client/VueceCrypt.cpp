#include "talk/base/logging.h"


#include "VueceCrypt.h"

#include <string>
#include <cstdio>
#include <iostream>
#include <limits>
#include <sstream>



VueceCrypt::VueceCrypt(void) {
	m_hProv = m_hHash = m_hKey = 0;
	m_pszDefaultKeyRaw = "a_G-?=\\/Qj<L^1R3$t%lDEgkEJU|=t|cJb/(AhsR.~lmhYKslvbh64~<Z0H|CP]W";
}

BOOL VueceCrypt::SetKey(LPCSTR szKey, LPCSTR szSalt/*=0*/) {
	m_fOK = TRUE;
	if (0 == m_hProv) {
		m_fOK = CryptAcquireContext(&m_hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	}

	if (m_fOK && (0 != m_hHash)) {
		m_fOK = CryptDestroyHash(m_hHash);
		m_hHash = 0;
	}

	if (m_fOK && (0 == m_hHash)) {
		m_fOK = CryptCreateHash(m_hProv, CALG_MD5, 0, 0, &m_hHash);
	}

	if (m_fOK) {

		if (0 == szKey) { // request to use default rawKey
			char szTmp[100];

			strcpy_s(szTmp, sizeof(szTmp), m_pszDefaultKeyRaw);

			if (szSalt) {
				strncat_s(szTmp, sizeof(szTmp), szSalt, 5); // use part of salt
			}

			// minor security tweak -- scramble the key+salt
			int nLen = strlen(szTmp) - 1;
			for (int j = 0; j < nLen; j++) {
				char c = szTmp[nLen - j];
				szTmp[nLen - j] = (char) (szTmp[j] + 5);
				szTmp[j] = c;
			}
			szKey = &szTmp[4]; // discard the first part, for fun
		}
		m_fOK = CryptHashData(m_hHash, (BYTE*) szKey, strlen(szKey), 0);
	}
	if (m_fOK) {
		m_fOK = CryptDeriveKey(m_hProv, CALG_RC4, m_hHash, CRYPT_EXPORTABLE, &m_hKey);
	}
	if (!m_fOK) {
		m_nLastErr = GetLastError();
		m_sErrMsg = "Error creating encryption key";
	}
	return (m_fOK);
}
//--- workhorse function:  Encrypt or decrypt "in place"
BOOL VueceCrypt::EncryptDecrypt(BYTE* pData, DWORD* dwDataLen, LPCSTR pKey, BOOL fEncrypt) {
	m_fOK = TRUE;
	SetKey((LPCSTR) pKey);
	if (fEncrypt) {
		m_fOK = CryptEncrypt(m_hKey, 0, TRUE, 0, pData, dwDataLen, *dwDataLen);
	} else {
		m_fOK = CryptDecrypt(m_hKey, 0, TRUE, 0, pData, dwDataLen);
	}
	return (m_fOK);
}

std::string VueceCrypt::EncryptStrToHex(LPCSTR szText, LPCSTR pszKey/*= 0*/, LPCSTR pszSalt/*= 0*/) {
	m_fOK = TRUE;
	//    CString sRet= "";
	std::string result;

	DWORD nDataLen = strlen(szText);

	if (pszSalt || pszKey || (0 == m_hKey)) {
		m_fOK = SetKey((LPCSTR) pszKey, pszSalt);
	}

	if (m_fOK) {

		char* pTmp = new char[nDataLen + 1];

		strncpy_s(pTmp, nDataLen + 1, szText, nDataLen + 1);

		m_fOK = CryptEncrypt(m_hKey, 0, TRUE, 0, (BYTE*) pTmp, &nDataLen, nDataLen);

		if (m_fOK) {

			result = EncodeToHex(pTmp, nDataLen);
		}

		delete pTmp;
	}

	return (result);
}

int VueceCrypt::EncryptStrToBinary(const char* szText, char* result, int resultBufSize, LPCSTR pszKey/*= 0*/, LPCSTR pszSalt/*= 0*/)
{

	m_fOK = TRUE;

	DWORD nDataLen = strlen(szText);

	LOG(INFO) << "VueceCrypt::EncryptStrToBinary, input text length: " << nDataLen << ", result buffer size: " << resultBufSize;

	if (pszSalt || pszKey || (0 == m_hKey)) {
		m_fOK = SetKey((LPCSTR) pszKey, pszSalt);
	}

	if (m_fOK) {

		strncpy_s(result, nDataLen + 1, szText, nDataLen + 1);

		//refer to http://msdn.microsoft.com/en-us/library/windows/desktop/aa379924(v=vs.85).aspx
		m_fOK = CryptEncrypt(
				m_hKey, //A handle to the encryption key.
				0,//A handle to a hash object.
				TRUE,//A Boolean value that specifies whether this is the last section in a series being encrypted.
				0, //is defined but reserved for future use.
				(BYTE*) result, //A pointer to a buffer that contains the plaintext to be encrypted. The plaintext in this buffer is overwritten with the ciphertext created by this function.
				&nDataLen, //A pointer to a DWORD value that , on entry, contains the length, in bytes, of the plaintext in the pbData buffer. On exit, this DWORD contains the length, in bytes, of the ciphertext written to the pbData buffer.
				resultBufSize   //Specifies the total size, in bytes, of the input pbData buffer.
				);

		if(m_fOK)
		{
			LOG(INFO) << "VueceCrypt::EncryptStrToBinary - Encryption is OK, encrypted data length: " << nDataLen;
			return nDataLen;
		}
	}

	return 0;

}

int VueceCrypt::DecryptStrFromBinary(char* input, int input_len, LPCSTR pszKey/*=0*/, LPCSTR pszSalt/*= 0*/) {
	m_fOK = TRUE;

	DWORD result_len = 0;

	if (pszSalt || pszKey || (0 == m_hKey))
	{
		m_fOK = SetKey((LPCSTR) pszKey, pszSalt);
	}

	LOG(INFO) << "VueceCrypt::DecryptStrFromBinary: input_len: " << input_len;

	if (m_fOK)
	{
		result_len = input_len;

		m_fOK = CryptDecrypt(
				m_hKey,
				0,
				TRUE,
				0,
				(BYTE*) input,
				&result_len
				);

		if (m_fOK)
		{
			LOG(INFO) << "VueceCrypt::DecryptStrFromBinary - Decryption is OK: result_len: " << result_len;
		}

	}

	return result_len;
}


std::string VueceCrypt::DecryptStrFromHex(LPCSTR szHex, LPCSTR pszKey/*=0*/, LPCSTR pszSalt/*= 0*/) {
	m_fOK = TRUE;
	//    CString sRet= "";
	std::string result;
	DWORD nDataLen = strlen(szHex);

	if (pszSalt || pszKey || (0 == m_hKey)) {
		m_fOK = SetKey((LPCSTR) pszKey, pszSalt);
	}

	if (m_fOK) {
		DWORD nDecryptLen = nDataLen / 2;
		char* pTmp = new char[nDecryptLen + 1];
		DecodeFromHex(szHex, (BYTE*) pTmp, nDecryptLen);
		m_fOK = CryptDecrypt(m_hKey, 0, TRUE, 0, (BYTE*) pTmp, &nDecryptLen);
		if (m_fOK) {
			result = pTmp;
		}
		delete pTmp;
	}
	return (result);
}

//--------------------------------------------------------
// inefficient but requires no explanation :-)
std::string VueceCrypt::EncodeToHex(char* p, int nLen) {
	std::ostringstream os;
	char buff[100];

	//    CString sRet, sTmp;
	for (int j = 0; j < nLen; j++) {
		//        sTmp.Format( "%02x", p[j] );
		//        sRet+= sTmp;
		sprintf(buff, "%02x", p[j]);
		//os.append(buff)
		std::string tmp(buff);
		os << tmp;
	}

	return os.str();
}

//---------------------------------------------------------
// returns length of decoded hex buffer
int VueceCrypt::DecodeFromHex(LPCSTR pSrc, BYTE* pDest, int nBufLen) {
	int nRet = 0;
	int nLen = strlen(pSrc);
	*pDest = 0;
	BYTE cIn1, cIn2, nFinal;
	for (int j = 0; j < nLen; j += 2) {
		cIn1 = (BYTE) toupper(*pSrc++);
		cIn1 -= '0';
		if (cIn1 > 9)
			cIn1 -= 7;
		cIn2 = (BYTE) toupper(*pSrc++);
		cIn2 -= '0';
		if (cIn2 > 9)
			cIn2 -= 7;
		nFinal = (BYTE)((cIn1 << 4) | cIn2);
		if (nFinal > 255)
			nFinal = 0; // in case trying to decode non-hex data
		*pDest++ = nFinal;
		*pDest = 0;
		if (nRet >= nBufLen) {
			break;
		}
		nRet++;
	}
	return (nRet);
}
