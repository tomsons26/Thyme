/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Compression manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "compressionmanager.h"
#include "endiantype.h"
#include "refpack.h"
#include <captainslog.h>
#include <cmath>
#include <cstring>

using std::memcmp;

// dummy to make sense of it
int compress2(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen, int level)
{
    return 0;
}

int uncompress(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen)
{
    return 0;
}

const char *CompressionManager::s_compressionNames[COMPRESSION_COUNT] = { "No compression",
    "RefPack",
    "LZH light (Nox, J2K)",
    "zlib compress 1",
    "zlib compress 2",
    "zlib compress 3",
    "zlib compress 4",
    "zlib compress 5",
    "zlib compress 6",
    "zlib compress 7",
    "zlib compress 8",
    "zlib compress 9",
    "B-Tree compression",
    "Huffman Tree compression" };

/**
 * @brief Detect if the data is compressed.
 */
bool CompressionManager::Is_Data_Compressed(const void *data, int size)
{
    return Get_Compression_Type(data, size) != COMPRESSION_NONE;
}

/**
 * @brief Get prefered compression type.
 */
CompressionType CompressionManager::Get_Prefered_Compression()
{
    return COMPRESSION_EAR;
}

/**
 * @brief Get type of compression used based on a small header.
 */
CompressionType CompressionManager::Get_Compression_Type(const void *data, int size)
{
    if (size < sizeof(CompHeader)) {
        return COMPRESSION_NONE;
    }

    const CompHeader *head = static_cast<const CompHeader *>(data);

    CompressionType type = COMPRESSION_NONE;

    switch (head->id) {
        case 'XON':
            type = COMPRESSION_NOX;
            break;
        case '1LZ':
            type = COMPRESSION_ZL1;
            break;
        case '2LZ':
            type = COMPRESSION_ZL2;
            break;
        case '3LZ':
            type = COMPRESSION_ZL3;
            break;
        case '4LZ':
            type = COMPRESSION_ZL4;
            break;
        case '5LZ':
            type = COMPRESSION_ZL5;
            break;
        case '6LZ':
            type = COMPRESSION_ZL6;
            break;
        case '7LZ':
            type = COMPRESSION_ZL7;
            break;
        case '8LZ':
            type = COMPRESSION_ZL8;
            break;
        case '9LZ':
            type = COMPRESSION_ZL9;
            break;
        case 'BAE':
            type = COMPRESSION_EAB;
            break;
        case 'HAE':
            type = COMPRESSION_EAH;
            break;
        case 'RAE':
            type = COMPRESSION_EAR;
            break;
    }

    return type;
}
/**
 * @brief ?.
 */
int CompressionManager::Get_Max_Compressed_Size(int size, CompressionType type)
{
    int max_size = 0;

    switch (type) {
        case COMPRESSION_NOX:
            // max_size = CalcNewSize(size) + sizeof(CompHeader);
            break;
        case COMPRESSION_ZL1:
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9:
            max_size = ceil((double)size * 1.1 + 20.0);
            break;
        case COMPRESSION_EAB:
        case COMPRESSION_EAH:
        case COMPRESSION_EAR:
            max_size = size + sizeof(CompHeader);
            break;
        default:
            break;
    }
    return max_size;
}

/**
 * @brief Get uncompressed size based on a small header.
 */
int CompressionManager::Get_Uncompressed_Size(const void *data, int size)
{
    if (size < sizeof(CompHeader)) {
        return size;
    }

    CompressionType type = Get_Compression_Type(data, size);

    const CompHeader *head = static_cast<const CompHeader *>(data);

    switch (type) {
        case COMPRESSION_NOX:
        case COMPRESSION_ZL1:
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9:
        case COMPRESSION_EAB:
        case COMPRESSION_EAH:
        case COMPRESSION_EAR:
            size = le32toh(head->uncomp_size);
            break;
        default:
            break;
    }

    return size;
}
#define BYTEn(x, n) (*((unsigned char *)&(x) + n))
#define BYTE2(x) BYTEn(x, 2)

    /**
 * @brief Compress uncompressed data. Only handles RefPack and ZLib compression.
 */
int CompressionManager::Compress_Data(CompressionType type, void *src, int src_size, void *dst, int dst_size)
{
    if (dst_size < sizeof(CompHeader)) {
        return 0;
    }

    switch (type) {
        case COMPRESSION_EAR: {
            CompHeader *head = (CompHeader *)dst;
            void *d = (head + 1);

            head->id = 'RAE';
            head->uncomp_size = 0;

            int comp_size = RefPack_Compress(d, src, src_size, nullptr);
            if (comp_size > 0) {
                head->uncomp_size = src_size;
                return comp_size + sizeof(CompHeader);
            }
            break;
        }
        case COMPRESSION_ZL1:
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9: {
            CompHeader *head = (CompHeader *)dst;
            void *d = (head + 1);

            head->id = 'ZL0';
            BYTE2(head->id) = type - 2 + '0';
            head->uncomp_size = 0;

            unsigned int comp_size = dst_size - sizeof(CompHeader);
            int res = compress2(d, &comp_size, src, src_size, type - 2);
            if (res == 0 || res == 1) {
                head->uncomp_size = src_size;
                return comp_size + sizeof(CompHeader);
            }
        }
        default:
            break;
    }

    return 0;
}

/**
 * @brief Decompress possibly compressed data. Only handles RefPack and ZLib compression.
 */
int CompressionManager::Decompress_Data(void *src, int src_size, void *dst, int dst_size)
{
    if (src_size < sizeof(CompHeader)) {
        return 0;
    }

    switch (Get_Compression_Type(src, src_size)) {
        case COMPRESSION_EAR: // RefPack
            src_size -= sizeof(CompHeader);
            return RefPack_Uncompress(dst, static_cast<const uint8_t *>(src) + sizeof(CompHeader), &src_size);
        case COMPRESSION_ZL1: // ZLib
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9:
            src_size -= sizeof(CompHeader);
            unsigned int dlen = dst_size;
            char res = uncompress(dst, &dlen, static_cast<const uint8_t *>(src) + sizeof(CompHeader), src_size);
            if (res == 0 || res == 1) {
                return dlen;
            }
            break;
        case COMPRESSION_NONE:
        case COMPRESSION_NOX:
        case COMPRESSION_EAB:
        case COMPRESSION_EAH:
        default:
            captainslog_error("Compression format '%s' unhandled, file a bug report.\n",
                Get_Compression_Name(Get_Compression_Type(src, src_size)));
            break;
    }

    return 0;
}
