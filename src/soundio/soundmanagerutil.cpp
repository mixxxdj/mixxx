#include "soundio/soundmanagerutil.h"

#include "engine/channels/enginechannel.h"

/// Constructs a ChannelGroup.
/// @param channelBase the first channel in the group.
/// @param channels the number of channels.
ChannelGroup::ChannelGroup(unsigned char channelBase, mixxx::audio::ChannelCount channels)
        : m_channelBase(channelBase), m_channels(channels) {
}

/// @return This ChannelGroup's base channel
unsigned char ChannelGroup::getChannelBase() const {
    return m_channelBase;
}

/// @return The number of channels in this ChannelGroup
mixxx::audio::ChannelCount ChannelGroup::getChannelCount() const {
    return m_channels;
}

/// Checks if another ChannelGroup shares channels with this one.
/// @param other the other ChannelGroup to check for a clash with.
/// @return true if the other and this ChannelGroup share any channels,
///          false otherwise.
bool ChannelGroup::clashesWith(const ChannelGroup &other) const {
    if (!m_channels.isValid() || !other.m_channels.isValid()) {
        return false; // can't clash if there are no channels in use
    }
    return (m_channelBase > other.m_channelBase
        && m_channelBase < other.m_channelBase + other.m_channels)
        ||
        (other.m_channelBase > m_channelBase
        && other.m_channelBase < m_channelBase + m_channels)
        || m_channelBase == other.m_channelBase;
}

/// Constructs an AudioPath object (must be called by a child class's
/// constructor, AudioPath is abstract).
/// @param channelBase the first channel on a sound device used by this AudioPath.
/// @param channels the number of channels used.
AudioPath::AudioPath(unsigned char channelBase, mixxx::audio::ChannelCount channels)
        : m_channelGroup(channelBase, channels),
          m_type(AudioPathType::Invalid),
          m_index(0) {
}

/// @return This AudioPath's type
AudioPathType AudioPath::getType() const {
    return m_type;
}

/// @return This AudioPath's ChannelGroup instance.
ChannelGroup AudioPath::getChannelGroup() const {
    return m_channelGroup;
}

/// @return This AudioPath's index, or 0 if this AudioPath isn't indexable.
unsigned char AudioPath::getIndex() const {
    return m_index;
}

/// Checks if this AudioPath's channels clash with another's
/// (see ChannelGroup::clashesWith).
bool AudioPath::channelsClash(const AudioPath &other) const {
    return m_channelGroup.clashesWith(other.m_channelGroup);
}

/// Returns a string describing the AudioPath for user benefit.
QString AudioPath::getString() const {
    return getTrStringFromType(m_type, m_index);
}

/// Returns a string given an AudioPathType.
/// @note This method is static.
/// @note For user-facing usage, see getTrStringFromType
QString AudioPath::getStringFromType(AudioPathType type) {
    switch (type) {
    case AudioPathType::Main:
        // This was renamed to "Main" in the GUI, but keep "Master" here to avoid
        // making users reconfigure the output when upgrading.
        // https://mixxx.org/news/2020-06-29-black-lives-matter/
        return QStringLiteral("Master");
    case AudioPathType::Headphones:
        return QStringLiteral("Headphones");
    case AudioPathType::Booth:
        return QStringLiteral("Booth");
    case AudioPathType::Bus:
        return QStringLiteral("Bus");
    case AudioPathType::Deck:
        return QStringLiteral("Deck");
    case AudioPathType::VinylControl:
        return QStringLiteral("Vinyl Control");
    case AudioPathType::Microphone:
        return QStringLiteral("Microphone");
    case AudioPathType::Auxiliary:
        return QStringLiteral("Auxiliary");
    case AudioPathType::RecordBroadcast:
        return QStringLiteral("Record/Broadcast");
    case AudioPathType::Invalid:
        // this shouldn't happen but g++ complains if I don't
        // handle this -- bkgood
        return QStringLiteral("Invalid");
    }
    return QObject::tr("Unknown path type %1").arg(static_cast<int>(type));
}

