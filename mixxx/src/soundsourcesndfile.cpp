#include "SoundSourcesndfile.h"
/*
  Class for reading files using libsndfile
*/
SoundSourceSndFile::SoundSourceSndFile(const char* filename)
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

SoundSourceSndFile::~SoundSourceSndFile()
{
    if (filelength > 0)
        sf_close(fh);
    delete info;
};

long SoundSourceSndFile::seek(long filepos)
{
    if (filelength > 0)
        if (sf_seek(fh, (sf_count_t)filepos, SEEK_SET) == -1)
            qDebug("libsndfile: Seek error.");
    
    return filepos;
}

/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceSndFile::read(unsigned long size, const SAMPLE* destination)
{
    if (filelength > 0)
        return sf_read_short(fh,(SAMPLE *)destination, size);
    else
    {
        for (unsigned int i=0; i<size; i++)
            ((SAMPLE *)destination)[i] = 0;
        return size;
    }
}

/*
  Return the length of the file in samples.
*/
inline long unsigned SoundSourceSndFile::length()
{
    return filelength;
}

