#ifndef SOUNDSOURCEHEAVYMP3_H
#define SOUNDSOURCEHEAVYMP3_H

#include <qobject.h>
#include "defs.h"
#include "soundsource.h"
#include <mad.h>
#include "errno.h"
#include <vector>

class SoundSourceHeavymp3 : public SoundSource {
 private:
	 std::vector<SAMPLE> buffer;
    long unsigned bufferlen;
    FILE *file;
    unsigned inputbuf_len;
    unsigned char *inputbuf;
    int bitrate;
    long position;
 public:
    SoundSourceHeavymp3(const char*);
    ~SoundSourceHeavymp3();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    long unsigned length();
};


#endif
