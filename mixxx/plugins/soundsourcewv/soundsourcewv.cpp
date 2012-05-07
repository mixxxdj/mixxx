//soundsourcewv.cpp : sound source proxy for .wv (WavPack files)
//created by fenugrec
//great help from rryan & others on #mixxx
//format_samples adapted from cmus (Peter Lemenkov)

#include <QtDebug>

#include <taglib/wavpackfile.h>

#include "soundsourcewv.h"

namespace Mixxx {

SoundSourceWV::SoundSourceWV(QString qFilename) : SoundSource(qFilename)
{
	// Initialize variables to invalid values in case loading fails.
	filewvc=NULL;
}


SoundSourceWV::~SoundSourceWV(){
   if (filewvc) {
	WavpackCloseFile(filewvc);
	filewvc=NULL;
   }
}

QList<QString> SoundSourceWV::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("wv");
    return list;
}


int SoundSourceWV::open()
{
	QByteArray qBAFilename = m_qFilename.toUtf8();
	char msg[80];	//hold posible error message

	filewvc = WavpackOpenFileInput(qBAFilename.data(),msg,OPEN_2CH_MAX | OPEN_WVC,0);
	if (!filewvc) {
		qDebug() << "SSWV::open: failed to open file : "<<msg;
		return ERR;
	}
	if (WavpackGetMode(filewvc) & MODE_FLOAT) {
		qDebug() << "SSWV::open: cannot load 32bit float files";
		WavpackCloseFile(filewvc);
		filewvc=NULL;
		return ERR;
	}
	// wavpack_open succeeded -> populate variables
	filelength = WavpackGetNumSamples(filewvc);
	m_iSampleRate=WavpackGetSampleRate(filewvc);
	m_iChannels=WavpackGetReducedChannels(filewvc);
	Bps=WavpackGetBytesPerSample(filewvc);
	qDebug () << "SSWV::open: opened filewvc with filelength: "<<filelength<<" SampleRate: " << m_iSampleRate
		<< " channels: " << m_iChannels << " bytes per samp: "<<Bps;
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
		timesamps=(size-sampsread)>>1;		//timesamps still remaining
		if (timesamps > (WV_BUF_LENGTH/m_iChannels)) {	//if requested size requires more than one buffer filling
			timesamps=(WV_BUF_LENGTH/m_iChannels);		//tempbuffer must hold (timesamps * channels) samples
			qDebug() << "SSWV::read : performance warning, size requested > buffer size !";
		}

		tsdone=WavpackUnpackSamples(filewvc, tempbuffer, timesamps);	//fill temp buffer with timesamps*4bytes*channels
				//data is right justified, format_samples() fixes that.

		SoundSourceWV::format_samples(Bps, (char *) (dest + (sampsread>>1)*m_iChannels), tempbuffer, tsdone*m_iChannels);
								//this will unpack the 4byte/sample
								//output of wUnpackSamples(), sign-extending or truncating to output 16bit / sample.
								//specifying dest+sampsread should resume the conversion where it was left if size requested
								//required multiple reads (size req. > fixed buffer size)

		sampsread = sampsread + (tsdone<<1);
		if (tsdone!=timesamps) {
			qDebug () << "SSWV::read : WavpackUnpackSamples read "<<sampsread<<" asamps out of "<<size<<" requested";
			break;	//exit the while loop : subsequent reads are sure to read less than required.
		}

	}

	if (m_iChannels==1) {		//if MONO : expand array to double it's size; see ssov.cpp
		for(int i=(sampsread/2-1); i>=0; i--) {	//algo courtesy of rryan !
			dest[i*2]     = dest[i];	//go through array backwards, expanding and copying L -> R
			dest[(i*2)+1] = dest[i];
		}
	}

	return sampsread;
}


inline long unsigned SoundSourceWV::length(){
	//filelength is # of timesamps.
	return filelength<<1;
}


int SoundSourceWV::parseHeader() {
    setType("wv");

#ifdef __WINDOWS__
    /* From Tobias: A Utf-8 string did not work on my Windows XP (German edition)
     * If you try this conversion, f.isValid() will return false in many cases
     * and processTaglibFile() will fail
     *
     * The method toLocal8Bit() returns the local 8-bit representation of the string as a QByteArray.
     * The returned byte array is undefined if the string contains characters not supported
     * by the local 8-bit encoding.
     */
    QByteArray qBAFilename = m_qFilename.toLocal8Bit();
#else
    QByteArray qBAFilename = m_qFilename.toUtf8();
#endif
    TagLib::WavPack::File f(qBAFilename.constData());

    // Takes care of all the default metadata
    bool result = processTaglibFile(f);

    TagLib::APE::Tag *ape = f.APETag();
    if (ape) {
        processAPETag(ape);
    }

    if (result)
        return OK;
    return ERR;
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
			*dst++ = (char) 0;		//left shift the 8 bit sample
			*dst++ = (char) *src++ ;//+ 128;		//only works with u8int ?
		}
		break;
	case 2:
		while (count--) {
			*dst++ = (char) (temp = *src++);	//low byte
			*dst++ = (char) (temp >> 8);		//high byte
		}
		break;
	case 3:	//modified to truncate to 16bits
		while (count--) {
			*dst++ = (char) (temp = (*src++) >> 8);
			*dst++ = (char) (temp >> 8);
		}
		break;
	case 4:	//also truncates
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

}  // namespace Mixxx