/// Returns a translated string given an AudioPathType.
/// @note This method is static.
QString AudioPath::getTrStringFromType(AudioPathType type, unsigned char index) {
    switch (type) {
    case AudioPathType::Invalid:
        // this shouldn't happen but g++ complains if I don't
        // handle this -- bkgood
        return QObject::tr("Invalid");
    case AudioPathType::Main:
        //: Audio path indetifier
        return QObject::tr("Main");
    case AudioPathType::Headphones:
        //: Audio path indetifier
        return QObject::tr("Headphones");
    case AudioPathType::Booth:
        //: Audio path indetifier
        return QObject::tr("Booth");
    case AudioPathType::Bus:
        switch (index) {
        case EngineChannel::LEFT:
            //: Audio path indetifier
            return QObject::tr("Left Bus");
        case EngineChannel::CENTER:
            //: Audio path indetifier
            return QObject::tr("Center Bus");
        case EngineChannel::RIGHT:
            //: Audio path indetifier
            return QObject::tr("Right Bus");
        default:
            //: Audio path indetifier
            return QObject::tr("Invalid Bus");
        }
    case AudioPathType::Deck:
        //: Audio path indetifier
        return QString("%1 %2").arg(QObject::tr("Deck"),
                                    QString::number(index + 1));
    case AudioPathType::VinylControl:
        //: Audio path indetifier
        return QString("%1 %2").arg(QObject::tr("Vinyl Control"),
                                    QString::number(index + 1));
    case AudioPathType::Microphone:
        //: Audio path indetifier
        return QString("%1 %2").arg(QObject::tr("Microphone"),
                                    QString::number(index + 1));
    case AudioPathType::Auxiliary:
        //: Audio path indetifier
        return QString("%1 %2").arg(QObject::tr("Auxiliary"),
                                    QString::number(index + 1));
    case AudioPathType::RecordBroadcast:
        //: Audio path indetifier
        return QObject::tr("Record/Broadcast");
    }
    //: Audio path
    return QObject::tr("Unknown path type %1").arg(static_cast<int>(type));
}

/// Returns an AudioPathType given a string.
/// @note This method is static.
AudioPathType AudioPath::getTypeFromString(QString string) {
    string = string.toLower();
    if (string == AudioPath::getStringFromType(AudioPathType::Main).toLower()) {
        return AudioPathType::Main;
    } else if (string == AudioPath::getStringFromType(AudioPathType::Booth).toLower()) {
        return AudioPathType::Booth;
    } else if (string == AudioPath::getStringFromType(AudioPathType::Headphones).toLower()) {
        return AudioPathType::Headphones;
    } else if (string == AudioPath::getStringFromType(AudioPathType::Bus).toLower()) {
        return AudioPathType::Bus;
    } else if (string == AudioPath::getStringFromType(AudioPathType::Deck).toLower()) {
        return AudioPathType::Deck;
    } else if (string == AudioPath::getStringFromType(AudioPathType::VinylControl).toLower()) {
        return AudioPathType::VinylControl;
    } else if (string == AudioPath::getStringFromType(AudioPathType::Microphone).toLower()) {
        return AudioPathType::Microphone;
    } else if (string == AudioPath::getStringFromType(AudioPathType::Auxiliary).toLower()) {
        return AudioPathType::Auxiliary;
    } else if (string == AudioPath::getStringFromType(AudioPathType::RecordBroadcast).toLower()) {
        return AudioPathType::RecordBroadcast;
    } else {
        return AudioPathType::Invalid;
    }
}

/// Defines whether or not an AudioPathType can be indexed.
/// @note This method is static.
bool AudioPath::isIndexed(AudioPathType type) {
    switch (type) {
    case AudioPathType::Bus:
    case AudioPathType::Deck:
    case AudioPathType::VinylControl:
    case AudioPathType::Microphone:
    case AudioPathType::Auxiliary:
        return true;
    default:
        break;
    }
    return false;
}

/// Returns an AudioPathType given an int.
/// @note This method is static.
AudioPathType AudioPath::getTypeFromInt(int typeInt) {
    if (typeInt < 0 || typeInt >= static_cast<int>(AudioPathType::Invalid)) {
        return AudioPathType::Invalid;
    }
    return static_cast<AudioPathType>(typeInt);
}

// static
mixxx::audio::ChannelCount AudioPath::minChannelsForType(AudioPathType type) {
    switch (type) {
    case AudioPathType::VinylControl:
        return mixxx::audio::ChannelCount::stereo();
    default:
        return mixxx::audio::ChannelCount::mono();
    }
}

// static
mixxx::audio::ChannelCount AudioPath::maxChannelsForType(AudioPathType type) {
    Q_UNUSED(type);
    return mixxx::audio::ChannelCount::stereo();
}

/// Constructs an AudioOutput.
AudioOutput::AudioOutput(AudioPathType type,
        unsigned char channelBase,
        mixxx::audio::ChannelCount channels,
        unsigned char index)
        : AudioPath(channelBase, channels) {
    // TODO(rryan): This is a virtual function call from a constructor.
    setType(type);
    if (isIndexed(type)) {
        m_index = index;
    } else {
        m_index = 0;
    }
}

/// Writes this AudioOutput's data to an XML element, preallocated from an XML
/// DOM document.
QDomElement AudioOutput::toXML(QDomElement *element) const {
    element->setTagName("output");
    element->setAttribute("type", AudioPath::getStringFromType(m_type));
    element->setAttribute("index", m_index);
    element->setAttribute("channel", m_channelGroup.getChannelBase());
    element->setAttribute("channel_count", m_channelGroup.getChannelCount().value());
    return *element;
}

