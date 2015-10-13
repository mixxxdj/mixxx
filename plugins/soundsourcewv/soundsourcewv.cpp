//soundsourcewv.cpp : sound source proxy for .wv (WavPack files)
//created by fenugrec
//great help from rryan & others on #mixxx
//format_samples adapted from cmus (Peter Lemenkov)

#include <QtDebug>
#include <QFile>

#include "soundsourcewv.h"
#include "soundsourcetaglib.h"
#include "sampleutil.h"

#include <taglib/wavpackfile.h>
namespace Mixxx {

//static
WavpackStreamReader SoundSourceWV::s_streamReader = {
    SoundSourceWV::ReadBytesCallback,
    SoundSourceWV::GetPosCallback,
    SoundSourceWV::SetPosAbsCallback,
    SoundSourceWV::SetPosRelCallback,
    SoundSourceWV::PushBackByteCallback,
    SoundSourceWV::GetlengthCallback,
    SoundSourceWV::CanSeekCallback,
    SoundSourceWV::WriteBytesCallback
};

SoundSourceWV::SoundSourceWV(QString qFilename)
    : SoundSource(qFilename),
    filewvc(NULL),
    Bps(0),
    filelength(0),
    m_pWVFile(NULL),
    m_pWVCFile(NULL) {
    setType("wv");
}

SoundSourceWV::~SoundSourceWV(){
   if (filewvc) {
    WavpackCloseFile(filewvc);
    filewvc = NULL;
   }
   if (m_pWVFile) {
       m_pWVFile->close();
       delete m_pWVFile;
       m_pWVFile = NULL;
   }
   if (m_pWVCFile) {
       m_pWVCFile->close();
       delete m_pWVCFile;
       m_pWVCFile = NULL;
   }
}

QList<QString> SoundSourceWV::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("wv");
    return list;
}


Result SoundSourceWV::open() {
    char msg[80];   // hold possible error message

    // We use WavpackOpenFileInputEx to support Unicode paths on windows
    // http://www.wavpack.com/lib_use.txt
    m_pWVFile = new QFile(getFilename());
    m_pWVFile->open(QFile::ReadOnly);
    QString correctionfileName(getFilename() + "c");
    if (QFile::exists(correctionfileName)) {
        // If there is a correction file, open it as well
        m_pWVCFile = new QFile(correctionfileName);
        m_pWVCFile->open(QFile::ReadOnly);
    }
    filewvc = WavpackOpenFileInputEx(&s_streamReader, m_pWVFile, m_pWVCFile, msg,
             OPEN_2CH_MAX | OPEN_WVC, 0);
    if (!filewvc) {
        qDebug() << "SSWV::open: failed to open file : "<< msg;
        return ERR;
    }
    if (WavpackGetMode(filewvc) & MODE_FLOAT) {
        qDebug() << "SSWV::open: cannot load 32bit float files";
        WavpackCloseFile(filewvc);
        filewvc = NULL;
        m_pWVFile->close();
        delete m_pWVFile;
        m_pWVFile = NULL;
        return ERR;
    }
    // wavpack_open succeeded -> populate variables
    filelength = WavpackGetNumSamples(filewvc);
    setSampleRate(WavpackGetSampleRate(filewvc));
    setChannels(WavpackGetReducedChannels(filewvc));
    Bps=WavpackGetBytesPerSample(filewvc);
    qDebug() << "SSWV::open: opened filewvc with filelength: " << filelength
            << " SampleRate: " << getSampleRate() << " channels: "
            << getChannels() << " bytes per samp: " << Bps;
    if (Bps>2) {
        qDebug() << "SSWV::open: warning: input file has > 2 bytes per sample, will be truncated to 16bits";
    }
    return OK;
}


long SoundSourceWV::seek(long filepos){
    if (WavpackSeekSample(filewvc,filepos>>1) != true) {
        qDebug() << "SSWV::seek : could not seek to sample #" << (filepos>>1);
        return 0;
    }
    return filepos;
}


