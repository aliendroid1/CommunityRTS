/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// FILE: Compression.cpp /////////////////////////////////////////////////////
// Author: Matthew D. Campbell
// LZH wrapper taken from Nox, originally from Jeff Brown
//////////////////////////////////////////////////////////////////////////////

#include "Compression.h"
#include "LZHCompress/NoxCompress.h"

//#define __MACTYPES__

#include <zlib.h>

#include "EAC/codex.h"
#include "EAC/btreecodex.h"
#include "EAC/huffcodex.h"
#include "EAC/refcodex.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma message("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// TheSuperHackers @todo Recover debug logging in this file?
#define DEBUG_LOG(x) {}

const char *CompressionManager::getCompressionNameByType( CompressionType compType )
{
	static const char *s_compressionNames[COMPRESSION_MAX+1] = {
		"No compression",
		"RefPack",
		"LZHL",
		"ZLib 1 (fast)",
		"ZLib 2",
		"ZLib 3",
		"ZLib 4",
		"ZLib 5 (default)",
		"ZLib 6",
		"ZLib 7",
		"ZLib 8",
		"ZLib 9 (slow)",
		"BTree",
		"Huff",
	};
	return s_compressionNames[compType];
}

// For perf timers, so we can have separate ones for compression/decompression
const char *CompressionManager::getDecompressionNameByType( CompressionType compType )
{
	static const char *s_decompressionNames[COMPRESSION_MAX+1] = {
		"d_None",
		"d_RefPack",
		"d_NoxLZW",
		"d_ZLib1",
		"d_ZLib2",
		"d_ZLib3",
		"d_ZLib4",
		"d_ZLib5",
		"d_ZLib6",
		"d_ZLib7",
		"d_ZLib8",
		"d_ZLib9",
		"d_BTree",
		"d_Huff",
	};
	return s_decompressionNames[compType];
}

// ---------------------------------------------------------------------------------------

Bool CompressionManager::isDataCompressed( const void *mem, Int len )
{
	CompressionType t = getCompressionType(mem, len);
	return t != COMPRESSION_NONE;
}

CompressionType CompressionManager::getPreferredCompression( void )
{
	return COMPRESSION_REFPACK;
}


CompressionType CompressionManager::getCompressionType( const void *mem, Int len )
{
	if (len < 8)
		return COMPRESSION_NONE;

	if ( memcmp( mem, "NOX\0", 4 ) == 0 )
		return COMPRESSION_NOXLZH;
	if ( memcmp( mem, "ZL1\0", 4 ) == 0 )
		return COMPRESSION_ZLIB1;
	if ( memcmp( mem, "ZL2\0", 4 ) == 0 )
		return COMPRESSION_ZLIB2;
	if ( memcmp( mem, "ZL3\0", 4 ) == 0 )
		return COMPRESSION_ZLIB3;
	if ( memcmp( mem, "ZL4\0", 4 ) == 0 )
		return COMPRESSION_ZLIB4;
	if ( memcmp( mem, "ZL5\0", 4 ) == 0 )
		return COMPRESSION_ZLIB5;
	if ( memcmp( mem, "ZL6\0", 4 ) == 0 )
		return COMPRESSION_ZLIB6;
	if ( memcmp( mem, "ZL7\0", 4 ) == 0 )
		return COMPRESSION_ZLIB7;
	if ( memcmp( mem, "ZL8\0", 4 ) == 0 )
		return COMPRESSION_ZLIB8;
	if ( memcmp( mem, "ZL9\0", 4 ) == 0 )
		return COMPRESSION_ZLIB9;
	if ( memcmp( mem, "EAB\0", 4 ) == 0 )
		return COMPRESSION_BTREE;
	if ( memcmp( mem, "EAH\0", 4 ) == 0 )
		return COMPRESSION_HUFF;
	if ( memcmp( mem, "EAR\0", 4 ) == 0 )
		return COMPRESSION_REFPACK;

	return COMPRESSION_NONE;
}

Int CompressionManager::getMaxCompressedSize( Int uncompressedLen, CompressionType compType )
{
	switch (compType)
	{
		case COMPRESSION_NOXLZH:
			return CalcNewSize(uncompressedLen) + 8;

		case COMPRESSION_BTREE:   // guessing here
		case COMPRESSION_HUFF:    // guessing here
		case COMPRESSION_REFPACK: // guessing here
			return uncompressedLen + 8;
		case COMPRESSION_ZLIB1:
		case COMPRESSION_ZLIB2:
		case COMPRESSION_ZLIB3:
		case COMPRESSION_ZLIB4:
		case COMPRESSION_ZLIB5:
		case COMPRESSION_ZLIB6:
		case COMPRESSION_ZLIB7:
		case COMPRESSION_ZLIB8:
		case COMPRESSION_ZLIB9:
			return (Int)(ceil(uncompressedLen * 1.1 + 12 + 8));
	}

	return 0;
}