/// Constructs and returns an AudioOutput given an XML element representing it.
/// @note This method is static.
AudioOutput AudioOutput::fromXML(const QDomElement &xml) {
    AudioPathType type(AudioPath::getTypeFromString(xml.attribute("type")));
    unsigned int index(xml.attribute("index", "0").toUInt());
    unsigned int channel(xml.attribute("channel", "0").toUInt());
    auto channels = mixxx::audio::ChannelCount::fromInt(
            xml.attribute("channel_count", "0").toInt());
    // In Mixxx < 1.12.0 we didn't save channels to file since they directly
    // corresponded to the type. To migrate users over, use mono for all
    // microphones and stereo for everything else since previously microphone
    // inputs were the only mono AudioPath.
    if (!channels.isValid()) {
        channels = mixxx::audio::ChannelCount::stereo();
    }
    return AudioOutput(type, channel, channels, index);
}

// static
/// Enumerates the AudioPathTypes supported by AudioOutput.
/// @note This method is static.
QList<AudioPathType> AudioOutput::getSupportedTypes() {
    return QList<AudioPathType>{
            AudioPathType::Main,
            AudioPathType::Booth,
            AudioPathType::Headphones,
            AudioPathType::Bus,
            AudioPathType::Deck,
            AudioPathType::RecordBroadcast,
    };
}

/// Implements setting the type of an AudioOutput, using
/// AudioOutput::getSupportedTypes.
void AudioOutput::setType(AudioPathType type) {
    if (AudioOutput::getSupportedTypes().contains(type)) {
        m_type = type;
    } else {
        m_type = AudioPathType::Invalid;
    }
}

/// Constructs an AudioInput.
AudioInput::AudioInput(AudioPathType type,
        unsigned char channelBase,
        mixxx::audio::ChannelCount channels,
        unsigned char index)
        : AudioPath(channelBase, channels) {
    // TODO(rryan): This is a virtual function call from a constructor.
    setType(type);
    if (isIndexed(type)) {
        m_index = index;
    } else {
        m_index = 0;
    }
}

AudioInput::~AudioInput() {

}

/// Writes this AudioInput's data to an XML element, preallocated from an XML
/// DOM document.
QDomElement AudioInput::toXML(QDomElement *element) const {
    element->setTagName("input");
    element->setAttribute("type", AudioPath::getStringFromType(m_type));
    element->setAttribute("index", m_index);
    element->setAttribute("channel", m_channelGroup.getChannelBase());
    element->setAttribute("channel_count", m_channelGroup.getChannelCount().value());
    return *element;
}

/// Constructs and returns an AudioInput given an XML element representing it.
/// @note This method is static.
AudioInput AudioInput::fromXML(const QDomElement &xml) {
    AudioPathType type(AudioPath::getTypeFromString(xml.attribute("type")));
    unsigned int index(xml.attribute("index", "0").toUInt());
    unsigned int channel(xml.attribute("channel", "0").toUInt());
    auto channels = mixxx::audio::ChannelCount::fromInt(
            xml.attribute("channel_count", "0").toInt());
    // In Mixxx <1.12.0 we didn't save channels to file since they directly
    // corresponded to the type. To migrate users over, use mono for all
    // microphones and stereo for everything else since previously microphone
    // inputs were the only mono AudioPath.
    if (!channels.isValid()) {
        channels = type == AudioPathType::Microphone ? mixxx::audio::ChannelCount::mono()
                                                     : mixxx::audio::ChannelCount::stereo();
    }
    return AudioInput(type, channel, channels, index);
}

/// Enumerates the AudioPathTypes supported by AudioInput.
/// @note This method is static.
QList<AudioPathType> AudioInput::getSupportedTypes() {
    return QList<AudioPathType>{
#ifdef __VINYLCONTROL__
            // this disables vinyl control for all of the sound devices stuff
            // (prefs, etc), minimal ifdefs :) -- bkgood
            AudioPathType::VinylControl,
#endif
            AudioPathType::Auxiliary,
            AudioPathType::Microphone,
            AudioPathType::RecordBroadcast,
    };
}

/// Implements setting the type of an AudioInput, using
/// AudioInput::getSupportedTypes.
void AudioInput::setType(AudioPathType type) {
    if (AudioInput::getSupportedTypes().contains(type)) {
        m_type = type;
    } else {
        m_type = AudioPathType::Invalid;
    }
}

QString SoundDeviceId::debugName() const {
    if (alsaHwDevice.isEmpty()) {
        return name + QStringLiteral(", ") + QString::number(portAudioIndex);
    } else {
        return name + QStringLiteral(", ") + alsaHwDevice + QStringLiteral(", ") + QString::number(portAudioIndex);
    }
}