unsigned SoundSourceWV::read(volatile unsigned long size, const SAMPLE* destination){
    //SAMPLE is "short int" => 16bits. [size] is timesamps*2 (because L+R)
    SAMPLE * dest = (SAMPLE*) destination;
    unsigned long sampsread=0;
    unsigned long timesamps, tsdone;

    //tempbuffer is fixed size : WV_BUF_LENGTH of uint32
    while (sampsread != size) {
        timesamps = (size-sampsread) >> 1; // timesamps still remaining
        if (timesamps > (unsigned long)(WV_BUF_LENGTH / getChannels())) {  //if requested size requires more than one buffer filling
            timesamps = (WV_BUF_LENGTH / getChannels());      //tempbuffer must hold (timesamps * channels) samples
            qDebug() << "SSWV::read : performance warning, size requested > buffer size !";
        }

        tsdone = WavpackUnpackSamples(filewvc, tempbuffer, timesamps);    //fill temp buffer with timesamps*4bytes*channels
                //data is right justified, format_samples() fixes that.

        SoundSourceWV::format_samples(Bps, (char *) (dest + (sampsread>>1) * getChannels()), tempbuffer, tsdone*getChannels());
                                //this will unpack the 4byte/sample
                                //output of wUnpackSamples(), sign-extending or truncating to output 16bit / sample.
                                //specifying dest+sampsread should resume the conversion where it was left if size requested
                                //required multiple reads (size req. > fixed buffer size)

        sampsread = sampsread + (tsdone<<1);
        if (tsdone!=timesamps) {
            qDebug () << "SSWV::read : WavpackUnpackSamples read "<<sampsread<<" asamps out of "<<size<<" requested";
            break;  //exit the while loop : subsequent reads are sure to read less than required.
        }

    }

    if (getChannels() == 1) { //if MONO : expand array to double it's size; see ssov.cpp
        SampleUtil::doubleMonoToDualMono(dest, sampsread / 2);
    }

    return sampsread;
}


inline long unsigned SoundSourceWV::length(){
    //filelength is # of timesamps.
    return filelength<<1;
}


Result SoundSourceWV::parseHeader() {
    TagLib::WavPack::File f(TAGLIB_FILENAME_FROM_QSTRING(getFilename()));
    if (!readFileHeader(this, f)) {
        return ERR;
    }

    TagLib::APE::Tag *ape = f.APETag();
    if (ape) {
        readAPETag(this, *ape);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(this, *tag);
        } else {
            return ERR;
        }
    }

    return OK;
}

QImage SoundSourceWV::parseCoverArt() {
    TagLib::WavPack::File f(TAGLIB_FILENAME_FROM_QSTRING(getFilename()));
    TagLib::APE::Tag *ape = f.APETag();
    if (ape) {
        return Mixxx::getCoverInAPETag(*ape);
    } else {
        return QImage();
    }
}

void SoundSourceWV::format_samples(int Bps, char *dst, int32_t *src, uint32_t count)
{
    //this handles converting the fixed 32bit per sample produced by UnpackSamples
    //to 16 bps, by truncating (24/32) or sign-extending (8)
    //could eventually be asm-optimized..
    int32_t temp;

    switch (Bps) {
    case 1:
        while (count--) {
            *dst++ = (char) 0;      //left shift the 8 bit sample
            *dst++ = (char) *src++ ;//+ 128;        //only works with u8int ?
        }
        break;
    case 2:
        while (count--) {
            *dst++ = (char) (temp = *src++);    //low byte
            *dst++ = (char) (temp >> 8);        //high byte
        }
        break;
    case 3: //modified to truncate to 16bits
        while (count--) {
            *dst++ = (char) (temp = (*src++) >> 8);
            *dst++ = (char) (temp >> 8);
        }
        break;
    case 4: //also truncates
        while (count--) {
            *dst++ = (char) (temp = (*src++) >> 16);
            *dst++ = (char) (temp >> 8);
            //*dst++ = (char) (temp >> 16);
            //*dst++ = (char) (temp >> 24);
        }
        break;
    }

    return;
}

//static
int32_t SoundSourceWV::ReadBytesCallback(void* id, void* data, int bcount)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->read((char*)data, bcount);
}


// static
uint32_t SoundSourceWV::GetPosCallback(void *id)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->pos();
}

//static
int SoundSourceWV::SetPosAbsCallback(void* id, unsigned int pos)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->seek(pos) ? 0 : -1;
}

//static
int SoundSourceWV::SetPosRelCallback(void *id, int delta, int mode)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }

    switch(mode) {
    case SEEK_SET:
        return pFile->seek(delta) ? 0 : -1;
    case SEEK_CUR:
        return pFile->seek(pFile->pos() + delta) ? 0 : -1;
    case SEEK_END:
        return pFile->seek(pFile->size() + delta) ? 0 : -1;
    default:
        return -1;
    }
}

//static
int SoundSourceWV::PushBackByteCallback(void* id, int c)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    pFile->ungetChar((char)c);
    return 1;
}

//static
uint32_t SoundSourceWV::GetlengthCallback(void* id)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->size();
}

//static
int SoundSourceWV::CanSeekCallback(void *id)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->isSequential() ? 0 : 1;
}

//static
int32_t SoundSourceWV::WriteBytesCallback(void* id, void* data, int32_t bcount)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return (int32_t)pFile->write((char*)data, bcount);
}

}  // namespace Mixxx
