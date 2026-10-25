#include "soundio/usbaudiodescriptors.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace mixxx {
namespace usbaudio {

namespace {

constexpr uint8_t kDescriptorInterface = 0x04;
constexpr uint8_t kDescriptorEndpoint = 0x05;
constexpr uint8_t kCsInterface = 0x24;
constexpr uint8_t kCsEndpoint = 0x25;

constexpr uint8_t kClassAudio = 0x01;
constexpr uint8_t kSubclassAudioControl = 0x01;
constexpr uint8_t kSubclassAudioStreaming = 0x02;
constexpr uint8_t kSubclassMidiStreaming = 0x03;
constexpr uint8_t kProtocolUac2 = 0x20;

// Audio control interface descriptor subtypes (UAC2)
constexpr uint8_t kAcInputTerminal = 0x02;
constexpr uint8_t kAcOutputTerminal = 0x03;
constexpr uint8_t kAcClockSource = 0x0A;
constexpr uint8_t kAcClockSelector = 0x0B;
constexpr uint8_t kAcClockMultiplier = 0x0C;

// Audio streaming interface descriptor subtypes
constexpr uint8_t kAsGeneral = 0x01;
constexpr uint8_t kAsFormatType = 0x02;
constexpr uint8_t kFormatTypeI = 0x01;
constexpr uint16_t kFormatTagPcm = 0x0001;
constexpr uint8_t kEpGeneral = 0x01;

constexpr uint8_t kTransferTypeIsochronous = 0x01;
constexpr uint8_t kTransferTypeBulk = 0x02;
constexpr int kUsageData = 0;
constexpr int kUsageFeedback = 1;

int rate24(const uint8_t* p) {
    return p[0] | (p[1] << 8) | (p[2] << 16);
}

} // namespace

bool StreamFormat::supportsRate(int rate) const {
    if (std::find(sampleRates.begin(), sampleRates.end(), rate) != sampleRates.end()) {
        return true;
    }
    return minRate > 0 && rate >= minRate && rate <= maxRate;
}

int ControlInterface::clockSourceForTerminal(int terminalId) const {
    auto terminal = terminalClocks.find(terminalId);
    if (terminal == terminalClocks.end()) {
        return -1;
    }
    int clockId = terminal->second;
    // Follow selectors (first input) and multipliers, guarding against loops
    for (int depth = 0; depth < 8; ++depth) {
        auto clock = clocks.find(clockId);
        if (clock == clocks.end()) {
            // Unknown entity, assume it is the source
            return clockId;
        }
        if (clock->second.type == ClockEntity::Type::Source || clock->second.sources.empty()) {
            return clockId;
        }
        clockId = clock->second.sources.front();
    }
    return -1;
}

const ControlInterface* DeviceDescription::controlFor(const StreamFormat& format) const {
    const ControlInterface* pResult = nullptr;
    for (const auto& control : controls) {
        if (control.interfaceNumber < format.interfaceNumber &&
                (!pResult || control.interfaceNumber > pResult->interfaceNumber)) {
            pResult = &control;
        }
    }
    return pResult;
}

DeviceDescription parse(const uint8_t* pData, size_t length) {
    DeviceDescription description;
    int currentStream = -1;
    int currentControl = -1;
    bool inMidi = false;
    bool lastEndpointIsData = false;

    size_t pos = 0;
    while (pos + 2 <= length) {
        const uint8_t* d = pData + pos;
        const size_t descriptorLength = d[0];
        if (descriptorLength < 2 || pos + descriptorLength > length) {
            break;
        }
        const uint8_t type = d[1];

        if (type == kDescriptorInterface) {
            if (descriptorLength < 9) {
                break;
            }
            const uint8_t interfaceClass = d[5];
            const uint8_t interfaceSubClass = d[6];
            const uint8_t interfaceProtocol = d[7];
            const int interfaceNumber = d[2];

            if (interfaceClass == kClassAudio && interfaceSubClass == kSubclassAudioControl) {
                inMidi = false;
                currentControl = description.controls.size();
                description.controls.push_back(ControlInterface{});
                description.controls.back().interfaceNumber = interfaceNumber;
                description.controls.back().uacVersion =
                        (interfaceProtocol == kProtocolUac2) ? 2 : 1;
            } else if (interfaceClass == kClassAudio &&
                    interfaceSubClass == kSubclassAudioStreaming) {
                inMidi = false;
                currentStream = description.streams.size();
                description.streams.push_back(StreamFormat{});
                description.streams.back().interfaceNumber = interfaceNumber;
                description.streams.back().uacVersion =
                        (interfaceProtocol == kProtocolUac2) ? 2 : 1;
            } else if (interfaceClass == kClassAudio &&
                    interfaceSubClass == kSubclassMidiStreaming) {
                inMidi = true;
                description.midi.interfaceNumber = interfaceNumber;
            } else {
                inMidi = false;
                currentStream = -1;
                currentControl = -1;
            }
        } else if (type == kCsInterface && descriptorLength >= 4 &&
                currentControl >= 0 && !inMidi) {
            ControlInterface& control = description.controls[currentControl];
            const uint8_t subtype = d[2];
            if (control.uacVersion == 2) {
                if (subtype == kAcInputTerminal || subtype == kAcOutputTerminal) {
                    if (descriptorLength >= 8) {
                        const int terminalId = d[3];
                        const int clockId = d[7];
                        control.terminalClocks[terminalId] = clockId;
                    }
                } else if (subtype == kAcClockSource) {
                    if (descriptorLength >= 8) {
                        const int clockId = d[3];
                        control.clocks[clockId] = ClockEntity{ClockEntity::Type::Source};
                    }
                } else if (subtype == kAcClockSelector) {
                    if (descriptorLength >= 6) {
                        const int clockId = d[3];
                        const int numSources = d[4];
                        ClockEntity entity{ClockEntity::Type::Selector};
                        for (int i = 0; i < numSources &&
                                5 + i < static_cast<int>(descriptorLength);
                                ++i) {
                            entity.sources.push_back(d[5 + i]);
                        }
                        control.clocks[clockId] = entity;
                    }
                } else if (subtype == kAcClockMultiplier) {
                    if (descriptorLength >= 6) {
                        const int clockId = d[3];
                        const int numSources = d[4];
                        ClockEntity entity{ClockEntity::Type::Multiplier};
                        for (int i = 0; i < numSources &&
                                5 + i < static_cast<int>(descriptorLength);
                                ++i) {
                            entity.sources.push_back(d[5 + i]);
                        }
                        control.clocks[clockId] = entity;
                    }
                }
            }
        } else if (type == kCsInterface && descriptorLength >= 4 &&
                currentStream >= 0 && !inMidi) {
            StreamFormat& format = description.streams[currentStream];
            const uint8_t subtype = d[2];
            if (subtype == kAsGeneral) {
                if (format.uacVersion == 1) {
                    if (descriptorLength >= 7) {
                        format.terminalLink = d[4] | (d[5] << 8);
                        format.bitResolution = d[6];
                    }
                } else {
                    if (descriptorLength >= 6) {
                        format.terminalLink = d[3] | (d[4] << 8);
                        format.bitResolution = d[5];
                    }
                }
            } else if (subtype == kAsFormatType) {
                if (descriptorLength >= 6) {
                    const uint8_t formatType = d[3];
                    if (formatType == kFormatTypeI) {
                        format.isPcm = true;
                        format.channels = d[4];
                        format.subframeBytes = d[5];
                        format.bitResolution = d[6] > 0 ? d[6] : format.subframeBytes * 8;
                    }
                }
            }
        } else if (type == kDescriptorEndpoint && descriptorLength >= 7 &&
                currentStream >= 0 && !inMidi) {
            StreamFormat& format = description.streams[currentStream];
            const uint8_t endpoint = d[2];
            const uint8_t attributes = d[3];
            const uint16_t maxPacketSize = d[4] | (d[5] << 8);
            const uint8_t bInterval = d[6];

            format.endpoint = endpoint;
            format.maxPacketSize = maxPacketSize;
            format.bInterval = bInterval;
            format.isOutput = (endpoint & 0x80) == 0;

            const uint8_t syncType = (attributes >> 2) & 0x03;
            if (syncType == 0)
                format.syncType = SyncType::Synchronous;
            else if (syncType == 1)
                format.syncType = SyncType::Asynchronous;
            else if (syncType == 2)
                format.syncType = SyncType::Adaptive;
            else
                format.syncType = SyncType::None;

            lastEndpointIsData = (attributes & kTransferTypeIsochronous) != 0;
        } else if (type == kCsEndpoint && descriptorLength >= 4 &&
                currentStream >= 0 && lastEndpointIsData &&
                d[2] == kEpGeneral) {
            StreamFormat& format = description.streams[currentStream];
            if (format.uacVersion == 1) {
                format.hasSampleRateControl = d[3] & 0x01;
            }
        } else if (type == kDescriptorInterface && inMidi &&
                descriptorLength >= 9) {
            // MIDI streaming interface already handled above
        } else if (type == kCsInterface && inMidi && descriptorLength >= 4) {
            // MIDI class-specific descriptors (ignored for now)
        } else if (type == kDescriptorEndpoint && inMidi &&
                descriptorLength >= 7) {
            const uint8_t endpoint = d[2];
            const uint16_t maxPacketSize = d[4] | (d[5] << 8);
            if (endpoint & 0x80) {
                description.midi.inEndpoint = endpoint;
                description.midi.inMaxPacketSize = maxPacketSize;
            } else {
                description.midi.outEndpoint = endpoint;
            }
        }

        pos += descriptorLength;
    }
    return description;
}

const StreamFormat* chooseOutput(const DeviceDescription& description) {
    const StreamFormat* pBest = nullptr;
    for (const auto& format : description.streams) {
        if (!format.isOutput || !format.isUsable()) {
            continue;
        }
        if (format.syncType == SyncType::Asynchronous && format.feedbackEndpoint == 0 &&
                !implicitFeedbackSource(description, format)) {
            continue;
        }
        if (!pBest || format.channels > pBest->channels ||
                (format.channels == pBest->channels &&
                        format.bitResolution > pBest->bitResolution)) {
            pBest = &format;
        }
    }
    return pBest;
}

const StreamFormat* implicitFeedbackSource(
        const DeviceDescription& description, const StreamFormat& output) {
    if (output.syncType != SyncType::Asynchronous || output.feedbackEndpoint != 0) {
        return nullptr;
    }
    const StreamFormat* pResult = nullptr;
    for (const auto& format : description.streams) {
        if (format.isOutput || !format.isUsable()) {
            continue;
        }
        if (output.bSynchAddress != 0 && format.endpoint == output.bSynchAddress) {
            return &format;
        }
        if (!pResult || format.channels < pResult->channels) {
            // The capture stream with fewest channels uses the least bandwidth
            pResult = &format;
        }
    }
    return pResult;
}

int packetsPerSecond(uint8_t bInterval, bool highSpeed) {
    const int exponent = std::clamp(static_cast<int>(bInterval), 1, 16) - 1;
    return std::max(1, (highSpeed ? 8000 : 1000) >> exponent);
}

uint32_t feedbackToFramesPerPacket(uint32_t raw,
        int feedbackBytes,
        uint32_t nominalFramesPerPacket) {
    if (raw == 0 || nominalFramesPerPacket == 0) {
        return 0;
    }
    if (feedbackBytes == 3) {
        raw &= 0xffffff;
    }
    // Feedback may be in 10.14 or 16.16, per frame or per microframe, or
    // otherwise misscaled by the device. The interpretations differ by at
    // least a factor of 2, so the first one close to the nominal rate wins.
    // Shifts are tried by magnitude: 16.16, then 10.14, then the rest.
    const uint64_t tolerance = nominalFramesPerPacket / 8;
    for (int shift : {0, 2, -2, 1, -1, 3, -3, 4, -4}) {
        for (int multiplier : {1, 2, 4, 8}) {
            uint64_t candidate = shift >= 0
                    ? (static_cast<uint64_t>(raw) << shift)
                    : (static_cast<uint64_t>(raw) >> (-shift));
            candidate *= multiplier;
            const uint64_t distance = candidate > nominalFramesPerPacket
                    ? candidate - nominalFramesPerPacket
                    : nominalFramesPerPacket - candidate;
            if (distance <= tolerance) {
                return static_cast<uint32_t>(std::min<uint64_t>(candidate, UINT32_MAX));
            }
        }
    }
    return 0;
}

std::string describe(const StreamFormat& format) {
    std::ostringstream out;
    out << "UAC" << format.uacVersion << (format.isOutput ? " out" : " in") << " if "
        << format.interfaceNumber << " alt " << format.altSetting << " ep 0x" << std::hex
        << static_cast<int>(format.endpoint) << std::dec << " " << format.channels << "ch "
        << format.bitResolution << "bit/" << format.subframeBytes * 8 << " max packet "
        << format.maxPacketSize << " interval " << static_cast<int>(format.bInterval);
    switch (format.syncType) {
    case SyncType::Asynchronous:
        out << " async";
        if (format.feedbackEndpoint) {
            out << " feedback 0x" << std::hex << static_cast<int>(format.feedbackEndpoint)
                << std::dec;
        } else {
            out << " implicit feedback";
        }
        break;
    case SyncType::Adaptive:
        out << " adaptive";
        break;
    case SyncType::Synchronous:
        out << " sync";
        break;
    default:
        break;
    }
    out << " rates";
    for (int rate : format.sampleRates) {
        out << " " << rate;
    }
    if (format.minRate > 0) {
        out << " " << format.minRate << "-" << format.maxRate;
    }
    return out.str();
}

} // namespace usbaudio
} // namespace mixxx