/**
 * @file soundmanagerutil.cpp
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
#include "soundmanagerutil.h"

/**
 * Constructs a ChannelGroup.
 * @param channelBase the first channel in the group.
 * @param channels the number of channels.
 */
ChannelGroup::ChannelGroup(unsigned char channelBase, unsigned char channels)
  : m_channelBase(channelBase)
  , m_channels(channels) {
}

/**
 * @return This ChannelGroup's base channel
 */
unsigned char ChannelGroup::getChannelBase() const {
    return m_channelBase;
}

/**
 * @return The number of channels in this ChannelGroup
 */
unsigned char ChannelGroup::getChannelCount() const {
    return m_channels;
}

/**
 * Defines equality between two ChannelGroups.
 * @return true if the two ChannelGroups share a common base channel
 *          and channel count, otherwise false.
 */
bool ChannelGroup::operator==(const ChannelGroup &other) const {
    return m_channelBase == other.m_channelBase
        && m_channels == other.m_channels;
}

/**
 * Checks if another ChannelGroup shares channels with this one.
 * @param other the other ChannelGroup to check for a clash with.
 * @return true if the other and this ChannelGroup share any channels,
 *          false otherwise.
 */
bool ChannelGroup::clashesWith(const ChannelGroup &other) const {
    if (m_channels == 0 || other.m_channels == 0) {
        return false; // can't clash if there are no channels in use
    }
    return (m_channelBase > other.m_channelBase
        && m_channelBase < other.m_channelBase + other.m_channels)
        ||
        (other.m_channelBase > m_channelBase
        && other.m_channelBase < m_channelBase + m_channels)
        || m_channelBase == other.m_channelBase;
}

/**
 * Generates a hash of this ChannelGroup, so it can act as a key in a QHash.
 * @return a hash for this ChannelGroup
 */
unsigned int ChannelGroup::getHash() const {
    return 0 | (m_channels << 8) | m_channelBase;
}

/**
 * Constructs an AudioPath object (must be called by a child class's
 * constructor, AudioPath is abstract).
 * @param channelBase the first channel on a sound device used by this AudioPath.
 * @param channels the number of channels used.
 */
AudioPath::AudioPath(unsigned char channelBase, unsigned char channels)
    : m_channelGroup(channelBase, channels) {
}

/**
 * @return This AudioPath's type
 */
AudioPathType AudioPath::getType() const {
    return m_type;
}

/**
 * @return This AudioPath's ChannelGroup instance.
 */
ChannelGroup AudioPath::getChannelGroup() const {
    return m_channelGroup;
}

/**
 * @return This AudioPath's index, or 0 if this AudioPath isn't indexable.
 */
unsigned char AudioPath::getIndex() const {
    return m_index;
}

/**
 * Defines equality for AudioPath objects.
 * @return true of this and other share a common type and index.
 */
bool AudioPath::operator==(const AudioPath &other) const {
    return m_type == other.m_type
        && m_index == other.m_index;
}

/**
 * Generates a hash of this AudioPath, so it can act as a key in a QHash.
 * @return a hash for this AudioPath
 */
unsigned int AudioPath::getHash() const {
    return 0 | (m_type << 8) | m_index;
}

/**
 * Checks if this AudioPath's channels clash with another's
 * (see ChannelGroup::clashesWith).
 */
bool AudioPath::channelsClash(const AudioPath &other) const {
    return m_channelGroup.clashesWith(other.m_channelGroup);
}

/**
 * Returns a string describing the AudioPath for user benefit.
 */
QString AudioPath::getString() const {
    if (isIndexed(getType())) {
        return QString("%1 %2").arg(getTrStringFromType(getType()),
                                    QString::number(m_index + 1));
    }
    return getTrStringFromType(getType());
}

/**
 * Returns a string given an AudioPathType.
 * @note This method is static.
 * @note For user-facing usage, see getTrStringFromType
 */
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
    case EXTPASSTHROUGH:
        return QString::fromAscii("Passthrough");
    }
    return QString::fromAscii("Unknown path type %1").arg(type);
}

/**
 * Returns a translated string given an AudioPathType.
 * @note This method is static.
 */
QString AudioPath::getTrStringFromType(AudioPathType type) {
    switch (type) {
    case INVALID:
        // this shouldn't happen but g++ complains if I don't
        // handle this -- bkgood
        return QString(QObject::tr("Invalid"));
    case MASTER:
        return QString(QObject::tr("Master"));
    case HEADPHONES:
        return QString(QObject::tr("Headphones"));
    case DECK:
        return QString(QObject::tr("Deck"));
    case VINYLCONTROL:
        return QString(QObject::tr("Vinyl Control"));
    case MICROPHONE:
        return QString(QObject::tr("Microphone"));
    case EXTPASSTHROUGH:
        return QString(QObject::tr("Passthrough"));
    }
    return QString(QObject::tr("Unknown path type %1")).arg(type);
}

/**
 * Returns an AudioPathType given a string.
 * @note This method is static.
 */
