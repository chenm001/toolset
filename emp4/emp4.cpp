#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <codecvt>
#include "utfconvert.h"

#if defined(GCC)

#elif defined(_MSC_VER)
#define ntohl(x)            _byteswap_ulong((x))
#pragma 
#else
#error
#endif

#ifdef __GNUC__
#define PACKED( class_to_pack ) class_to_pack __attribute__((__packed__))
#else
#define PACKED( class_to_pack ) __pragma( pack(push, 1) ) class_to_pack __pragma( pack(pop) )
#endif

#define MAKE_ATOM(x)        (ntohl(*(long*)(x)))

#define ATOM_BOOKMARKS          "book"
#define ATOM_BOOKMARKS_NUM      "bnum"

#define MAX_BOX_LEN             (8)
#define MAX_TIME_LEN            (4)
#define MAX_TITLE_LEN           (100)
#define MAX_RSVD_LEN            (500)

PACKED(struct bookmark_t
{
    uint8_t     m_box[MAX_BOX_LEN];
    uint8_t     m_time[MAX_TIME_LEN];
    uint8_t     m_title[MAX_TITLE_LEN];
    uint8_t     m_rsvd[MAX_RSVD_LEN];
});

#define MAX_BOOKMARKS_NUM       (50)
#define MAX_BOOKMARKS_LEN       (sizeof(struct bookmark_t));


const uint8_t boxBKMK[] = {0x00, 0x00, 0x02, 0x64, 'b', 'k', 'm', 'k'};
const uint8_t boxBNUM[] = {0x00, 0x00, 0x00, 0x08, 'b', 'n', 'u', 'm'};

bool WideStringToString(const std::wstring& src,std::string &str)
{
    std::locale sys_locale("");

    const wchar_t* data_from = src.c_str();
    const wchar_t* data_from_end = src.c_str() + src.size();
    const wchar_t* data_from_next = 0;

    int wchar_size = 4;
    char* data_to = new char[(src.size() + 1) * wchar_size];
    char* data_to_end = data_to + (src.size() + 1) * wchar_size;
    char* data_to_next = 0;

    memset( data_to, 0, (src.size() + 1) * wchar_size );

    typedef std::codecvt<wchar_t, char, mbstate_t> convert_facet;
    mbstate_t out_state = {0};
    auto result = std::use_facet<convert_facet>(sys_locale).out(
        out_state, data_from, data_from_end, data_from_next,
        data_to, data_to_end, data_to_next );
    if( result == convert_facet::ok)
    {
        str = data_to;
        delete[] data_to;
        return true;
    }
    delete[] data_to;
    return false;
}

bool StringToWideString( const std::string& src,std::wstring &wstr)
{
    std::locale sys_locale("");
    const char* data_from = src.c_str();
    const char* data_from_end = src.c_str() + src.size();
    const char* data_from_next = 0;

    wchar_t* data_to = new wchar_t[src.size() + 1];
    wchar_t* data_to_end = data_to + src.size() + 1;
    wchar_t* data_to_next = 0;

    wmemset( data_to, 0, src.size() + 1 );

    typedef std::codecvt<wchar_t, char, mbstate_t> convert_facet;
    mbstate_t in_state = {0};
    auto result = std::use_facet<convert_facet>(sys_locale).in(
        in_state, data_from, data_from_end, data_from_next,
        data_to, data_to_end, data_to_next );
    if( result == convert_facet::ok )
    {
        wstr = data_to;
        delete[] data_to;
        return true;
    }
    delete[] data_to;
    return false;
}


bool WCharStringToUTF8String(const std::wstring &wstr,std::string &u8str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    u8str= conv.to_bytes(wstr);
    return true;
}

bool UTF8StringToWCharString(const std::string &u8str,std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
    wstr=conv.from_bytes( u8str );
    return true;
}

// TODO: Optimize by MP4 box struct
size_t findCustomAtom(const char *atom, const uint8_t *inBuf, const size_t inBufSize)
{
    const uint32_t atomID = MAKE_ATOM(atom);
    uint32_t val = ntohl(*(uint32_t*)&inBuf[0]);

    for(size_t i = 4; i < inBufSize; i++)
    {
        if(val == atomID)
        {
            return i - 4;
        }
        val = (val << 8) + inBuf[i];
    }
    return -1;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage %s filename\n", argv[0]);
        return 0;
    }

    FILE *fpi = fopen(argv[1], "rb");
    assert(fpi != NULL);

    size_t fSize = 0;
    uint8_t *fBuf = NULL;

    fseek(fpi, 0, SEEK_END);
    fSize = ftell(fpi);
    rewind(fpi);

    fBuf = (uint8_t*)malloc(fSize);
    assert(fBuf != NULL);
    const size_t readed = fread(fBuf, fSize, 1, fpi);
    fclose(fpi);
    assert(readed == 1);

    int atomBook = findCustomAtom(ATOM_BOOKMARKS, fBuf, fSize);
    if(atomBook < 0)
    {
        fprintf(stderr, "Can't find bookmark\n");
        return 0;
    }
    int bookSize = ntohl(*(uint32_t*)&fBuf[atomBook - 4]);
    fprintf(stderr, "Found [%s] at %d (0x%X) with size %d bytes\n", ATOM_BOOKMARKS, atomBook, atomBook, bookSize);

    int atomBnum = findCustomAtom(ATOM_BOOKMARKS_NUM, &fBuf[atomBook], fSize);
    assert(atomBnum >= 0);
    atomBnum += atomBook;
    int bnumSize = ntohl(*(uint32_t*)&fBuf[atomBnum - 4]);
    fprintf(stderr, "Found [%s] at %d (0x%X) with size %d bytes\n", ATOM_BOOKMARKS_NUM, atomBnum, atomBnum, bnumSize);

    int cnt = (bnumSize - 8) / MAX_BOOKMARKS_LEN;
    fprintf(stderr, "Found %d bookmarks\n", cnt);


    const struct bookmark_t *ptr = (struct bookmark_t *)&fBuf[atomBnum - 4 + 8];

    for(int i = 0; i < cnt; i++)
    {
        if(memcmp(ptr->m_box, boxBKMK, sizeof(boxBKMK)))
        {
            fprintf(stderr, "bookmark check failed!\n");
            break;
        }

        std::u16string title16 = std::u16string( reinterpret_cast<const char16_t*>(ptr->m_title), sizeof(ptr->m_title) / 2 );
        std::string title8 = utf16be_to_utf8(title16);

        std::u16string rsvd16 = std::u16string( reinterpret_cast<const char16_t*>(ptr->m_rsvd), sizeof(ptr->m_rsvd) / 2 );
        std::string rsvd8 = utf16be_to_utf8(rsvd16);

        uint32_t mTime = ntohl(*(uint32_t*)ptr->m_time);
        int timeH = (mTime / 3600000);
        int timeM = (mTime / 60000) % 60;
        int timeS = (mTime / 1000) % 60;
        int timeU = mTime % 1000;

        std::wstring strW16;
        UTF8StringToWCharString(title8, strW16);
        WideStringToString(strW16, title8);

        fprintf(stderr, "[%2d] time=%02d:%02d:%02d, title=[%-40s], rsvd=[%s]\n",
            i,
            timeH,
            timeM,
            timeS,
            title8.c_str(),
            rsvd8.c_str()
            );

        ptr++;
    }

    free(fBuf);
    return 0;
}
