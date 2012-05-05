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
#include "enginepfldelay.h"
#include "engine/syncworker.h"
#include "sampleutil.h"

#ifdef __LADSPA__
#include "engineladspa.h"
#endif


EngineMaster::EngineMaster(ConfigObject<ConfigValue> * _config,
                           const char * group,
                           bool bEnableSidechain) {

    m_pWorkerScheduler = new EngineWorkerScheduler(this);
    m_pWorkerScheduler->start();
    m_pSyncWorker = new SyncWorker(m_pWorkerScheduler);

    // Master sample rate
    m_pMasterSampleRate = new ControlObject(ConfigKey(group, "samplerate"));
    m_pMasterSampleRate->set(44100.);

    // Latency control
    m_pMasterLatency = new ControlObject(ConfigKey(group, "latency"));

    // Master rate
    m_pMasterRate = new ControlPotmeter(ConfigKey(group, "rate"), -1.0, 1.0);

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
    head_mix->setDefaultValue(-1.);
    head_mix->set(-1.);

    // Headphone Clipping
    head_clipping = new EngineClipping("");
    
    // Headphone Delay
    m_pHeadDelay = new EnginePflDelay();
    
    // Eq Bypass
    m_pBypassEq = new ControlPushButton(ConfigKey(group, "bypass_eq"));
    m_pBypassEq->setButtonMode(ControlPushButton::TOGGLE);
    
    //set up input passthrough o
    //TODO: we should set up n passthroughs for n decks
	m_passthrough.append(new ControlPushButton(ConfigKey("[Channel1]","inputpassthrough")));
	m_passthrough.append(new ControlPushButton(ConfigKey("[Channel2]","inputpassthrough")));
	m_passthrough[0]->setButtonMode(ControlPushButton::TOGGLE);
	m_passthrough[1]->setButtonMode(ControlPushButton::TOGGLE);
	m_bPassthroughWasActive[0] = false;
	m_bPassthroughWasActive[1] = false;
	m_iLastThruRead[0] = m_iLastThruRead[1] = -1;
    m_iLastThruWrote[0] = m_iLastThruWrote[1] = -1;
    m_iThruFill[0] = m_iThruFill[1] = 0;
    m_iThruBufferCount = _config->getValueString(ConfigKey("[Master]","passthrough_buffercount")).toInt();
    m_iThruBufferCount = math_max(1, m_iThruBufferCount);
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

    //Starts a thread for recording and shoutcast
    sidechain = NULL;
    if (bEnableSidechain) {
        sidechain = new EngineSideChain(_config);
        connect(sidechain, SIGNAL(isRecording(bool)),
                this, SIGNAL(isRecording(bool)));
        connect(sidechain, SIGNAL(bytesRecorded(int)),
                this, SIGNAL(bytesRecorded(int)));
    }

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
    delete m_pHeadDelay;
    delete clipping;
    delete vumeter;
    delete head_clipping;
    delete sidechain;

    delete xFaderCalibration;
    delete xFaderCurve;

    delete m_pMasterSampleRate;
    delete m_pMasterLatency;
    delete m_pMasterRate;

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

    delete m_pWorkerScheduler;
    delete m_pSyncWorker;
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
        
        bool needsProcessing = false;
        if (pChannel->isMaster()) {
            masterOutput |= (1 << channel_number);
            needsProcessing = true;
        }

        // If the channel is enabled for previewing in headphones, copy it
        // over to the headphone buffer
        if (pChannel->isPFL()) {
            headphoneOutput |= (1 << channel_number);
            needsProcessing = true;
        }

        //FIXME: this assumes second two decks are passthrough
        bool isPassthroughActive = false;
        int deck_index = channel_number-1;
        if (deck_index >= 0 && deck_index < 2)
        	isPassthroughActive = m_passthrough[deck_index]->get();
        	
        if(isPassthroughActive)
    	{
    		//if we're passing through, we don't care if channel
	    	//is active or not
    		//overwrite the channel buffer with the input so the decks, 
    		//not just the master get output
    		
    		passthroughBufferMutex[deck_index].lock();
    		
    		//currently playing silence while filling up the buffer
    		if (m_bFilling[deck_index])
    		{
    			if (m_iThruFill[deck_index] >= m_iThruBufferCount)
				{
					//qDebug() << "Buffer filled" << m_iThruFill[deck_index] << m_iThruBufferCount;
					m_bFilling[deck_index] = false;
				}
				else
				{
					//qDebug() << "Buffer filling" << m_iThruFill[deck_index] << m_iThruBufferCount;
					SampleUtil::applyGain(pChannelInfo->m_pBuffer, 0.0f, iBufferSize);
				}
			}
    		
    		if (!m_bFilling[deck_index])
    		{
    			if (m_iThruFill[deck_index] < 1)
				{
					qDebug() << "ERROR: input passthrough buffer underrun";
					SampleUtil::applyGain(pChannelInfo->m_pBuffer, 0.0f, iBufferSize);
					m_bFilling[deck_index] = true;
				}
				else
				{
				    m_iLastThruRead[deck_index] = (m_iLastThruRead[deck_index]+1) % m_iThruBufferCount;
					m_iThruFill[deck_index]--;
					//qDebug() << "read" << m_iLastThruRead[deck_index] << "fill" << m_iThruFill[deck_index];
					SampleUtil::copyWithGain(pChannelInfo->m_pBuffer,
											m_passthroughBuffers[deck_index]+m_iLastThruRead[deck_index]*iBufferSize, 
											1.0f, iBufferSize);
				}
			}
    		passthroughBufferMutex[deck_index].unlock();
	    	pChannel->process(pChannelInfo->m_pBuffer, pChannelInfo->m_pBuffer, iBufferSize);
	    }
	    else
	    {
	        if (deck_index >=0 && deck_index < 2)
        	{
        	    //if it was active last time, zero out
   	        	if (m_bPassthroughWasActive[deck_index])
   	        	{
   	        		SampleUtil::applyGain(pChannelInfo->m_pBuffer, 0.0f, iBufferSize);
   	        		m_bPassthroughWasActive[deck_index] = false;
					m_iLastThruRead[deck_index] = -1;
					m_iLastThruWrote[deck_index] = -1;
					m_iThruFill[deck_index] = 0;
					//m_iThruBufferCount[deck_index] = 1;
					m_bFilling[deck_index] = true;

   	        	}
   	        }
   	        if (!pChannel->isActive()) {
				continue;
    		}
    		if (needsProcessing)
    		{
				pChannel->process(NULL, pChannelInfo->m_pBuffer, iBufferSize);
			}
	    }
	    if (deck_index >=0 && deck_index < 2)
	    	m_bPassthroughWasActive[deck_index] = isPassthroughActive;
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
    if (sidechain != NULL) {
        sidechain->submitSamples(m_pMaster, iBufferSize);
    }

    // Add master to headphone with appropriate gain
    SampleUtil::addWithGain(m_pHead, m_pMaster, cmaster_gain, iBufferSize);

    // Head volume and clipping
    SampleUtil::applyGain(m_pHead, m_pHeadVolume->get(), iBufferSize);
    head_clipping->process(m_pHead, m_pHead, iBufferSize);
    
    //delay the headphone sound by the appropriate amount
    m_pHeadDelay->process(m_pHead, m_pHead, iBufferSize);

    //Master/headphones interleaving is now done in
    //SoundManager::requestBuffer() - Albert Nov 18/07

    // Schedule a ControlObject sync
    m_pSyncWorker->schedule();

    // We're close to the end of the callback. Wake up the engine worker
    // scheduler so that it runs the workers.
    m_pWorkerScheduler->runWorkers();
}