AudioPathType AudioPath::getTypeFromString(QString string) {
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
    } else if (string == AudioPath::getStringFromType(AudioPath::EXTPASSTHROUGH).toLower()) {
        return AudioPath::EXTPASSTHROUGH;
    } else {
        return AudioPath::INVALID;
    }
}

/**
 * Defines whether or not an AudioPathType can be indexed.
 * @note This method is static.
 */
bool AudioPath::isIndexed(AudioPathType type) {
    switch (type) {
    case DECK:
    case VINYLCONTROL:
    case EXTPASSTHROUGH:
        return true;
    case MICROPHONE:
    default:
        break;
    }
    return false;
}

/**
 * Returns an AudioPathType given an int.
 * @note This method is static.
 */
AudioPathType AudioPath::getTypeFromInt(int typeInt) {
    if (typeInt < 0 || typeInt >= AudioPath::INVALID) {
        return AudioPath::INVALID;
    }
    return static_cast<AudioPathType>(typeInt);
}

/**
 * Returns the number of channels needed on a sound device for an
 * AudioPathType.
 * @note This method is static.
 */
unsigned char AudioPath::channelsNeededForType(AudioPathType type) {
    switch (type) {
    case AudioPath::MICROPHONE:
        return 1;
    default:
        return 2;
    }
}

/**
 * Constructs an AudioOutput.
 */
AudioOutput::AudioOutput(AudioPathType type /* = INVALID */,
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

/**
 * Writes this AudioOutput's data to an XML element, preallocated from an XML
 * DOM document.
 */
QDomElement AudioOutput::toXML(QDomElement *element) const {
    element->setTagName("output");
    element->setAttribute("type", AudioPath::getStringFromType(m_type));
    element->setAttribute("index", m_index);
    element->setAttribute("channel", m_channelGroup.getChannelBase());
    return *element;
}

/**
 * Constructs and returns an AudioOutput given an XML element representing it.
 * @note This method is static.
 */
AudioOutput AudioOutput::fromXML(const QDomElement &xml) {
    AudioPathType type(AudioPath::getTypeFromString(xml.attribute("type")));
    unsigned int index(xml.attribute("index", "0").toUInt());
    unsigned int channel(xml.attribute("channel", "0").toUInt());
    return AudioOutput(type, channel, index);
}

//static
/**
 * Enumerates the AudioPathTypes supported by AudioOutput.
 * @note This method is static.
 */
QList<AudioPathType> AudioOutput::getSupportedTypes() {
    QList<AudioPathType> types;
    types.append(MASTER);
    types.append(HEADPHONES);
    types.append(DECK);
    return types;
}

/**
 * Implements setting the type of an AudioOutput, using
 * AudioOutput::getSupportedTypes.
 */
void AudioOutput::setType(AudioPathType type) {
    if (AudioOutput::getSupportedTypes().contains(type)) {
        m_type = type;
    } else {
        m_type = AudioPath::INVALID;
    }
}

/**
 * Constructs an AudioInput.
 */
AudioInput::AudioInput(AudioPathType type /* = INVALID */,
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

/**
 * Writes this AudioInput's data to an XML element, preallocated from an XML
 * DOM document.
 */
QDomElement AudioInput::toXML(QDomElement *element) const {
    element->setTagName("input");
    element->setAttribute("type", AudioPath::getStringFromType(m_type));
    element->setAttribute("index", m_index);
    element->setAttribute("channel", m_channelGroup.getChannelBase());
    return *element;
}

/**
 * Constructs and returns an AudioInput given an XML element representing it.
 * @note This method is static.
 */
AudioInput AudioInput::fromXML(const QDomElement &xml) {
    AudioPathType type(AudioPath::getTypeFromString(xml.attribute("type")));
    unsigned int index(xml.attribute("index", "0").toUInt());
    unsigned int channel(xml.attribute("channel", "0").toUInt());
    return AudioInput(type, channel, index);
}

/**
 * Enumerates the AudioPathTypes supported by AudioInput.
 * @note This method is static.
 */
QList<AudioPathType> AudioInput::getSupportedTypes() {
    QList<AudioPathType> types;
#ifdef __VINYLCONTROL__
    // this disables vinyl control for all of the sound devices stuff
    // (prefs, etc), minimal ifdefs :) -- bkgood
    types.append(VINYLCONTROL);
#endif
    types.append(MICROPHONE);
    return types;
}

/**
 * Implements setting the type of an AudioInput, using
 * AudioInput::getSupportedTypes.
 */
void AudioInput::setType(AudioPathType type) {
    if (AudioInput::getSupportedTypes().contains(type)) {
        m_type = type;
    } else {
        m_type = AudioPath::INVALID;
    }
}

/**
 * Defined for QHash, so ChannelGroup can be used as a QHash key.
 */
unsigned int qHash(const ChannelGroup &group) {
    return group.getHash();
}

/**
 * Defined for QHash, so AudioOutput can be used as a QHash key.
 */
unsigned int qHash(const AudioOutput &output) {
    return output.getHash();
}

/**
 * Defined for QHash, so AudioInput can be used as a QHash key.
 */
unsigned int qHash(const AudioInput &input) {
    return input.getHash();
}
