/**
 * @file audiopath.h
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

#ifndef AUDIOPATH_H
#define AUDIOPATH_H

#include <QString>

/**
 * @class ChannelGroup
 * @brief Describes a group of channels, typically a pair for stereo sound in Mixxx.
 */
class ChannelGroup {
public:
    ChannelGroup(unsigned int channelBase, unsigned int channels);
    unsigned int getChannelBase() const;
    unsigned int getChannelCount() const;
    bool operator==(const ChannelGroup &other) const;
    bool clashesWith(const ChannelGroup &other) const;
    unsigned int getHash() const;
private:
    unsigned int m_channelBase; // base (first) channel used on device 
    unsigned int m_channels; // number of channels used (s/b 2 in most cases)
};

/**
 * @class AudioPath
 * @brief Describes a path for audio to take.
 * @warning Subclass me before using!
 */
class AudioPath {
public:
    enum AudioPathType {
        MASTER,
        HEADPHONES,
        DECK,
        VINYLCONTROL,
        MICROPHONE,
        PASSTHROUGH
    };
    AudioPath(unsigned int channelBase, unsigned int channels);
    AudioPathType getType() const;
    ChannelGroup getChannelGroup() const;
    unsigned int getIndex() const;
    bool operator==(const AudioPath& other) const;
    unsigned int getHash() const;
    bool channelsClash(const AudioPath& other) const;
    QString getString() const;
    static QString getStringFromType(AudioPathType type);
    static bool isIndexable(AudioPathType type);
protected:
    AudioPathType m_type;
    ChannelGroup m_channelGroup;
    unsigned int m_index;
};

/**
 * @class AudioSource
 * @extends AudioPath
 * @brief A source of audio in Mixxx that is to be output to a group of
 *        channels on an audio interface.
 */
class AudioSource : public AudioPath {
public:
    AudioSource(AudioPathType type, unsigned int channelBase,
                unsigned int channels, unsigned int index = 0);
    static QList<AudioPathType> getSupportedTypes();
};

/**
 * @class AudioReceiver
 * @extends AudioPath
 * @brief A source of audio at a group of channels on an audio interface
 *        that is be processed in Mixxx.
 */
class AudioReceiver : public AudioPath {
public:
    AudioReceiver(AudioPathType type, unsigned int channelBase,
                  unsigned int channels, unsigned int index = 0);
    static QList<AudioPathType> getSupportedTypes();
};

// globals for QHash
unsigned int qHash(const AudioSource &src);
unsigned int qHash(const AudioReceiver &recv);

#endif
