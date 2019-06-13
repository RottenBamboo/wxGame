#pragma once
#include "SceneManager.h"
#include<regex>
#include<iostream>
#include <fstream>
#include <filesystem>

namespace wxGame
{
	std::vector<SceneManager*> g_vecpSceneManager;
	template<typename T> class ParseText
	{
		T* m_data;
		size_t size;
		ParseText() :size = 0, m_data = nullptr; = default;

		ParseText(T* data)
		{
			m_data = data;
		}

		~ParseText()
		{
			if (m_data)
			{
				m_data = nullptr;
			}
		};
	};

	typedef unsigned long       DWORD;
	typedef int                 BOOL;
	typedef unsigned char       BYTE;
	typedef unsigned short      WORD;
	typedef float               FLOAT;
	typedef FLOAT               *PFLOAT;

	typedef int                 INT;
	typedef unsigned int        UINT;
	typedef unsigned int        *PUINT;

	// Unsigned base types.
	typedef unsigned __int8	BYTE;		// 8-bit  unsigned.
	typedef unsigned __int16	WORD;		// 16-bit unsigned.
	typedef unsigned __int32	UINT;		// 32-bit unsigned.
	typedef unsigned long		DWORD;		// defined in windows.h
	typedef unsigned __int64	QWORD;		// 64-bit unsigned.

	// Signed base types.
	typedef signed __int8		SBYTE;		// 8-bit  signed.
	typedef signed __int16		SWORD;		// 16-bit signed.
	typedef signed __int32 	INT;			// 32-bit signed.
	typedef long				LONG;		// defined in windows.h
	typedef signed __int64		SQWORD;		// 64-bit signed.

	// Character types.
	typedef char				ANSICHAR;	// An ANSI character. normally a signed type.
	typedef wchar_t			UNICHAR;		// A unicode character. normally a signed type.
	typedef wchar_t			WCHAR;		// defined in windows.h

	// Other base types.
	typedef UINT				UBOOL;		// Boolean 0 (false) or 1 (true).
	typedef float				FLOAT;		// 32-bit IEEE floating point.
	typedef double			DOUBLE;		// 64-bit IEEE double.

#ifndef __TCHAR_DEFINED
	typedef char            _TCHAR;
	typedef signed char     _TSCHAR;
	typedef unsigned char   _TUCHAR;
	typedef char            _TXCHAR;
	typedef int             _TINT;
#define __TCHAR_DEFINED
#endif

#ifndef _TCHAR_DEFINED
#if     !__STDC__
	typedef wchar_t            TCHAR;
#endif
#define _TCHAR_DEFINED
#endif
	// !UNICODE

#ifdef UNICODE
#define lstrlen  lstrlenW
#else
#define lstrlen  lstrlenA
#endif // !UNICODE

	class FUTF8ToTCHAR_Convert
	{
	public:
		FUTF8ToTCHAR_Convert()
		{
		}

		enum { UNICODE_BOGUS_CHAR_CODEPOINT = '?' };

