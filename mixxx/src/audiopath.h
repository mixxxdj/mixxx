/**
 * @file audiopath.h
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100611
 * @note This file/these classes use uchar instead of uint because they are
 *       passed around a lot in code which needs to be fast (portaudio
 *       callback). Using uchars means an AudioPath should fit in 4 bytes of
 *       memory and hopefully last longer in high-level cache -- hopefully.
 *       None of these values should really be >255 anyway. -- bkgood
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDIOPATH_H
#define AUDIOPATH_H

#include <QString>

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
 * @warning Subclass me before using!
 */
class AudioPath {
public:
    // XXX if you add a new type here, be sure to add it to the various
    // methods including getStringFromType, isIndexable, getTypeFromInt,
    // channelsNeededForType (if necessary), the subclasses' getSupportedTypes
    // (if necessary), etc. -- bkgood
    enum AudioPathType {
        INVALID,
        MASTER,
        HEADPHONES,
        DECK,
        VINYLCONTROL,
        MICROPHONE,
        PASSTHROUGH,
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
    static AudioPathType getTypeFromString(QString string);
    static bool isIndexable(AudioPathType type);
    static AudioPathType getTypeFromInt(int typeInt);
    static unsigned char channelsNeededForType(AudioPathType type);
protected:
    virtual void setType(AudioPathType type) = 0;
    AudioPathType m_type;
    ChannelGroup m_channelGroup;
    unsigned char m_index;
};

/**
 * @class AudioSource
 * @extends AudioPath
 * @brief A source of audio in Mixxx that is to be output to a group of
 *        channels on an audio interface.
 */
class AudioSource : public AudioPath {
public:
    AudioSource(AudioPathType type = INVALID, unsigned char channelBase = 0,
                unsigned char index = 0);
    static QList<AudioPathType> getSupportedTypes();
protected:
    void setType(AudioPathType type);
};

/**
 * @class AudioReceiver
 * @extends AudioPath
 * @brief A source of audio at a group of channels on an audio interface
 *        that is be processed in Mixxx.
 */
class AudioReceiver : public AudioPath {
public:
    AudioReceiver(AudioPathType type = INVALID, unsigned char channelBase = 0,
                  unsigned char index = 0);
    static QList<AudioPathType> getSupportedTypes();
protected:
    void setType(AudioPathType type);
};

// globals for QHash
unsigned int qHash(const AudioSource &src);
unsigned int qHash(const AudioReceiver &recv);

#endif
