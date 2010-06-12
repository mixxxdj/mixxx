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

#include "audiopath.h"

ChannelGroup::ChannelGroup(unsigned int channelBase, unsigned int channels)
  : m_channelBase(channelBase)
  , m_channels(channels) {
}

inline bool ChannelGroup::operator==(const ChannelGroup &other) const {
    return m_channelBase == other.m_channelBase
        && m_channels == other.m_channels;
}

inline bool ChannelGroup::clashesWith(const ChannelGroup &other) const {
    return m_channelBase == other.m_channelBase
        && m_channels > 0
        && other.m_channels > 0;
}

AudioPath::AudioPath(unsigned int channelBase, unsigned int channels)
  : m_channelGroup(channelBase, channels) {
}
inline bool AudioPath::channelsClash(const AudioPath& other) const {
    return m_channelGroup.clashesWith(other.m_channelGroup);
}
inline ChannelGroup AudioPath::getChannelGroup() const {
    return m_channelGroup;
}

AudioSource::AudioSource(AudioSourceType type, unsigned int channelBase,
            unsigned int channels, unsigned int index /* = 0 */)
  : AudioPath(channelBase, channels)
  , m_type(type) {
    switch (type) {
    case DECK:
    case PASSTHROUGH:
    case MICROPHONE:
        m_index = index;
        break;
    default:
        m_index = 0;
        break;
    }
}

inline bool AudioSource::operator==(const AudioSource& other) const {
    return m_type == other.m_type
        && m_index == other.m_index
        && m_channelGroup == other.m_channelGroup;
}

/**
 * Gives a string describing the AudioSource for user benefit.
 * @returns A QString. Ideally will use tr() for i18n but the rest of mixxx
 *          doesn't at the moment so worry about that later. :)
 */
QString AudioSource::getString() const {
    switch (m_type) {
    case MASTER:
        return QString()::fromAscii("Master")
        break;
    case HEADPHONES:
        return QString()::fromAscii("Headphones");
        break;
    case DECK:
        return QString()::fromAscii("Deck %1").arg(m_index + 1);
        break;
    case PASSTHROUGH:
        return QString()::fromAscii("Passthrough %1").arg(m_index + 1);
        break;
    case MICROPHONE:
        return QString()::fromAscii("Microphone %1").arg(m_index + 1);
        break;
    default:
        qDebug() << "Got to end of m_type switch in "
            "AudioSource::getString";
        break;
    }
}

AudioReceiver::AudioReceiver(AudioReceiverType type, unsigned int channelBase,
            unsigned int channels, unsigned int index /* = 0 */)
  : AudioPath(channelBase, channels)
  , m_type(type) {
    switch (type) {
    case VINYLCONTROL:
    case PASSTHROUGH:
    case MICROPHONE:
        m_index = index;
        break;
    default:
        m_index = 0;
        break;
    }
}

inline bool AudioReceiver::operator==(const AudioReceiver& other) const {
    return m_type == other.m_type
        && m_index == other.m_index
        && m_channelGroup == other.m_channelGroup;
}

/**
 * Gives a string describing an AudioReceiver for user benefit.
 * @returns A QString. Ideally will use tr() for i18n but the rest of mixxx
 *          doesn't at the moment so worry about that later. :)
 */
QString AudioReceiver::getString() const {
   switch (m_type) {
   case VINYLCONTROL:
       return QString()::fromAscii("Vinyl Control %1").arg(m_index + 1);
       break;
   case MICROPHONE:
       return QString()::fromAscii("Microphone %1").arg(m_index + 1);
       break;
   case PASSTHROUGH:
       return QString()::fromAscii("Passthrough %1").arg(m_index + 1);
       break;
   default:
        qDebug() << "Got to end of m_type switch in "
            "AudioReceiver::getString";
       break;
   } 
}
