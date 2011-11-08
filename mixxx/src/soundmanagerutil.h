/**
 * @file soundmanagerutil.h
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100611
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDMANAGERUTIL_U
#define SOUNDMANAGERUTIL_U

#include <QString>
#include <QtXml>
#include <QMutex>

#include "defs.h" // for CSAMPLE (???)

/**
 * @class ChannelGroup
 * @brief Describes a group of channels, typically a pair for stereo sound in
 *        Mixxx.
 */
class ChannelGroup {
public:
    ChannelGroup(unsigned char channelBase, unsigned char channels);
    unsigned char getChannelBase() const;
    unsigned char getChannelCount() const;
    bool operator==(const ChannelGroup &other) const;
    bool clashesWith(const ChannelGroup &other) const;
    unsigned int getHash() const;
private:
    unsigned char m_channelBase; // base (first) channel used on device
    unsigned char m_channels; // number of channels used (s/b 2 in most cases)
};

/**
 * @class AudioPath
 * @brief Describes a path for audio to take.
 * @note This needs a new name, the current one sucks. If you find one,
 *       feel free to rename as necessary.
 */
class AudioPath {
public:
    // XXX if you add a new type here, be sure to add it to the various
    // methods including getStringFromType, isIndexed, getTypeFromInt,
    // channelsNeededForType (if necessary), the subclasses' getSupportedTypes
    // (if necessary), etc. -- bkgood
    enum AudioPathType {
        MASTER,
        HEADPHONES,
        DECK,
        VINYLCONTROL,
        MICROPHONE,
        EXTPASSTHROUGH,
        INVALID, // if this isn't last bad things will happen -bkgood
    };
    AudioPath(unsigned char channelBase, unsigned char channels);
    AudioPathType getType() const;
    ChannelGroup getChannelGroup() const;
    unsigned char getIndex() const;
    bool operator==(const AudioPath &other) const;
    unsigned int getHash() const;
    bool channelsClash(const AudioPath &other) const;
    QString getString() const;
    static QString getStringFromType(AudioPathType type);
    static QString getTrStringFromType(AudioPathType type);
    static AudioPathType getTypeFromString(QString string);
    static bool isIndexed(AudioPathType type);
    static AudioPathType getTypeFromInt(int typeInt);
    static unsigned char channelsNeededForType(AudioPathType type);
protected:
    virtual void setType(AudioPathType type) = 0;
    AudioPathType m_type;
    ChannelGroup m_channelGroup;
    unsigned char m_index;
};

/**
 * @class AudioOutput
 * @extends AudioPath
 * @brief A source of audio in Mixxx that is to be output to a group of
 *        channels on an audio interface.
 */
class AudioOutput : public AudioPath {
public:
    AudioOutput(AudioPathType type = INVALID, unsigned char channelBase = 0,
                unsigned char index = 0);
    QDomElement toXML(QDomElement *element) const;
    static AudioOutput fromXML(const QDomElement &xml);
    static QList<AudioPathType> getSupportedTypes();
protected:
    void setType(AudioPathType type);
};

/**
 * @class AudioInput
 * @extends AudioPath
 * @brief A source of audio at a group of channels on an audio interface
 *        that is be processed in Mixxx.
 */
class AudioInput : public AudioPath {
public:
    AudioInput(AudioPathType type = INVALID, unsigned char channelBase = 0,
               unsigned char index = 0);
    QDomElement toXML(QDomElement *element) const;
    static AudioInput fromXML(const QDomElement &xml);
    static QList<AudioPathType> getSupportedTypes();
protected:
    void setType(AudioPathType type);
};

class AudioSource {
public:
    virtual const CSAMPLE* buffer(AudioOutput output) const = 0;
    virtual void onOutputConnected(AudioOutput output) { Q_UNUSED(output); };
    virtual void onOutputDisconnected(AudioOutput output) { Q_UNUSED(output); };
};

class AudioDestination {
public:
    virtual void receiveBuffer(AudioInput input, const short* pBuffer, unsigned int iNumFrames) = 0;
    virtual void onInputConnected(AudioInput input) { Q_UNUSED(input); };
    virtual void onInputDisconnected(AudioInput input) { Q_UNUSED(input); };
};

typedef AudioPath::AudioPathType AudioPathType;

// globals for QHash
unsigned int qHash(const ChannelGroup &group);
unsigned int qHash(const AudioOutput &output);
unsigned int qHash(const AudioInput &input);

#endif
