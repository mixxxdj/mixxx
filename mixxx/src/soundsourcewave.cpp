#include "soundsourcewave.h"
/*
  Class for reading files using libsndfile
*/
SoundSourceWave::SoundSourceWave(const char* filename)
{
    info = new SF_INFO;
    fh = sf_open(filename,SFM_READ,info);
    if (fh == 0 || !sf_format_check(info))
    {
        qDebug("libsndfile: Error opening file.");
        filelength = 0;
        return;
    } else
        filelength = 2*info->frames;

    if (info->channels != 2)
    {
        qDebug("libsndfile: Only two-channel files are supported.");
        sf_close(fh);
        filelength = 0;
        return;
    }        
    channels = 2;
    
    type = "wav file.";
}

SoundSourceWave::~SoundSourceWave()
{
    sf_close(fh);
    delete info;
};

long SoundSourceWave::seek(long filepos)
{
    if (sf_seek(fh, (sf_count_t)filepos, SEEK_SET) == -1)
        qDebug("libsndfile: Seek error.");
    return filepos;
}

/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceWave::read(unsigned long size, const SAMPLE* destination)
{
    return sf_read_short(fh,(SAMPLE *)destination, size/channels)*channels;
}

/*
  Return the length of the file in samples.
*/
long unsigned SoundSourceWave::length()
{
    return filelength;
}

