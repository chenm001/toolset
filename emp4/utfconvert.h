#ifndef __UTFCONVERT_H__
#define __UTFCONVERT_H__
#include <string>


// ��UTF16�����ַ�����������Ҫ��BOM���
std::string utf16_to_utf8(const std::u16string& u16str);

// ��UTF16 LE������ַ�������
std::string utf16le_to_utf8(const std::u16string& u16str);

// ��UTF16BE�����ַ�������
std::string utf16be_to_utf8(const std::u16string& u16str);

// ��ȡת��ΪUTF-16 LE������ַ���
std::u16string utf8_to_utf16le(const std::string& u8str, bool addbom = false, bool* ok = NULL);

// ��ȡת��ΪUTF-16 BE���ַ���
std::u16string utf8_to_utf16be(const std::string& u8str, bool addbom = false, bool* ok = NULL);

#endif //! __UTFCONVERT_H__
