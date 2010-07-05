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

unsigned int ChannelGroup::getHash() const {
    return ((m_channelBase & 0xFF) << 8) | (m_channels & 0xFF);
}

AudioPath::AudioPath(unsigned char channelBase, unsigned char channels)
    : m_channelGroup(channelBase, channels) {
}

AudioPath::AudioPathType AudioPath::getType() const {
    // m_type is only stored in setType, which is safe, so this is safe
    return AudioPath::AudioPathType(m_type);
}

// protected
void AudioPath::setType(AudioPath::AudioPathType type) {
    // this method is guaranteed safe as long as the caller gives
    // a proper AudioPathType, i.e. it is as safe as it was when storing m_type
    // as AudioPathType proper -- bkgood
    m_type = (unsigned char) type;
}

ChannelGroup AudioPath::getChannelGroup() const {
    return m_channelGroup;
}

unsigned char AudioPath::getIndex() const {
    return m_index;
}

bool AudioPath::operator==(const AudioPath& other) const {
    return m_type == other.m_type
        && m_index == other.m_index
        && m_channelGroup == other.m_channelGroup;
}

unsigned int AudioPath::getHash() const {
    return ((m_type & 0xFF) << 24)
        | ((m_index & 0xFF) << 16)
        | (m_channelGroup.getHash() << 8);
}

bool AudioPath::channelsClash(const AudioPath& other) const {
    return m_channelGroup.clashesWith(other.m_channelGroup);
}

/**
 * Gives a string describing the AudioPath for user benefit.
 * @returns A QString. Ideally will use tr() for i18n but the rest of mixxx
 *          doesn't at the moment so worry about that later. :)
 */
QString AudioPath::getString() const {
    if (isIndexable(getType())) {
        return QString("%1 %2")
            .arg(getStringFromType(getType())).arg(m_index + 1);
    }
    return getStringFromType(getType());
}

//static
QString AudioPath::getStringFromType(AudioPathType type) {
    switch (type) {
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
AudioPath::AudioPathType getTypeFromString(QString string) {
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
    }
}

//static
bool AudioPath::isIndexable(AudioPathType type) {
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
AudioPath::AudioPathType AudioPath::getTypeFromInt(int typeInt) {
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
    }
    // gcc will warn us here if we missed a type.
    // if you're reading this and added a new type, check anyway -- bkgood
}

//static
unsigned char AudioPath::channelsNeededForType(AudioPath::AudioPathType type)
{
    switch (type) {
    case AudioPath::MICROPHONE:
        return 1;
    default:
        return 2;
    }
}

AudioSource::AudioSource(AudioPath::AudioPathType type,
        unsigned char channelBase,
        unsigned char index /* = 0 */)
    : AudioPath(channelBase, AudioPath::channelsNeededForType(type)) {
    if (getSupportedTypes().contains(type)) {
        setType(type);
    }
    if (isIndexable(type)) {
        m_index = index;
    } else {
        index = 0;
    }
}

//static
QList<AudioPath::AudioPathType> AudioSource::getSupportedTypes() {
    QList<AudioPath::AudioPathType> types;
    types.append(MASTER);
    types.append(HEADPHONES);
    types.append(DECK);
    return types;
}

AudioReceiver::AudioReceiver(AudioPath::AudioPathType type,
        unsigned char channelBase,
        unsigned char index /* = 0 */)
  : AudioPath(channelBase, AudioPath::channelsNeededForType(type)) {
    if (getSupportedTypes().contains(type)) {
        setType(type);
    }
    if (isIndexable(type)) {
        m_index = index;
    } else {
        index = 0;
    }
}

//static
QList<AudioPath::AudioPathType> AudioReceiver::getSupportedTypes() {
    QList<AudioPath::AudioPathType> types;
    types.append(VINYLCONTROL);
    return types;
}


unsigned int qHash(const AudioSource &src) {
    return src.getHash();
}

unsigned int qHash(const AudioReceiver &recv) {
    return recv.getHash();
}
