/*****************************************************************************
 * Author: Lukáš Lalinský <info@acoustid.org>                                *
 *****************************************************************************/

#include "crc.h"
#include "gzip.h"

inline QByteArray render32BitInt(unsigned long value)
{
	unsigned char data[4];
	data[0] = (value      ) & 255;
	data[1] = (value >>  8) & 255;
	data[2] = (value >> 16) & 255;
	data[3] = (value >> 24) & 255;
	return QByteArray((char *)data, 4);
}

inline unsigned long calculateCrc32(const QByteArray &data)
{
	crc_t crc;
	crc = crc_init();
	crc = crc_update(crc, (unsigned char *)data.data(), data.size());
	crc = crc_finalize(crc);
	return crc;
}

QByteArray gzipCompress(const QByteArray &data)
{
	const char header[10] = {
		0x1f, 0x8b,	// ID1 + ID2
		8,			// Compression Method
		0,			// Flags
		0, 0, 0, 0, // Modification Time
		2,			// Extra Flags
		255,		// Operating System
	};

	QByteArray compressedData = qCompress(data);
	compressedData.remove(0, 6); // Qt size + zlib header
	compressedData.remove(compressedData.size() - 4, 4); // zlib footer

	QByteArray result;
	result.append(header, 10);
	result.append(compressedData);
	result.append(render32BitInt(calculateCrc32(data)));
	result.append(render32BitInt(data.size()));
	return result;
}

