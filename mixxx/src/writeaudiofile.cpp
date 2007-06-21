#include "writeaudiofile.h"
#include "dlgprefrecord.h"
#include <sndfile.h>

WriteAudioFile::WriteAudioFile(ConfigObject<ConfigValue> *_config)
{
    ready = false;
    sf = NULL;
    config = _config;
}

WriteAudioFile::~WriteAudioFile()
{
    close();
}

void WriteAudioFile::open()
{
    const char *path = config->getValueString(ConfigKey(PREF_KEY,"Path")).ascii();
    int format = config->getValueString(ConfigKey(PREF_KEY, "Encoding")).toInt();

    ready = false;
    if(config->getValueString(ConfigKey(PREF_KEY, "Record")).compare("TRUE") == 0)
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
	    case IDEX_FLAC:
		sfInfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
		break;
	    default:
		qDebug("Corrupt record section in preference file");
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
	}
    }
}

void WriteAudioFile::write(const CSAMPLE *pIn, int iBufferSize)
{
    Q_ASSERT(iBufferSize % 2 == 0);
    if(config->getValueString(ConfigKey(PREF_KEY, "Record")).compare("TRUE") == 0)
    {
	if(ready == true)
	{
	    sf_write_float(sf, pIn, iBufferSize);
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
