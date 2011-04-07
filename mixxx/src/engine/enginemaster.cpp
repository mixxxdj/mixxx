/***************************************************************************
                          enginemaster.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QDebug>
#include <QList>
#include <QPair>

#include "controlpushbutton.h"
#include "configobject.h"
#include "controllogpotmeter.h"
#include "controlpotmeter.h"
#include "enginebuffer.h"
#include "enginemaster.h"
#include "engine/engineworkerscheduler.h"
#include "enginebuffer.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "enginevumeter.h"
#include "enginexfader.h"
#include "enginesidechain.h"
#include "sampleutil.h"

#ifdef __LADSPA__
#include "engineladspa.h"
#endif


EngineMaster::EngineMaster(ConfigObject<ConfigValue> * _config,
                           const char * group) {

    m_pWorkerScheduler = new EngineWorkerScheduler(this);

    // Master sample rate
    ControlObject * sr = new ControlObject(ConfigKey(group, "samplerate"));
    sr->set(44100.);

    // Latency control
    new ControlObject(ConfigKey(group, "latency"));

    // Master rate
    new ControlPotmeter(ConfigKey(group, "rate"), -1.0, 1.0);

#ifdef __LADSPA__
    // LADSPA
    ladspa = new EngineLADSPA();
#endif

    // Crossfader
    crossfader = new ControlPotmeter(ConfigKey(group, "crossfader"),-1.,1.);

    // Balance
    m_pBalance = new ControlPotmeter(ConfigKey(group, "balance"), -1., 1.);

    // Master volume
    m_pMasterVolume = new ControlLogpotmeter(ConfigKey(group, "volume"), 5.);

    // Clipping
    clipping = new EngineClipping(group);

    // VU meter:
    vumeter = new EngineVuMeter(group);

    // Headphone volume
    m_pHeadVolume = new ControlLogpotmeter(ConfigKey(group, "headVolume"), 5.);

    // Headphone mix (left/right)
    head_mix = new ControlPotmeter(ConfigKey(group, "headMix"),-1.,1.);
    head_mix->set(-1.);

    // Headphone Clipping
    head_clipping = new EngineClipping("");
    
    //set up input passthrough o
	m_passthrough.append(new ControlPushButton(ConfigKey("[Channel1]","inputpassthrough")));
	m_passthrough.append(new ControlPushButton(ConfigKey("[Channel2]","inputpassthrough")));
	m_passthrough[0]->setToggleButton(true);
	m_passthrough[1]->setToggleButton(true);
	m_bPassthroughWasActive[0] = false;
	m_bPassthroughWasActive[1] = false;
	m_iLastThruRead[0] = m_iLastThruRead[1] = -1;
    m_iLastThruWrote[0] = m_iLastThruWrote[1] = -1;
    m_iThruFill[0] = m_iThruFill[1] = 0;
    m_iThruBufferCount[0] = m_iThruBufferCount[1] = 1;
    m_bFilling[0] = m_bFilling[1] = true;

    // Allocate buffers
    m_pHead = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pMaster = SampleUtil::alloc(MAX_BUFFER_LEN);
    memset(m_pHead, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pMaster, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
	m_passthroughBuffers.append(SampleUtil::alloc(MAX_BUFFER_LEN));
	m_passthroughBuffers.append(SampleUtil::alloc(MAX_BUFFER_LEN));
	memset(m_passthroughBuffers[0], 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
	memset(m_passthroughBuffers[1], 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
	    
    sidechain = new EngineSideChain(_config);

	//df.setFileName("mixxx-debug.csv");
	//df.open(QIODevice::WriteOnly | QIODevice::Text);
	//writer.setDevice(&df);

    //X-Fader Setup
    xFaderCurve = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderCurve"), 0., 2.);
    xFaderCalibration = new ControlPotmeter(
        ConfigKey("[Mixer Profile]", "xFaderCalibration"), -2., 2.);
}

EngineMaster::~EngineMaster()
{
    qDebug() << "in ~EngineMaster()";
    delete crossfader;
    delete m_pBalance;
    delete head_mix;
    delete m_pMasterVolume;
    delete m_pHeadVolume;
    delete clipping;
    delete vumeter;
    delete head_clipping;
    delete sidechain;

    delete xFaderCalibration;
    delete xFaderCurve;

    SampleUtil::free(m_pHead);
    SampleUtil::free(m_pMaster);

    QMutableListIterator<ChannelInfo*> channel_it(m_channels);
    while (channel_it.hasNext()) {
        ChannelInfo* pChannelInfo = channel_it.next();
        channel_it.remove();
        SampleUtil::free(pChannelInfo->m_pBuffer);
        delete pChannelInfo->m_pChannel;
        delete pChannelInfo->m_pVolumeControl;
        delete pChannelInfo;
    }
}

const CSAMPLE* EngineMaster::getMasterBuffer() const
{
    return m_pMaster;
}

const CSAMPLE* EngineMaster::getHeadphoneBuffer() const
{
    return m_pHead;
}

void EngineMaster::mixChannels(unsigned int channelBitvector, unsigned int maxChannels,
                               CSAMPLE* pOutput, unsigned int iBufferSize,
                               GainCalculator* pGainCalculator) {
    // Common case: 2 decks, 4 samplers, 1 mic
    ChannelInfo* pChannel1 = NULL;
    ChannelInfo* pChannel2 = NULL;
    ChannelInfo* pChannel3 = NULL;
    ChannelInfo* pChannel4 = NULL;
    ChannelInfo* pChannel5 = NULL;
    ChannelInfo* pChannel6 = NULL;
    ChannelInfo* pChannel7 = NULL;

    unsigned int totalActive = 0;
    for (unsigned int i = 0; i < maxChannels; ++i) {
        if ((channelBitvector & (1 << i)) == 0) {
            continue;
        }

        ++totalActive;

        if (pChannel1 == NULL) {
            pChannel1 = m_channels[i];
        } else if (pChannel2 == NULL) {
            pChannel2 = m_channels[i];
        } else if (pChannel3 == NULL) {
            pChannel3 = m_channels[i];
        } else if (pChannel4 == NULL) {
            pChannel4 = m_channels[i];
        } else if (pChannel5 == NULL) {
            pChannel5 = m_channels[i];
        } else if (pChannel6 == NULL) {
            pChannel6 = m_channels[i];
        } else if (pChannel7 == NULL) {
            pChannel7 = m_channels[i];
        }
    }

    if (totalActive == 0) {
        SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);
    } else if (totalActive == 1) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        double gain1 = pGainCalculator->getGain(pChannel1);
        SampleUtil::copyWithGain(pOutput,
                                 pBuffer1, gain1,
                                 iBufferSize);
    } else if (totalActive == 2) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        double gain1 = pGainCalculator->getGain(pChannel1);
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        double gain2 = pGainCalculator->getGain(pChannel2);
        SampleUtil::copy2WithGain(pOutput,
                                  pBuffer1, gain1,
                                  pBuffer2, gain2,
                                  iBufferSize);
    } else if (totalActive == 3) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        double gain1 = pGainCalculator->getGain(pChannel1);
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        double gain2 = pGainCalculator->getGain(pChannel2);
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        double gain3 = pGainCalculator->getGain(pChannel3);

        SampleUtil::copy3WithGain(pOutput,
                                  pBuffer1, gain1,
                                  pBuffer2, gain2,
                                  pBuffer3, gain3,
                                  iBufferSize);
    } else if (totalActive == 4) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        double gain1 = pGainCalculator->getGain(pChannel1);
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        double gain2 = pGainCalculator->getGain(pChannel2);
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        double gain3 = pGainCalculator->getGain(pChannel3);
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        double gain4 = pGainCalculator->getGain(pChannel4);
        SampleUtil::copy4WithGain(pOutput,
                                  pBuffer1, gain1,
                                  pBuffer2, gain2,
                                  pBuffer3, gain3,
                                  pBuffer4, gain4,
                                  iBufferSize);
    } else if (totalActive == 5) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        double gain1 = pGainCalculator->getGain(pChannel1);
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        double gain2 = pGainCalculator->getGain(pChannel2);
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        double gain3 = pGainCalculator->getGain(pChannel3);
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        double gain4 = pGainCalculator->getGain(pChannel4);
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        double gain5 = pGainCalculator->getGain(pChannel5);

        SampleUtil::copy5WithGain(pOutput,
                                  pBuffer1, gain1,
                                  pBuffer2, gain2,
                                  pBuffer3, gain3,
                                  pBuffer4, gain4,
                                  pBuffer5, gain5,
                                  iBufferSize);
    } else if (totalActive == 6) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        double gain1 = pGainCalculator->getGain(pChannel1);
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        double gain2 = pGainCalculator->getGain(pChannel2);
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        double gain3 = pGainCalculator->getGain(pChannel3);
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        double gain4 = pGainCalculator->getGain(pChannel4);
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        double gain5 = pGainCalculator->getGain(pChannel5);
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        double gain6 = pGainCalculator->getGain(pChannel6);
        SampleUtil::copy6WithGain(pOutput,
                                  pBuffer1, gain1,
                                  pBuffer2, gain2,
                                  pBuffer3, gain3,
                                  pBuffer4, gain4,
                                  pBuffer5, gain5,
                                  pBuffer6, gain6,
                                  iBufferSize);
    } else if (totalActive == 7) {
        CSAMPLE* pBuffer1 = pChannel1->m_pBuffer;
        double gain1 = pGainCalculator->getGain(pChannel1);
        CSAMPLE* pBuffer2 = pChannel2->m_pBuffer;
        double gain2 = pGainCalculator->getGain(pChannel2);
        CSAMPLE* pBuffer3 = pChannel3->m_pBuffer;
        double gain3 = pGainCalculator->getGain(pChannel3);
        CSAMPLE* pBuffer4 = pChannel4->m_pBuffer;
        double gain4 = pGainCalculator->getGain(pChannel4);
        CSAMPLE* pBuffer5 = pChannel5->m_pBuffer;
        double gain5 = pGainCalculator->getGain(pChannel5);
        CSAMPLE* pBuffer6 = pChannel6->m_pBuffer;
        double gain6 = pGainCalculator->getGain(pChannel6);
        CSAMPLE* pBuffer7 = pChannel7->m_pBuffer;
        double gain7 = pGainCalculator->getGain(pChannel7);
        SampleUtil::copy7WithGain(pOutput,
                                  pBuffer1, gain1,
                                  pBuffer2, gain2,
                                  pBuffer3, gain3,
                                  pBuffer4, gain4,
                                  pBuffer5, gain5,
                                  pBuffer6, gain6,
                                  pBuffer7, gain7,
                                  iBufferSize);
    } else {
        // Set pOutput to all 0s
        SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);

        for (unsigned int i = 0; i < maxChannels; ++i) {
            if (channelBitvector & (1 << i)) {
                ChannelInfo* pChannelInfo = m_channels[i];
                CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;
                double gain = pGainCalculator->getGain(pChannelInfo);
                SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);
            }
        }
    }
}

void EngineMaster::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE **pOutput = (CSAMPLE**)pOut;
    Q_UNUSED(pOutput);

    // Prepare each channel for output

    // Bitvector of enabled channels
    const unsigned int maxChannels = 32;
    unsigned int masterOutput = 0;
    unsigned int headphoneOutput = 0;

    // Compute headphone mix
    // Head phone left/right mix
    float cf_val = head_mix->get();
    float chead_gain = 0.5*(-cf_val+1.);
    float cmaster_gain = 0.5*(cf_val+1.);
    // qDebug() << "head val " << cf_val << ", head " << chead_gain
    //          << ", master " << cmaster_gain;

    QList<ChannelInfo*>::iterator it = m_channels.begin();
    for (unsigned int channel_number = 0;
         it != m_channels.end(); ++it, ++channel_number) {
        ChannelInfo* pChannelInfo = *it;
        EngineChannel* pChannel = pChannelInfo->m_pChannel;
        
        CSAMPLE* buffer;
        
        bool isactive = m_passthrough[channel_number]->get();
        if(isactive)
    	{
    		//if we're passing through, we don't care if channel
	    	//is active or not
    		//overwrite the channel buffer with the input so the decks, 
    		//not just the master get output
    		
    		buffer = pChannelInfo->m_pBuffer;
    		passthroughBufferMutex[channel_number].lock();
    		
    		//currently playing silence while filling up the buffer
    		if (m_bFilling[channel_number])
    		{
    			if (m_iThruFill[channel_number] < math_max(1, m_iThruBufferCount[channel_number] - 2))
				{
					qDebug() << "Buffer filling" << m_iThruFill[channel_number] << math_max(1, m_iThruBufferCount[channel_number] - 2);
					SampleUtil::applyGain(buffer, 0.0f, iBufferSize);
				}
				else
				{
					qDebug() << "Buffer filled";
					m_bFilling[channel_number] = false;
				}
			}
    		
    		if (!m_bFilling[channel_number])
    		{
    			if (m_iThruFill[channel_number] == 0)
				{
					qDebug() << "ERROR: input passthrough buffer underrun";
					SampleUtil::applyGain(buffer, 0.0f, iBufferSize);
					m_bFilling[channel_number] = true;
				}
				else
				{
					if (m_iLastThruRead[channel_number] >= m_iThruBufferCount[channel_number] - 1)
						m_iLastThruRead[channel_number] = 0;
					else
						m_iLastThruRead[channel_number]++;
					m_iThruFill[channel_number]--;
					SampleUtil::copyWithGain(buffer,
											m_passthroughBuffers[channel_number]+m_iLastThruRead[channel_number]*iBufferSize, 
											1.0f, iBufferSize);
				}
			}
    		passthroughBufferMutex[channel_number].unlock();
	    	pChannel->process(buffer, buffer, iBufferSize);
	    }
	    else
	    {
   	        if (!pChannel->isActive()) {
   	        	//if it was active last time, zero out
   	        	if (m_bPassthroughWasActive[channel_number])
   	        	{
   	        		//SampleUtil::applyGain(m_channelBuffers[channel_number], 0.0f, iBufferSize);
   	        		m_bPassthroughWasActive[channel_number] = false;
					m_iLastThruRead[channel_number] = -1;
					m_iLastThruWrote[channel_number] = -1;
					m_iThruFill[channel_number] = 0;
					m_iThruBufferCount[channel_number] = 1;
					m_bFilling[channel_number] = true;

   	        	}
				continue;
    		}

	    	buffer = m_channelBuffers[channel_number];
			//channel->process(NULL, buffer, iBufferSize);
			pChannel->process(NULL, buffer, iBufferSize);
	    }
	    m_bPassthroughWasActive[channel_number] = isactive;

        masterOutput |= (1 << channel_number);

        //// Process the buffer
        //pChannel->process(NULL, pChannelInfo->m_pBuffer, iBufferSize);
        masterOutput |= (1 << channel_number);

        // Process the buffer
        pChannel->process(NULL, pChannelInfo->m_pBuffer, iBufferSize);

        // If the channel is enabled for previewing in headphones, copy it
        // over to the headphone buffer
        if (pChannel->isPFL()) {
            headphoneOutput |= (1 << channel_number);
        }
    }

    // Mix all the enabled headphone channels together.
    m_headphoneGain.setGain(chead_gain);
    mixChannels(headphoneOutput, maxChannels, m_pHead, iBufferSize, &m_headphoneGain);

    // Calculate the crossfader gains for left and right side of the crossfader
    float c1_gain, c2_gain;
    EngineXfader::getXfadeGains(c1_gain, c2_gain,
                                crossfader->get(), xFaderCurve->get(),
                                xFaderCalibration->get());

    // Now set the gains for overall volume and the left, center, right gains.
    m_masterGain.setGains(m_pMasterVolume->get(), c1_gain, 1.0, c2_gain);

    // Perform the master mix
    mixChannels(masterOutput, maxChannels, m_pMaster, iBufferSize, &m_masterGain);

#ifdef __LADSPA__
    // LADPSA master effects
    ladspa->process(m_pMaster, m_pMaster, iBufferSize);
#endif

    // Clipping
    clipping->process(m_pMaster, m_pMaster, iBufferSize);
    
    // Balance values
    float balright = 1.;
    float balleft = 1.;
    float bal = m_pBalance->get();
    if (bal>0.)
        balleft -= bal;
    else if (bal<0.)
        balright += bal;

    // Perform balancing on main out
    SampleUtil::applyAlternatingGain(m_pMaster, balleft, balright, iBufferSize);

    // Update VU meter (it does not return anything). Needs to be here so that
    // master balance is reflected in the VU meter.
    if (vumeter != NULL)
        vumeter->process(m_pMaster, m_pMaster, iBufferSize);

    //Submit master samples to the side chain to do shoutcasting, recording,
    //etc.  (cpu intensive non-realtime tasks)
    sidechain->submitSamples(m_pMaster, iBufferSize);

    // Add master to headphone with appropriate gain
    SampleUtil::addWithGain(m_pHead, m_pMaster, cmaster_gain, iBufferSize);

    // Head volume and clipping
    SampleUtil::applyGain(m_pHead, m_pHeadVolume->get(), iBufferSize);
    head_clipping->process(m_pHead, m_pHead, iBufferSize);

    //Master/headphones interleaving is now done in
    //SoundManager::requestBuffer() - Albert Nov 18/07

    // We're close to the end of the callback. Schedule the workers. Hopefully
    // the work thread doesn't get scheduled between now and then.
    m_pWorkerScheduler->runWorkers();
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    ChannelInfo* pChannelInfo = new ChannelInfo();
    pChannelInfo->m_pChannel = pChannel;
    pChannelInfo->m_pVolumeControl = new ControlLogpotmeter(
        ConfigKey(pChannel->getGroup(), "volume"), 1.0);
    pChannelInfo->m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    memset(pChannelInfo->m_pBuffer, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    m_channels.push_back(pChannelInfo);
    pChannelInfo->m_pChannel->getEngineBuffer()->bindWorkers(m_pWorkerScheduler);

    // TODO(XXX) WARNING HUGE HACK ALERT In the case of 2-decks, this code hooks
    // the two EngineBuffers together so they can beat-sync off of each other.
    // rryan 6/2010
    if (m_channels.length() == 2) {
        EngineBuffer *pBuffer1 = m_channels[0]->m_pChannel->getEngineBuffer();
        EngineBuffer *pBuffer2 = m_channels[1]->m_pChannel->getEngineBuffer();
        pBuffer1->setOtherEngineBuffer(pBuffer2);
        pBuffer2->setOtherEngineBuffer(pBuffer1);
    }
}

void EngineMaster::pushPassthroughBuffer(int c, short *input, int len)
{
	Q_ASSERT(c<2); // really, now.

	passthroughBufferMutex[c].lock();

	//check that we aren't overflowing.	
	if (m_iThruFill[c] == m_iThruBufferCount[c])
	{
		if ((m_iThruBufferCount[c] + 1) * len > MAX_BUFFER_LEN)
		{
			qDebug() << "ERROR: Input Passthrough buffer overflow -- can't add buffers";
		}
		else
		{
			qDebug() << "WARNING: Input Passthrough buffer overflow, adding another. total:" << m_iThruBufferCount[c]+1;
			m_bFilling[c]=true;
			m_iThruBufferCount[c]++;
			//move the read pointer ahead too -- since my "pointers" are going
			//to be equal when buffer overflows, this is not really necessary
			if (m_iLastThruRead[c] >= m_iLastThruWrote[c])
				m_iLastThruRead[c]++;
		
			//move memory:
			// destination is two buffers away from last written
			// source is one away from last written
			// length is distance from end of everything to the destination
		
			//no need to move if we're at the end of the old buffers
			if(m_iLastThruWrote[c] < m_iThruBufferCount[c] - 2)
				memmove(m_passthroughBuffers[c]+ (m_iLastThruWrote[c] + 2) * len, 
						m_passthroughBuffers[c]+ (m_iLastThruWrote[c] + 1) * len, 
						(int)(m_iThruBufferCount[c] - (m_iLastThruWrote[c] + 2)) * len * sizeof(m_passthroughBuffers[c]));
		
			//now we can write to 3, say, and it will read from, say, 4.			
		}
	}

	if (m_iLastThruWrote[c] >= m_iThruBufferCount[c] - 1)
		m_iLastThruWrote[c] = 0;
	else
		m_iLastThruWrote[c]++;
	m_iThruFill[c]++;

	//qDebug() << "writ" << m_iLastThruWrote[c] << "fill" << m_iThruFill[c]  << &m_passthroughBuffers[c][(m_iLastThruWrote[c]*len)];

	for (int i=0; i<len; i++)
	{
		//why don't we need to divide by SHRT_MAX???
		m_passthroughBuffers[c][(m_iLastThruWrote[c]*len) + i] = (CSAMPLE)input[i];// / (float)SHRT_MAX;
		/*m_passthroughBuffers[c][i] = (CSAMPLE)input[i];// / (float)SHRT_MAX;*/
		//qDebug() << i << (CSAMPLE)input[i] << m_passthroughBuffers[c][(m_iLastThruWrote[c]*len) + i];
		//if (i % 2 == 0)
			//writer << i << "," <<(CSAMPLE)input[i] << "\n";
	}
	//qDebug() << "write" << m_iLastThruWrote[c] << (m_iLastThruWrote[c]*len) << (m_iLastThruWrote[c]*len) + len - 1;

	passthroughBufferMutex[c].unlock();
}

int EngineMaster::numChannels() const {
    return m_channels.size();
}

const CSAMPLE* EngineMaster::getChannelBuffer(unsigned int i) const {
    if (i < numChannels()) {
        return m_channels[i]->m_pBuffer;
    }
    return NULL;
}