		// I wrote this function for originally for PhysicsFS. --ryan.
		// !!! FIXME: Maybe this shouldn't be inline...
		static DWORD utf8codepoint(const ANSICHAR **_str)
		{
			const char *str = *_str;
			DWORD retval = 0;
			DWORD octet = (DWORD)((BYTE)*str);
			DWORD octet2, octet3, octet4;

			if (octet == 0)  // null terminator, end of string.
				return 0;

			else if (octet < 128)  // one octet char: 0 to 127
			{
				(*_str)++;  // skip to next possible start of codepoint.
				return(octet);
			}

			else if ((octet > 127) && (octet < 192))  // bad (starts with 10xxxxxx).
			{
				// Apparently each of these is supposed to be flagged as a bogus
				//  char, instead of just resyncing to the next valid codepoint.
				(*_str)++;  // skip to next possible start of codepoint.
				return UNICODE_BOGUS_CHAR_CODEPOINT;
			}

			else if (octet < 224)  // two octets
			{
				octet -= (128 + 64);
				octet2 = (DWORD)((BYTE) *(++str));
				if ((octet2 & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				*_str += 2;  // skip to next possible start of codepoint.
				retval = ((octet << 6) | (octet2 - 128));
				if ((retval >= 0x80) && (retval <= 0x7FF))
					return retval;
			}

			else if (octet < 240)  // three octets
			{
				octet -= (128 + 64 + 32);
				octet2 = (DWORD)((BYTE) *(++str));
				if ((octet2 & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet3 = (DWORD)((BYTE) *(++str));
				if ((octet3 & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				*_str += 3;  // skip to next possible start of codepoint.
				retval = (((octet << 12)) | ((octet2 - 128) << 6) | ((octet3 - 128)));

				// There are seven "UTF-16 surrogates" that are illegal in UTF-8.
				switch (retval)
				{
				case 0xD800:
				case 0xDB7F:
				case 0xDB80:
				case 0xDBFF:
				case 0xDC00:
				case 0xDF80:
				case 0xDFFF:
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}

				// 0xFFFE and 0xFFFF are illegal, too, so we check them at the edge.
				if ((retval >= 0x800) && (retval <= 0xFFFD))
					return retval;
			}

			else if (octet < 248)  // four octets
			{
				octet -= (128 + 64 + 32 + 16);
				octet2 = (DWORD)((BYTE) *(++str));
				if ((octet2 & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet3 = (DWORD)((BYTE) *(++str));
				if ((octet3 & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet4 = (DWORD)((BYTE) *(++str));
				if ((octet4 & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				*_str += 4;  // skip to next possible start of codepoint.
				retval = (((octet << 18)) | ((octet2 - 128) << 12) |
					((octet3 - 128) << 6) | ((octet4 - 128)));
				if ((retval >= 0x10000) && (retval <= 0x10FFFF))
					return retval;
			}

			// Five and six octet sequences became illegal in rfc3629.
			//  We throw the codepoint away, but parse them to make sure we move
			//  ahead the right number of bytes and don't overflow the buffer.

			else if (octet < 252)  // five octets
			{
				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				*_str += 5;  // skip to next possible start of codepoint.
				return UNICODE_BOGUS_CHAR_CODEPOINT;
			}

			else  // six octets
			{
				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				octet = (DWORD)((BYTE) *(++str));
				if ((octet & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
					return UNICODE_BOGUS_CHAR_CODEPOINT;

				*_str += 6;  // skip to next possible start of codepoint.
				return UNICODE_BOGUS_CHAR_CODEPOINT;
			}

			return UNICODE_BOGUS_CHAR_CODEPOINT;  // catch everything else.
		}


		/**
		 * Converts the string to the desired format. Allocates memory if the
		 * specified destination buffer isn't large enough
		 *
		 * @param Source The source string to convert
		 * @param Dest the destination buffer that holds the converted data
		 * @param Size the size of the dest buffer in bytes
		 */
		TCHAR* Convert(const ANSICHAR* Source, TCHAR* Dest, DWORD Size)
		{
			// Determine whether we need to allocate memory or not
			DWORD Length = (DWORD)strlen(Source) + 1;
			if (Length > Size)
			{
				// Need to allocate memory because the string is too big
				Dest = new TCHAR[Length * sizeof(TCHAR)];
			}

			TCHAR *retval = Dest;

			// Now do the conversion
			// You have to do this even if !UNICODE, since high-ASCII chars
			//  become multibyte. If you aren't using UNICODE and aren't using
			//  a Latin1 charset, you are just screwed, since we don't handle
			//  codepages, etc.
			while (--Length)
			{
				DWORD cp = utf8codepoint(&Source);
				if (cp == 0)
				{
					break;  // end of string, we're done.
				}

				// Please note that we're truncating this to a UCS-2 Windows TCHAR.
				//  A UCS-4 Unix wchar_t can hold this, and we're ignoring UTF-16 for now.
				else if (cp > 0xFFFF)
				{
					cp = UNICODE_BOGUS_CHAR_CODEPOINT;
				}

				*(Dest++) = cp;
			}
			*Dest = 0;

			return retval;
		}

		/* return the string length without the null terminator */
		UINT Length(TCHAR* Dest)
		{
			return (UINT)wcslen(Dest);
		}
	};

	/**
	 * Class takes one type of string and converts it to another. The class includes a
	 * chunk of presized memory of the destination type. If the presized array is
	 * too small, it mallocs the memory needed and frees on the class going out of
	 * scope. Uses the overloaded cast operator to return the converted data.
	 */
	template<typename CONVERT_TO, typename CONVERT_FROM, typename BASE_CONVERTER,
		DWORD DefaultConversionSize = 128>
		class TStringConversion :
		public BASE_CONVERTER
	{
		/**
		 * Holds the converted data if the size is large enough
		 */
		CONVERT_TO Buffer[DefaultConversionSize];
		/**
		 * Points to the converted data. If this pointer doesn't match Buffer, then
		 * memory was allocated and needs to be freed.
		 */
		CONVERT_TO* ConvertedString;

		/** Hide the default ctor */
		TStringConversion();

	public:
		/**
		 * Converts the data by using the Convert() method on the base class
		 */
		explicit inline TStringConversion(const CONVERT_FROM* Source)
		{
			if (Source != NULL)
			{
				// Use base class' convert method
				ConvertedString = BASE_CONVERTER::Convert(Source, Buffer, DefaultConversionSize);
			}
			else
			{
				ConvertedString = NULL;
			}
		}

		/**
		 * If memory was allocated, then it is freed below
		 */
		inline ~TStringConversion()
		{
			if (ConvertedString != NULL)
			{
				// Make the string empty so people don't hold onto pointers
				*ConvertedString = 0;
				// Free the memory if it was allocated in order to do the conversion
				if (ConvertedString != Buffer)
				{
					delete[] ConvertedString;
				}
			}
		}

		/** Operator to get access to the converted string */
		inline operator CONVERT_TO*(void) const
		{
			return ConvertedString;
		}

		/** Length function (returns the length of the string in number of CONVERT_TO units, excluding the NULL terminator */
		inline UINT Length()
		{
			return ConvertedString ? BASE_CONVERTER::Length(ConvertedString) : 0;
		}
	};

	class FTCHARToUTF8_Convert
	{
	public:
		FTCHARToUTF8_Convert()
		{
		}

		enum { UNICODE_BOGUS_CHAR_CODEPOINT = '?' };

		// I wrote this function for originally for PhysicsFS. --ryan.
		// !!! FIXME: Maybe this shouldn't be inline...
		static void utf8fromcodepoint(DWORD cp, ANSICHAR **_dst, DWORD *_len)
		{
			char *dst = *_dst;
			INT len = *_len;

			if (len == 0)
				return;

			if (cp > 0x10FFFF)   // No Unicode codepoints above 10FFFFh, (for now!)
				cp = UNICODE_BOGUS_CHAR_CODEPOINT;
			else if ((cp == 0xFFFE) || (cp == 0xFFFF))  // illegal values.
				cp = UNICODE_BOGUS_CHAR_CODEPOINT;
			else
			{
				// There are seven "UTF-16 surrogates" that are illegal in UTF-8.
				switch (cp)
				{
				case 0xD800:
				case 0xDB7F:
				case 0xDB80:
				case 0xDBFF:
				case 0xDC00:
				case 0xDF80:
				case 0xDFFF:
					cp = UNICODE_BOGUS_CHAR_CODEPOINT;
				}
			}

			// Do the encoding...
			if (cp < 0x80)
			{
				*(dst++) = (char)cp;
				len--;
			}

			else if (cp < 0x800)
			{
				if (len < 2)
					len = 0;
				else
				{
					*(dst++) = (char)((cp >> 6) | 128 | 64);
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 2;
				}
			}

			else if (cp < 0x10000)
			{
				if (len < 3)
					len = 0;
				else
				{
					*(dst++) = (char)((cp >> 12) | 128 | 64 | 32);
					*(dst++) = (char)((cp >> 6) & 0x3F) | 128;
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 3;
				}
			}

			else
			{
				if (len < 4)
					len = 0;
				else
				{
					*(dst++) = (char)((cp >> 18) | 128 | 64 | 32 | 16);
					*(dst++) = (char)((cp >> 12) & 0x3F) | 128;
					*(dst++) = (char)((cp >> 6) & 0x3F) | 128;
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 4;
				}
			}

			*_dst = dst;
			*_len = len;
		}


		/**
		 * Converts the string to the desired format. Allocates memory if the
		 * specified destination buffer isn't large enough
		 *
		 * @param Source The source string to convert
		 * @param Dest the destination buffer that holds the converted data
		 * @param Size the size of the dest buffer in bytes
		 */
		ANSICHAR* Convert(const TCHAR* Source, ANSICHAR* Dest, DWORD Size)
		{
			// Determine whether we need to allocate memory or not
			DWORD LengthW = (DWORD)wcslen(Source) + 1;

			// Needs to be 4x the wide in case each converted char is multibyte
			DWORD LengthA = LengthW * 4;

			if (LengthA > Size)
			{
				// Need to allocate memory because the string is too big
				Dest = new char[LengthA * sizeof(ANSICHAR)];
			}

			ANSICHAR *retval = Dest;

			// Now do the conversion
			// You have to do this even if !UNICODE, since high-ASCII chars
			//  become multibyte. If you aren't using UNICODE and aren't using
			//  a Latin1 charset, you are just screwed, since we don't handle
			//  codepages, etc.
			while (--LengthW)
			{
				const DWORD cp = (DWORD) *(Source++);
				if (cp == 0) break;
				utf8fromcodepoint(cp, &Dest, &LengthA);
			}
			*Dest = 0;

			return retval;
		}

		/* return the string length without the null terminator */
		UINT Length(ANSICHAR* Dest)
		{
			return (UINT)strlen(Dest);
		}
	};

#include <io.h>
#include <stdlib.h>
	typedef TStringConversion<ANSICHAR, TCHAR, FTCHARToUTF8_Convert> FTCHARToUTF8;
	typedef TStringConversion<TCHAR, ANSICHAR, FUTF8ToTCHAR_Convert> FUTF8ToTCHAR;
#define UTF8_TO_TCHAR(str) (TCHAR*)FUTF8ToTCHAR((const ANSICHAR*)str)
#define TCHAR_TO_UTF8(str) (ANSICHAR*)FTCHARToUTF8((const TCHAR*)str)
	namespace fs = std::experimental::filesystem;
	static const std::string FileName("UnitData.ini");
	static const std::string Path("..\\Debug\\");
	static const std::string PathInto("\\");
	static const std::wstring MatchStringTest1(L"(HeroIconPos[\\s]*=[\\s]*)([0-9]{1,2})");
	static const std::wstring MatchStringTest2(L"MinimapCamp");

	static void ReplaceWstring1(std::wstring& replaceWstr, wchar_t& CoordX, wchar_t& CoordY)
	{
		std::wstring replaceWstring(L"MinimapCamp = {\n\'ÓÑ·½\' = \"" + std::wstring(&CoordX) + \
			L"," + std::wstring(&CoordY) + L";12,12\",\n\'µÐ·½\' = \""\
			+ std::wstring(&CoordX) + L"," + std::wstring(&CoordY) + L";13,12\",\n}");
		int a = 0;
		int b = 1;
		replaceWstr = replaceWstring;
	}

	static void ReplaceWstring2(std::wstring& replaceWstr, wchar_t& CoordX, wchar_t& CoordY)
	{
		std::wstring replaceWstring(L"MinimapIcon");
		replaceWstr = replaceWstring;
	}

	static void LoadUTF8(const char* filename, bool pureBinary)
	{
		std::ifstream fin(filename, std::ios::in);
		fin.seekg(0, std::ios_base::end); // offset to end position
		std::ifstream::pos_type end = fin.tellg();
		size_t size = (int)end + 1; //end of the pBuffer is '\0', so size of the pBuffer should be one more
		char *pBuffer = new char[size];
		memset(pBuffer, 0, size);
		fin.seekg(0, std::ios_base::beg);
		fin.read(pBuffer, size);
		std::wstring trBuffer = UTF8_TO_TCHAR(pBuffer);
		size_t len = wcslen(trBuffer.c_str());
		fin.close();
		std::wsmatch result;
		std::wregex wmatch_ogex(MatchStringTest1.c_str());
		bool may = std::regex_search(trBuffer, result, wmatch_ogex);
		if (may)
		{
			int num = _wtoi(result[2].str().c_str());
			int CoordX = 0, CoordY = 0;
			CoordX = num % 16;
			CoordY = num / 16;
			std::wstring strReplace;
			wchar_t* X = new wchar_t[10];
			wchar_t* Y = new wchar_t[10];
			_itow_s(CoordX, X, 10, 10);
			_itow_s(CoordY, Y, 10, 10);
			ReplaceWstring1(strReplace, *X, *Y);
			trBuffer = regex_replace(trBuffer, wmatch_ogex, strReplace);
			std::string finalBuffer = TCHAR_TO_UTF8(trBuffer.c_str());
			std::ofstream fout(filename, std::ios::out);
			fout.write(finalBuffer.c_str(), finalBuffer.size());
			fout.close();
		}
		delete[] pBuffer;
		pBuffer = nullptr;
	}

	void load()
	{
		for (auto& it : fs::directory_iterator(Path))
		{
			auto itp = it.path();
			auto outter = itp.filename();
			const std::string all = Path + outter.string();
			for (auto& it : fs::directory_iterator(all))
			{
				auto itp = it.path();
				auto inner = itp.filename();
				if (inner.string() == FileName)
				{
					std::string all_path = all + PathInto + inner.string();
					LoadUTF8(all_path.c_str(), true);
				}
			}
		}
	}

}