Int CompressionManager::getUncompressedSize( const void *mem, Int len )
{
	if (len < 8)
		return len;

	CompressionType compType = getCompressionType( mem, len );
	switch (compType)
	{
		case COMPRESSION_NOXLZH:
		case COMPRESSION_ZLIB1:
		case COMPRESSION_ZLIB2:
		case COMPRESSION_ZLIB3:
		case COMPRESSION_ZLIB4:
		case COMPRESSION_ZLIB5:
		case COMPRESSION_ZLIB6:
		case COMPRESSION_ZLIB7:
		case COMPRESSION_ZLIB8:
		case COMPRESSION_ZLIB9:
		case COMPRESSION_BTREE:
		case COMPRESSION_HUFF:
		case COMPRESSION_REFPACK:
			return *(Int *)(((UnsignedByte *)mem)+4);
	}

	return len;
}

Int CompressionManager::compressData( CompressionType compType, void *srcVoid, Int srcLen, void *destVoid, Int destLen )
{
	if (destLen < 8)
		return 0;

	destLen -= 8;

	UnsignedByte *src = (UnsignedByte *)srcVoid;
	UnsignedByte *dest = (UnsignedByte *)destVoid;

	if (compType == COMPRESSION_BTREE)
	{
		memcpy(dest, "EAB\0", 4);
		*(Int *)(dest+4) = 0;
		Int ret = BTREE_encode(dest+8, src, srcLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return ret + 8;
		}
		else
			return 0;
	}
	if (compType == COMPRESSION_HUFF)
	{
		memcpy(dest, "EAH\0", 4);
		*(Int *)(dest+4) = 0;
		Int ret = HUFF_encode(dest+8, src, srcLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return ret + 8;
		}
		else
			return 0;
	}
	if (compType == COMPRESSION_REFPACK)
	{
		memcpy(dest, "EAR\0", 4);
		*(Int *)(dest+4) = 0;
		Int ret = REF_encode(dest+8, src, srcLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return ret + 8;
		}
		else
			return 0;
	}

	if (compType == COMPRESSION_NOXLZH)
	{
		memcpy(dest, "NOX\0", 4);
		*(Int *)(dest+4) = 0;
		Bool ret = CompressMemory(src, srcLen, dest+8, destLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return destLen + 8;
		}
		else
			return 0;
	}

	if (compType >= COMPRESSION_ZLIB1 && compType <= COMPRESSION_ZLIB9)
	{
		Int level = compType - COMPRESSION_ZLIB1 + 1; // 1-9
		memcpy(dest, "ZL0\0", 4);
		dest[2] = '0' + level;
		*(Int *)(dest+4) = 0;

		unsigned long outLen = destLen;
		Int err = compress2( (Bytef*)dest+8, &outLen, (const Bytef*)src, srcLen, level );

		if (err == Z_OK || err == Z_STREAM_END)
		{
			*(Int *)(dest+4) = srcLen;
			return outLen + 8;
		}
		else
		{
			DEBUG_LOG(("ZLib compression error (level is %d, src len is %d) %d\n", level, srcLen, err));
			return 0;
		}
	}

	return 0;
}

Int CompressionManager::decompressData( void *srcVoid, Int srcLen, void *destVoid, Int destLen )
{
	if (srcLen < 8)
		return 0;

	UnsignedByte *src = (UnsignedByte *)srcVoid;
	UnsignedByte *dest = (UnsignedByte *)destVoid;

	CompressionType compType = getCompressionType(src, srcLen);

	if (compType == COMPRESSION_BTREE)
	{
		Int slen = srcLen - 8;
		Int ret = BTREE_decode(dest, src+8, &slen);
		if (ret)
			return ret;
		else
			return 0;
	}
	if (compType == COMPRESSION_HUFF)
	{
		Int slen = srcLen - 8;
		Int ret = HUFF_decode(dest, src+8, &slen);
		if (ret)
			return ret;
		else
			return 0;
	}
	if (compType == COMPRESSION_REFPACK)
	{
		Int slen = srcLen - 8;
		Int ret = REF_decode(dest, src+8, &slen);
		if (ret)
			return ret;
		else
			return 0;
	}

	if (compType == COMPRESSION_NOXLZH)
	{
		Bool ret = DecompressMemory(src+8, srcLen-8, dest, destLen);
		if (ret)
			return destLen;
		else
			return 0;
	}

	if (compType >= COMPRESSION_ZLIB1 && compType <= COMPRESSION_ZLIB9)
	{
		unsigned long outLen = destLen;
		Int err = uncompress((Bytef*)dest, &outLen, (const Bytef*)src+8, srcLen-8);
		if (err == Z_OK || err == Z_STREAM_END)
		{
			return outLen;
		}
		else
		{
			DEBUG_LOG(("ZLib decompression error (src is level %d, %d bytes long) %d\n",
				compType - COMPRESSION_ZLIB1 + 1 /* 1-9 */, srcLen, err));
			return 0;
		}
	}

	return 0;
}

