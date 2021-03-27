#ifndef _MAIN_H
#define _MAIN_H

#define GAME_REGION_NA              (0x00000001)
#define GAME_REGION_JAPAN           (0x00000002)
#define GAME_REGION_RESTOFWORLD     (0x00000004)
#define GAME_REGION_MANUFACTURING   (0x80000000)

typedef struct _minimal_header_st
{
    unsigned int  dwMagic;                         // 0x0000 - magic number [should be "XBEH"]
    unsigned char pbDigitalSignature[256];         // 0x0004 - digital signature
    unsigned int  dwBaseAddr;                      // 0x0104 - base address
    unsigned int  dwSizeofHeaders;                 // 0x0108 - size of headers
    unsigned int  dwSizeofImage;                   // 0x010C - size of image
    unsigned int  dwSizeofImageHeader;             // 0x0110 - size of image header
    unsigned int  dwTimeDate;                      // 0x0114 - timedate stamp
    unsigned int  dwCertificateAddr;               // 0x0118 - certificate address
    unsigned int  dwSections;                      // 0x011C - number of sections
    unsigned int  dwSectionHeadersAddr;            // 0x0120 - section headers address
}minimal_header_st;

typedef struct _certificate_st
{
    unsigned int    dwSize;                               // 0x0000 - size of certificate
    unsigned int    dwTimeDate;                           // 0x0004 - timedate stamp
    unsigned int    dwTitleId;                            // 0x0008 - title id
    unsigned short  wszTitleName[40];                     // 0x000C - title name (unicode)
    unsigned int    dwAlternateTitleId[0x10];             // 0x005C - alternate title ids
    unsigned int    dwAllowedMedia;                       // 0x009C - allowed media types
    unsigned int    dwGameRegion;                         // 0x00A0 - game region
    unsigned int    dwGameRatings;                        // 0x00A4 - game ratings
    unsigned int    dwDiskNumber;                         // 0x00A8 - disk number
    unsigned int    dwVersion;                            // 0x00AC - version
    unsigned char   bzLanKey[16];                         // 0x00B0 - lan key
    unsigned char   bzSignatureKey[16];                   // 0x00C0 - signature key
    unsigned char   bzTitleAlternateSignatureKey[16][16]; // 0x00D0 - alternate signature keys
}certificate_st;

typedef union
{
    minimal_header_st headerdata;
    unsigned char headerraw[sizeof(minimal_header_st)];
    
}header_data_un;

typedef union
{
    certificate_st certdata;
    unsigned char certraw[sizeof(certificate_st)];
    
}cert_data_un;

#endif

