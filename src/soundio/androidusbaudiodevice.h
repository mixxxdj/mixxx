#pragma once

#include <QJniObject>
#include <QList>
#include <QString>
#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "soundio/usbaudiodescriptors.h"

struct libusb_context;
struct libusb_device_handle;
struct libusb_transfer;

namespace mixxx {
namespace androidusb {

/// When Mixxx drives USB sound cards itself instead of using Android's audio
enum class DirectMode {
    /// Only multichannel (> 2 outputs) sound cards, which many Android devices
    /// fail to play to (e.g. DJ controllers with a headphone output)
    Auto = 0,
    /// Every supported USB sound card
    Always = 1,
    /// Never, always use Android's audio system
    Never = 2,
};

/// User space driver for class compliant USB audio interfaces on Android.
///
/// Some Android devices can't play to multichannel USB sound cards (e.g.
/// Qualcomm "USB audio offload" rejects the 4 channel output of DJ
/// controllers). This class bypasses the Android audio system: it detaches
/// the kernel driver and streams the audio itself with isochronous transfers.
/// It supports USB Audio Class 1 and 2 with synchronous, adaptive and
/// asynchronous (explicit or implicit feedback) endpoints. Because the kernel
/// driver owns the whole sound card, the USB-MIDI interface of the device is
/// driven here as well.
class AndroidUsbAudioDevice {
  public:
    /// Fills pDestination with the given number of frames in the device
    /// format. Called on the USB event thread.
    using RenderCallback = std::function<void(uint8_t* pDestination, int frames)>;
    /// Receives one 4 byte USB-MIDI event packet. Called on the USB event thread.
    using MidiCallback = std::function<void(const uint8_t* pPacket)>;

    ~AndroidUsbAudioDevice();

    /// Looks for supported USB audio devices and asks for the USB permission
    /// if needed. Devices found by a previous scan are kept as long as they
    /// are connected. With waitForPermission == false the permission is only
    /// requested; the answer triggers another scan through the hotplug
    /// notification. Must be called on the Qt main thread.
    static std::vector<std::shared_ptr<AndroidUsbAudioDevice>> scan(
            DirectMode mode, bool waitForPermission);
    /// The devices found by the last scan that are still connected
    static std::vector<std::shared_ptr<AndroidUsbAudioDevice>> devices();

    const QString& productName() const {
        return m_productName;
    }
    const QString& usbDeviceName() const {
        return m_usbDeviceName;
    }
    const QString& manufacturerName() const {
        return m_manufacturerName;
    }
    uint16_t vendorId() const {
        return m_vendorId;
    }
    uint16_t productId() const {
        return m_productId;
    }
    const usbaudio::StreamFormat& outputFormat() const {
        return m_outputFormat;
    }
    const usbaudio::MidiInterface& midiInterface() const {
        return m_description.midi;
    }
    /// Sample rates the output supports. For UAC2 devices this is refined
    /// once the device has been claimed.
    QList<int> sampleRates() const;
    bool isConnected() const {
        return !m_disconnected.load();
    }
    /// True once the kernel driver has been detached. From then on the
    /// device is only reachable through this class (also for MIDI).
    bool isClaimed() const {
        return m_claimed.load();
    }

    /// Starts the output. Each transfer carries about transferMillis of
    /// audio and numTransfers are queued at the USB controller.
    bool startOutput(int sampleRate,
            int transferMillis,
            int numTransfers,
            RenderCallback renderCallback);
    void stopOutput();
    bool isOutputRunning() const {
        return m_outputRunning.load();
    }
    /// Number of frames submitted to the USB controller and not yet played
    int framesInFlight() const {
        return m_framesInFlight.load();
    }

    bool startMidiInput(MidiCallback callback);
    void stopMidiInput();
    /// Sends USB-MIDI event packets (multiples of 4 bytes)
    bool sendMidi(const uint8_t* pData, int length);

  private:
    explicit AndroidUsbAudioDevice(const QJniObject& usbDevice);

    bool open(DirectMode mode, bool waitForPermission);
    bool claim();
    bool claimInterface(int interfaceNumber);
    void queryUac2SampleRates();
    bool setSampleRate(const usbaudio::StreamFormat& format, int sampleRate);
    /// The sample rate the device reports, or 0 if it can't be read
    int readSampleRate(const usbaudio::StreamFormat& format);
    /// Sets the rate of the output (and its implicit feedback capture) with
    /// the altsettings active as the device needs them
    void applySampleRate(int sampleRate);
    /// Measures the rate the device actually plays at from its clock, and
    /// logs it once per start
    void measureClock(uint64_t framesPerPacket16_16);
    void startEventThread();
    void stopEventThread();
    void markDisconnected();
    bool isHighSpeed() const;

    void fillOutputTransfer(libusb_transfer* pTransfer, bool silent);
    int nextPacketFrames();
    void cancelAndFree(std::vector<libusb_transfer*>* pTransfers, std::atomic<int>* pPending);
    static void outputTransferCallback(libusb_transfer* pTransfer);
    static void feedbackTransferCallback(libusb_transfer* pTransfer);
    static void implicitTransferCallback(libusb_transfer* pTransfer);
    static void midiInTransferCallback(libusb_transfer* pTransfer);

    QJniObject m_usbDevice;
    QJniObject m_connection;
    QString m_productName;
    QString m_usbDeviceName;
    QString m_manufacturerName;
    uint16_t m_vendorId;
    uint16_t m_productId;
    libusb_context* m_pContext;
    libusb_device_handle* m_pHandle;

    usbaudio::DeviceDescription m_description;
    usbaudio::StreamFormat m_outputFormat;
    /// Capture stream that clocks an implicit feedback output
    usbaudio::StreamFormat m_implicitFormat;
    bool m_hasImplicitFeedback;
    std::vector<int> m_claimedInterfaces;

    std::mutex m_mutex;
    std::atomic<bool> m_claimed;
    std::atomic<bool> m_disconnected;
    std::thread m_eventThread;
    std::atomic<bool> m_eventThreadRunning;

    // Audio output; the following are only used on the event thread while
    // the output is running
    RenderCallback m_renderCallback;
    std::vector<libusb_transfer*> m_outputTransfers;
    std::atomic<bool> m_outputRunning;
    std::atomic<int> m_outputTransfersPending;
    std::atomic<int> m_framesInFlight;
    int m_sampleRate;
    int m_packetsPerSecond;
    int m_frameAccumulator;
    /// Asynchronous endpoints: frames per packet in 16.16 fixed point
    uint32_t m_nominalFramesPerPacket;
    uint32_t m_framesPerPacket;
    uint32_t m_fractionalFrames;
    std::vector<libusb_transfer*> m_feedbackTransfers;
    std::atomic<int> m_feedbackTransfersPending;
    std::deque<int> m_implicitPacketFrames;
    // Clock measurement of the first second, on the event thread
    uint64_t m_measuredFrames16_16;
    int m_measuredPackets;
    bool m_clockLogged;
    bool m_midiInRunning;
    bool m_midiInPending;
    libusb_transfer* m_pMidiInTransfer;
    MidiCallback m_midiCallback;
};

} // namespace androidusb
} // namespace mixxx