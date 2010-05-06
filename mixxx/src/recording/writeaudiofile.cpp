#include <QtCore>
#include <sndfile.h>
#include "../controlobjectthread.h"
#include "../controlobject.h"
#include "../dlgprefrecord.h"
#include "writeaudiofile.h"
#include "defs_recording.h"


WriteAudioFile::WriteAudioFile(ConfigObject<ConfigValue> * _config)
{
    ready = false;
    sf = NULL;
    memset(&sfInfo, 0, sizeof(sfInfo));
    config = _config;
    ctrlRec = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]", "Record")));
}

WriteAudioFile::~WriteAudioFile()
{
    delete ctrlRec;
    close();
}

void WriteAudioFile::open()
{
    //Note: libsndfile doesn't seem to like utf8 strings unfortunately :(
    QByteArray baPath = config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Path")).toAscii();
    const char *path = baPath.data();
    QString encodingType = config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding"));

    ready = false;
    if(ctrlRec->get() == RECORD_ON)
    {
        //if the record flag is set

        //set sfInfo
        sfInfo.samplerate = config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();
        sfInfo.channels = 2;

        if (encodingType == ENCODING_WAVE)
            sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        else if (encodingType == ENCODING_AIFF)
            sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
            //# TODO: Fix code SF_FORMAT_FLAC block below to work on Windows
    #ifdef SF_FORMAT_FLAC
        else if (encodingType == ENCODING_FLAC)
            sfInfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
    #endif
        else if (encodingType == ENCODING_OGG || encodingType == ENCODING_MP3)
        {
            qDebug() << "Error: OGG and MP3 decoding not handled by libsndfile!";
        }
        else
        {
            qDebug() << "Corrupt recording encoding section in preference file:" << encodingType;
            return;
        }

        sf = sf_open(path, SFM_WRITE, &sfInfo);

        if(sf == NULL)
        {
            qDebug("Error initializing recording libsndfile");
            ready = false;
        }
        else
        {
            sf_command (sf, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;
            ready = true;

            //set meta data
            int ret;
            QByteArray baTitle = config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")).toAscii();
            ret = sf_set_string(sf, SF_STR_TITLE, baTitle.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
            QByteArray baAuthor = config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")).toAscii();
            ret = sf_set_string(sf, SF_STR_ARTIST, baAuthor.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
            QByteArray baComment = config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Comment")).toAscii();
            ret = sf_set_string(sf, SF_STR_COMMENT, baComment.data()); 
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

            ret = sf_set_string(sf, SF_STR_TITLE, "Testing");
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
        }
    }
    else
        qWarning() << "Tried to open WriteAudioFile before recording control was set to RECORD_ON";
}

void WriteAudioFile::write(const CSAMPLE * pIn, int iBufferSize)
{
    Q_ASSERT(iBufferSize % 2 == 0);
    CSAMPLE *pSamples = (CSAMPLE*) pIn;
    
    if(ctrlRec->get() == RECORD_ON)
    {
        if(ready == true)
        {
//      #ifdef SF_FORMAT_FLAC
            sf_write_float(sf, pSamples, iBufferSize);
            //qDebug() << "writing";
//      #endif
        }
        else
        {
            open();
        }
    }
    else
    {
        close();
    }
}

void WriteAudioFile::close()
{
    if(sf != NULL)
    {
        sf_close(sf);
        sf = NULL;
    }
    ready = false;
}