void EngineMaster::addChannel(EngineChannel* pChannel) {
    ChannelInfo* pChannelInfo = new ChannelInfo();
    pChannelInfo->m_pChannel = pChannel;
    pChannelInfo->m_pVolumeControl = new ControlLogpotmeter(
        ConfigKey(pChannel->getGroup(), "volume"), 1.0);
    pChannelInfo->m_pVolumeControl->setDefaultValue(1.0);
    pChannelInfo->m_pVolumeControl->set(1.0);
    pChannelInfo->m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    SampleUtil::applyGain(pChannelInfo->m_pBuffer, 0, MAX_BUFFER_LEN);
    m_channels.push_back(pChannelInfo);

    EngineBuffer* pBuffer = pChannelInfo->m_pChannel->getEngineBuffer();
    if (pBuffer != NULL) {
        pBuffer->bindWorkers(m_pWorkerScheduler);
    }

    // TODO(XXX) WARNING HUGE HACK ALERT In the case of 2-decks, this code hooks
    // the two EngineBuffers together so they can beat-sync off of each other.
    // rryan 6/2010
    bool isDeck1 = pChannel->getGroup() == "[Channel1]";
    bool isDeck2 = pChannel->getGroup() == "[Channel2]";
    if (isDeck1 || isDeck2) {
        QString otherGroup = isDeck1 ? "[Channel2]" : "[Channel1]";
        for (QList<ChannelInfo*>::const_iterator i = m_channels.constBegin();
             i != m_channels.constEnd(); ++i) {
            const ChannelInfo* pChannelInfo = *i;
            if (pChannelInfo->m_pChannel->getGroup() == otherGroup) {
                EngineBuffer *pBuffer1 = pChannel->getEngineBuffer();
                EngineBuffer *pBuffer2 = pChannelInfo->m_pChannel->getEngineBuffer();
                if (pBuffer1 != NULL && pBuffer2 != NULL) {
                    pBuffer1->setOtherEngineBuffer(pBuffer2);
                    pBuffer2->setOtherEngineBuffer(pBuffer1);
                }
            }
        }
    }
}

