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

ChannelGroup::ChannelGroup(unsigned int channelBase, unsigned int channels)
  : m_channelBase(channelBase)
  , m_channels(channels) {
}

unsigned int ChannelGroup::getChannelBase() const {
    return m_channelBase;
}

unsigned int ChannelGroup::getChannelCount() const {
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

AudioPath::AudioPath(unsigned int channelBase, unsigned int channels)
    : m_channelGroup(channelBase, channels) {
}

AudioPath::AudioPathType AudioPath::getType() const {
    return m_type;
}

ChannelGroup AudioPath::getChannelGroup() const {
    return m_channelGroup;
}

unsigned int AudioPath::getIndex() const {
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
    if (isIndexable(m_type)) {
        return QString("%1 %2").arg(getStringFromType(m_type)).arg(m_index + 1);
    }
    return getStringFromType(m_type);
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

AudioSource::AudioSource(AudioPath::AudioPathType type, unsigned int channelBase,
            unsigned int channels, unsigned int index /* = 0 */)
    : AudioPath(channelBase, channels) {
    if (getSupportedTypes().contains(type)) {
        m_type = type;
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

AudioReceiver::AudioReceiver(AudioPath::AudioPathType type, unsigned int channelBase,
            unsigned int channels, unsigned int index /* = 0 */)
  : AudioPath(channelBase, channels) {
    if (getSupportedTypes().contains(type)) {
        m_type = type;
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
