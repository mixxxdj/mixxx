#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

/// Parser for the descriptors of USB Audio Class 1 and 2 devices.
///
/// It only depends on the C++ standard library so it can be unit tested
/// without a device; AndroidUsbAudioDevice feeds it the raw descriptors
/// returned by UsbDeviceConnection.getRawDescriptors().
namespace mixxx {
namespace usbaudio {

enum class SyncType {
    None = 0,
    Asynchronous = 1,
    Adaptive = 2,
    Synchronous = 3,
};

/// One alternate setting of an audio streaming interface
struct StreamFormat {
    int interfaceNumber = -1;
    int altSetting = -1;
    int uacVersion = 1;
    bool isOutput = true;
    uint8_t endpoint = 0;
    int maxPacketSize = 0;
    uint8_t bInterval = 1;
    SyncType syncType = SyncType::None;
    /// Explicit feedback endpoint (asynchronous output), 0 if none
    uint8_t feedbackEndpoint = 0;
    uint8_t bSynchAddress = 0;
    bool isPcm = false;
    int channels = 0;
    int subframeBytes = 0;
    int bitResolution = 0;
    int terminalLink = -1;
    /// Discrete rates (UAC1). For UAC2 the rates are queried from the clock.
    std::vector<int> sampleRates;
    /// Continuous range (UAC1 bSamFreqType == 0)
    int minRate = 0;
    int maxRate = 0;
    /// UAC1: the endpoint accepts SET_CUR sampling frequency
    bool hasSampleRateControl = false;

    bool isUsable() const {
        return interfaceNumber >= 0 && altSetting > 0 && endpoint != 0 && isPcm &&
                channels > 0 && (subframeBytes == 2 || subframeBytes == 3 || subframeBytes == 4);
    }
    int frameBytes() const {
        return channels * subframeBytes;
    }
    bool supportsRate(int rate) const;
};

/// A clock entity of an UAC2 audio control interface
struct ClockEntity {
    enum class Type {
        Source,
        Selector,
        Multiplier,
    };
    Type type = Type::Source;
    std::vector<int> sources; // inputs of selectors and multipliers
};

struct ControlInterface {
    int interfaceNumber = -1;
    int uacVersion = 1;
    /// Terminal id -> clock id (UAC2)
    std::map<int, int> terminalClocks;
    std::map<int, ClockEntity> clocks;

    /// Follows selectors and multipliers to the clock source of a terminal,
    /// -1 if unknown
    int clockSourceForTerminal(int terminalId) const;
};

struct MidiInterface {
    int interfaceNumber = -1;
    uint8_t inEndpoint = 0;
    uint8_t outEndpoint = 0;
    int inMaxPacketSize = 0;

    bool isValid() const {
        return interfaceNumber >= 0 && (inEndpoint != 0 || outEndpoint != 0);
    }
};

struct DeviceDescription {
    std::vector<ControlInterface> controls;
    std::vector<StreamFormat> streams;
    MidiInterface midi;

    /// The audio control interface that belongs to the streaming interface
    /// (the one with the closest lower interface number)
    const ControlInterface* controlFor(const StreamFormat& format) const;
};

/// Parses a device descriptor followed by the configuration descriptor(s),
/// or just a configuration descriptor.
DeviceDescription parse(const uint8_t* pData, size_t length);

/// Picks the best output format: most channels, then highest resolution.
/// Returns nullptr if there is no usable output.
const StreamFormat* chooseOutput(const DeviceDescription& description);

/// The capture stream that clocks an asynchronous output without a feedback
/// endpoint ("implicit feedback"), or nullptr.
const StreamFormat* implicitFeedbackSource(
        const DeviceDescription& description, const StreamFormat& output);

/// Packets per second of an isochronous endpoint
int packetsPerSecond(uint8_t bInterval, bool highSpeed);

/// Converts a raw feedback value to frames per packet in 16.16 fixed point.
/// The format is guessed by comparing with the nominal rate, like the Linux
/// driver does (10.14 for full speed, 16.16 for high speed, some devices
/// differ). Returns 0 if the value is implausible.
uint32_t feedbackToFramesPerPacket(uint32_t raw,
        int feedbackBytes,
        uint32_t nominalFramesPerPacket);

/// Human readable summary for logs
std::string describe(const StreamFormat& format);

} // namespace usbaudio
} // namespace mixxx