void EngineMaster::pushPassthroughBuffer(int c, short *input, int len)
{
	Q_ASSERT(c<2); // really, now.
	
	len /= 2; // this seems to be needed all of a sudden

	passthroughBufferMutex[c].lock();

	//check that we aren't overflowing.	
	if (m_iThruFill[c] >= m_iThruBufferCount)
	{
		qDebug() << "WARNING: Input Passthrough buffer overflow " << m_iThruBufferCount;
		return;
	}

    m_iLastThruWrote[c] = (m_iLastThruWrote[c] + 1) % m_iThruBufferCount;
	m_iThruFill[c]++;

	//qDebug() << "writ" << m_iLastThruWrote[c] << "fill" << m_iThruFill[c]; //  << &m_passthroughBuffers[c][(m_iLastThruWrote[c]*len)];

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
const CSAMPLE* EngineMaster::getDeckBuffer(unsigned int i) const {
    return getChannelBuffer(QString("[Channel%1]").arg(i+1));
}

const CSAMPLE* EngineMaster::getChannelBuffer(QString group) const {
    for (QList<ChannelInfo*>::const_iterator i = m_channels.constBegin();
         i != m_channels.constEnd(); ++i) {
        const ChannelInfo* pChannelInfo = *i;
        if (pChannelInfo->m_pChannel->getGroup() == group) {
            return pChannelInfo->m_pBuffer;
        }
    }
    return NULL;
}

const CSAMPLE* EngineMaster::buffer(AudioOutput output) const {
    switch (output.getType()) {
    case AudioOutput::MASTER:
        return getMasterBuffer();
        break;
    case AudioOutput::HEADPHONES:
        return getHeadphoneBuffer();
        break;
    case AudioOutput::DECK:
        return getDeckBuffer(output.getIndex());
        break;
    default:
        return NULL;
    }
}
