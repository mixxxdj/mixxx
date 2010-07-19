/**
 * @file audiopath.cpp
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

#include <QtCore>
#include "audiopath.h"

ChannelGroup::ChannelGroup(unsigned char channelBase, unsigned char channels)
  : m_channelBase(channelBase)
  , m_channels(channels) {
}

unsigned char ChannelGroup::getChannelBase() const {
    return m_channelBase;
}

unsigned char ChannelGroup::getChannelCount() const {
    return m_channels;
}

bool ChannelGroup::operator==(const ChannelGroup &other) const {
    return m_channelBase == other.m_channelBase
        && m_channels == other.m_channels;
}

bool ChannelGroup::clashesWith(const ChannelGroup &other) const {
    if (m_channels == 0 || other.m_channels == 0) {
        return false; // can't clash if there are no channels in use
    }
    return (m_channelBase > other.m_channelBase
        && m_channelBase <= other.m_channelBase + other.m_channels)
        ||
        (other.m_channelBase > m_channelBase
        && other.m_channelBase <= m_channelBase + m_channels);
}

AudioPath::AudioPath(unsigned char channelBase, unsigned char channels)
    : m_channelGroup(channelBase, channels) {
}

AudioPathType AudioPath::getType() const {
    return m_type;
}

ChannelGroup AudioPath::getChannelGroup() const {
    return m_channelGroup;
}

unsigned char AudioPath::getIndex() const {
    return m_index;
}

bool AudioPath::operator==(const AudioPath &other) const {
    return m_type == other.m_type
        && m_index == other.m_index;
}

unsigned int AudioPath::getHash() const {
    return 0 | (m_type << 8) | m_index;
}

bool AudioPath::channelsClash(const AudioPath &other) const {
    return m_channelGroup.clashesWith(other.m_channelGroup);
}

/**
 * Gives a string describing the AudioPath for user benefit.
 * @returns A QString. Ideally will use tr() for i18n but the rest of mixxx
 *          doesn't at the moment so worry about that later. :)
 */
QString AudioPath::getString() const {
    if (isIndexed(getType())) {
        return QString("%1 %2")
            .arg(getStringFromType(getType())).arg(m_index + 1);
    }
    return getStringFromType(getType());
}

//static
QString AudioPath::getStringFromType(AudioPathType type) {
    switch (type) {
    case INVALID:
        // this shouldn't happen but g++ complains if I don't
        // handle this -- bkgood
        return QString::fromAscii("Invalid");
    case MASTER:
        return QString::fromAscii("Master");
    case HEADPHONES:
        return QString::fromAscii("Headphones");
    case DECK:
        return QString::fromAscii("Deck");
    case VINYLCONTROL:
        return QString::fromAscii("Vinyl Control");
    case MICROPHONE:
        return QString::fromAscii("Microphone");
    case PASSTHROUGH:
        return QString::fromAscii("Passthrough");
    }
    return QString::fromAscii("Unknown path type %1").arg(type);
}

//static
AudioPathType getTypeFromString(QString string) {
    string = string.toLower();
    if (string == AudioPath::getStringFromType(AudioPath::MASTER).toLower()) {
        return AudioPath::MASTER;
    } else if (string == AudioPath::getStringFromType(AudioPath::HEADPHONES).toLower()) {
        return AudioPath::HEADPHONES;
    } else if (string == AudioPath::getStringFromType(AudioPath::DECK).toLower()) {
        return AudioPath::DECK;
    } else if (string == AudioPath::getStringFromType(AudioPath::VINYLCONTROL).toLower()) {
        return AudioPath::VINYLCONTROL;
    } else if (string == AudioPath::getStringFromType(AudioPath::MICROPHONE).toLower()) {
        return AudioPath::MICROPHONE;
    } else if (string == AudioPath::getStringFromType(AudioPath::PASSTHROUGH).toLower()) {
        return AudioPath::PASSTHROUGH;
    } else {
        return AudioPath::INVALID;
    }
}

//static
bool AudioPath::isIndexed(AudioPathType type) {
    switch (type) {
    case DECK:
    case VINYLCONTROL:
    case PASSTHROUGH:
    case MICROPHONE:
        return true;
    default:
        break;
    }
    return false;
}

// static
AudioPathType AudioPath::getTypeFromInt(int typeInt) {
    switch (typeInt) {
    case AudioPath::MASTER:
        return AudioPath::MASTER;
    case AudioPath::HEADPHONES:
        return AudioPath::HEADPHONES;
    case AudioPath::DECK:
        return AudioPath::DECK;
    case AudioPath::VINYLCONTROL:
        return AudioPath::VINYLCONTROL;
    case AudioPath::PASSTHROUGH:
        return AudioPath::PASSTHROUGH;
    case AudioPath::MICROPHONE:
        return AudioPath::MICROPHONE;
    default:
        return AudioPath::INVALID;
    }
}

//static
unsigned char AudioPath::channelsNeededForType(AudioPathType type) {
    switch (type) {
    case AudioPath::MICROPHONE:
        return 1;
    default:
        return 2;
    }
}

AudioSource::AudioSource(AudioPathType type /* = INVALID */,
        unsigned char channelBase /* = 0 */,
        unsigned char index /* = 0 */)
    : AudioPath(channelBase, AudioPath::channelsNeededForType(type)) {
    setType(type);
    if (isIndexed(type)) {
        m_index = index;
    } else {
        m_index = 0;
    }
}

//static
QList<AudioPathType> AudioSource::getSupportedTypes() {
    QList<AudioPathType> types;
    types.append(MASTER);
    types.append(HEADPHONES);
    types.append(DECK);
    return types;
}

// protected
void AudioSource::setType(AudioPathType type) {
    if (AudioSource::getSupportedTypes().contains(type)) {
        m_type = type;
    } else {
        m_type = AudioPath::INVALID;
    }
}


AudioReceiver::AudioReceiver(AudioPathType type /* = INVALID */,
        unsigned char channelBase /* = 0 */,
        unsigned char index /* = 0 */)
  : AudioPath(channelBase, AudioPath::channelsNeededForType(type)) {
    setType(type);
    if (isIndexed(type)) {
        m_index = index;
    } else {
        m_index = 0;
    }
}

//static
QList<AudioPathType> AudioReceiver::getSupportedTypes() {
    QList<AudioPathType> types;
#ifdef __VINYLCONTROL__
    // this disables vinyl control for all of the sound devices stuff
    // (prefs, etc), minimal ifdefs :) -- bkgood
    types.append(VINYLCONTROL);
#endif
    return types;
}

// protected
void AudioReceiver::setType(AudioPathType type) {
    if (AudioReceiver::getSupportedTypes().contains(type)) {
        m_type = type;
    } else {
        m_type = AudioPath::INVALID;
    }
}


unsigned int qHash(const AudioSource &src) {
    return src.getHash();
}

unsigned int qHash(const AudioReceiver &recv) {
    return recv.getHash();
}
