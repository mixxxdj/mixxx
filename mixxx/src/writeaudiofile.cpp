#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "writeaudiofile.h"
#include "dlgprefrecord.h"
#include <sndfile.h>
#include <QtDebug>

WriteAudioFile::WriteAudioFile(ConfigObject<ConfigValue> * _config)
{
    ready = false;
    sf = NULL;
    config = _config;
    ctrlRec = new ControlObject(ConfigKey("[Master]", "Record"));
}

WriteAudioFile::~WriteAudioFile()
{
    delete ctrlRec;
    close();
}

void WriteAudioFile::open()
{
    const char * path = config->getValueString(ConfigKey(PREF_KEY,"Path")).latin1();
    int format = config->getValueString(ConfigKey(PREF_KEY, "Encoding")).toInt();

    ready = false;
    if(ctrlRec->get() == RECORD_ON)
    {
        //if the record flag is set

        //set sfInfo
        sfInfo.samplerate = 44100;
        sfInfo.channels = 2;
        switch(format)
        {
        case IDEX_WAVE:
            sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
            break;
        case IDEX_AIFF:
            sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
            break;
            //# TODO: Fix code SF_FORMAT_FLAC block below to work on Windows
    #ifdef SF_FORMAT_FLAC
        case IDEX_FLAC:
            sfInfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
            break;
    #endif
        default:
            qDebug("Corrupt record section in preference file: %d", format);
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
            ret = sf_set_string(sf, SF_STR_TITLE, config->getValueString(ConfigKey(PREF_KEY, "Title")).ascii());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
            ret = sf_set_string(sf, SF_STR_ARTIST, config->getValueString(ConfigKey(PREF_KEY, "Author")).ascii());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
            ret = sf_set_string(sf, SF_STR_COMMENT, config->getValueString(ConfigKey(PREF_KEY, "Comment")).ascii());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

            ret = sf_set_string(sf, SF_STR_TITLE, "Testing");
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
        }
    }
}

void WriteAudioFile::write(const CSAMPLE * pIn, int iBufferSize)
{
    Q_ASSERT(iBufferSize % 2 == 0);
    if(ControlObject::getControl(ConfigKey("[Master]", "Record"))->get() == RECORD_ON)
    {
        if(ready == true)
        {
      #ifdef SF_FORMAT_FLAC
            sf_write_float(sf, pIn, iBufferSize);
      #endif
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
        sf_close(sf);
    ready = false;
}
