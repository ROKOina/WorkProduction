#include <Windows.h>
#include "Encoding.h"

// UTF-8 to UTF-16 •ÏŠ·
void Encoding::UTF8ToUTF16(const char* utf8, wchar_t* utf16, int utf16_size)
{
	WORD* destPosition = reinterpret_cast<WORD*>(utf16);
	WORD destCode = 0;
	int strByte = 0;
	int convertCount = 0;

	int	n = 0;
	while (1)
	{
		if ((reinterpret_cast<BYTE*>(destPosition) - reinterpret_cast<BYTE*>(utf16)) >= 8190)
		{
			destPosition[4095] = 0;
			break;
		}
		if (strByte == 1)
		{
			*destPosition++ = destCode;
			destCode = 0;
			strByte = 0;
			convertCount++;
		}
		else if (strByte > 1)
		{
			destCode <<= 6;
			destCode += (utf8[n] & 0x3F);
			strByte--;
			n++;
			continue;
		}

		if (utf8[n] == 0)
		{
			*destPosition++ = 0;
			*destPosition++ = 0;
			break;
		}

		if ((utf8[n] & 0x80) != 0x80)
		{
			strByte = 1;
			destCode = utf8[n] & 0x7F;
		}
		else if (((utf8[n] & 0xE0) == 0xC0) &&
			((utf8[n + 1] & 0xC0) == 0x80))
		{
			strByte = 2;
			destCode = utf8[n] & 0x1F;
		}
		else if (((utf8[n] & 0xF0) == 0xE0) &&
			((utf8[n + 1] & 0xC0) == 0x80) &&
			((utf8[n + 2] & 0xC0) == 0x80))
		{
			strByte = 3;
			destCode = utf8[n] & 0x0F;
		}
		else
		{
			//printf("convertUtf8toUtf16 : error !!\n");
			strByte = 1;
			destCode = utf8[n];
		}
		n++;
	}
}

// UTF-16 to UTF-8 •ÏŠ·
void Encoding::UTF16ToUTF8(const wchar_t* utf16, char* utf8, int utf8_size)
{
	BYTE* utf8_ptr = reinterpret_cast<BYTE*>(utf8);
	const WORD* utf16_ptr = reinterpret_cast<const WORD*>(utf16);

	for (int n = 0; ; n++)
	{
		if ((reinterpret_cast<BYTE*>(utf8_ptr) - reinterpret_cast<BYTE*>(utf8)) >= 4095)
		{
			utf8_ptr[4095] = 0;
			break;
		}

		WORD word = static_cast<WORD>(utf16_ptr[n]);
		if (word == 0x0000)
		{
			*utf8_ptr = 0x0000;
			break;
		}
		else if (word <= 0x007f)
		{
			*utf8_ptr = static_cast<char>((word & 0x007f));	utf8_ptr++;
		}
		else if (word <= 0x07ff)
		{
			*utf8_ptr = static_cast<char>(((word & 0x07c0) >> 6) | 0xc0);	utf8_ptr++;
			*utf8_ptr = static_cast<char>(((word & 0x003f)) | 0x80);		utf8_ptr++;
		}
		else
		{
			*utf8_ptr = static_cast<char>(((word & 0xf000) >> 12) | 0xe0);	utf8_ptr++;
			*utf8_ptr = static_cast<char>(((word & 0x0fc0) >> 6) | 0x80);	utf8_ptr++;
			*utf8_ptr = static_cast<char>(((word & 0x003f)) | 0x80);		utf8_ptr++;
		}
	}
}

// ƒŠƒeƒ‰ƒ‹•¶Žš to UTF-16 •ÏŠ·
void Encoding::StringToUTF16(const char* utf8, wchar_t* utf16, int utf16_size)
{
	if (!::MultiByteToWideChar(CP_ACP, 0, utf8, -1, utf16, utf16_size))
	{
		//printf("convertUtf8toUtf16 : error !!\n");
		utf16[0] = 0;
	}
}

// UTF-16 to ƒŠƒeƒ‰ƒ‹•¶Žš •ÏŠ·
void Encoding::UTF16ToString(const wchar_t* utf16, char* string, int string_size)
{
	if (!::WideCharToMultiByte(CP_ACP, 0, utf16, -1, string, string_size, nullptr, nullptr))
	{
		string[0] = 0;
	}
}

// ƒŠƒeƒ‰ƒ‹•¶Žš to UTF-8 •ÏŠ·
void Encoding::StringToUTF8(const char* string, char* utf8, int utf8_size)
{
	wchar_t utf16[1024];
	int utf16_size = sizeof(utf16) / sizeof(utf16[0]);
	if (!::MultiByteToWideChar(CP_ACP, 0, string, -1, utf16, utf16_size))
	{
		utf16[0] = 0;
		return;
	}

	return UTF16ToUTF8(utf16, utf8, utf8_size);
}
