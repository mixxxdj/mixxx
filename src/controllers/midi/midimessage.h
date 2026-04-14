#pragma once

#include <QFlags>
#include <QJSValue>
#include <QList>
#include <QMetaType>
#include <QPair>
#include <QUuid>
#include <QtDebug>
#include <cstdint>
#include <memory>
#include <variant>

#include "controllers/midi/midiopcode.h"
#include "preferences/usersettings.h"
#include "util/always_false_v.h"
#include "util/compatibility/qhash.h"

enum class MidiOption : uint16_t {
    None = 0x0000,
    Invert = 0x0001,
    Rot64 = 0x0002,
    Rot64Invert = 0x0004,
    Rot64Fast = 0x0008,
    Diff = 0x0010,
    /// Button Down (!=00) and Button Up (00) events happen together
    Button = 0x0020,
    /// Button Down (!=00) and Button Up (00) events happen separately
    Switch = 0x0040,
    /// Accelerated Difference from 64
    Spread64 = 0x0080,
    /// Generic Hercules Range Correction (0x01 -> +1; 0x7f -> -1)
    HercJog = 0x0100,
    /// Relative Knob which can be turned forever and outputs a signed value
    SelectKnob = 0x0200,
    /// Prevents Sudden Changes when hardware position differs from software value
    SoftTakeover = 0x0400,
    /// Maps a MIDI control to a custom JavaScript function
    Script = 0x0800,
    /// Nessage supplies the LSB of a 14-bit message
    FourteenBitLSB = 0x1000,
    /// Nessage supplies the MSB of a 14-bit message
    FourteenBitMSB = 0x2000,
    /// Generic Hercules Range Correction (0x01 -> +5; 0x7f -> -5)
    HercJogFast = 0x4000,
};
Q_DECLARE_FLAGS(MidiOptions, MidiOption);
Q_DECLARE_OPERATORS_FOR_FLAGS(MidiOptions);
Q_DECLARE_METATYPE(MidiOptions);

struct MidiOutput {
    MidiOutput()
            : message(0) {
        // MSVC gets confused and thinks min/max are macros so they can't appear
        // in the initializer list.
        min = 0.0;
        max = 0.0;
    }

    bool operator==(const MidiOutput& other) const {
        return min == other.min && max == other.max && message == other.message;
    }

    double min;
    double max;
    union {
        uint32_t message;
        struct
        {
            unsigned char status : 8;
            unsigned char control : 8;
            unsigned char on : 8;
            unsigned char off : 8;
        };
    };
};

struct MidiKey {
    MidiKey();
    MidiKey(unsigned char status, unsigned char control);

    bool operator==(const MidiKey& other) const {
        return key == other.key;
    }

    union {
        uint16_t key;
        struct
        {
            unsigned char status : 8;
            unsigned char control : 8;
        };
    };
};

struct MidiInputMapping {
    MidiInputMapping() {
    }

    MidiInputMapping(MidiKey key, MidiOptions options)
            : key(key),
              options(options) {
    }

    MidiInputMapping(MidiKey key,
            MidiOptions options,
            const std::variant<ConfigKey, std::shared_ptr<QJSValue>>& control)
            : key(key),
              options(options),
              control(control) {
    }

    MidiInputMapping(MidiKey key,
            MidiOptions options,
            const std::variant<ConfigKey, std::shared_ptr<QJSValue>>& control,
            const QString& description)
            : key(key),
              options(options),
              control(control),
              description(description) {
    }

    bool operator==(const MidiInputMapping& other) const {
        return std::visit([&](auto first, auto second) {
            using T = std::decay_t<decltype(first)>;
            using U = std::decay_t<decltype(second)>;

            if constexpr (!std::is_same_v<T, U>) {
                return false;
            } else if constexpr (std::is_same_v<T, ConfigKey>) {
                return key == other.key &&
                        options == other.options &&
                        std::get<ConfigKey>(control) == std::get<ConfigKey>(other.control) &&
                        description == other.description;
            } else if constexpr (std::is_same_v<T, std::shared_ptr<QJSValue>>) {
                const auto& otherControl = std::get<std::shared_ptr<QJSValue>>(other.control);
                const auto& thisControl = std::get<std::shared_ptr<QJSValue>>(control);
                return key == other.key && options == other.options &&
                        thisControl->strictlyEquals(*otherControl) &&
                        description == other.description;
            } else
                static_assert(always_false_v<T>, "non-exhaustive visitor");
        },
                control,
                other.control);
    }

    MidiKey key;
    MidiOptions options;
    // TODO: find a new name to represent both an XML's control entry and an anonymous JS function
    std::variant<ConfigKey, std::shared_ptr<QJSValue>> control;
    QString description;
};
typedef QList<MidiInputMapping> MidiInputMappings;

struct MidiOutputMapping {
    bool operator==(const MidiOutputMapping& other) const {
        return output == other.output && controlKey == other.controlKey &&
                description == other.description;
    }

    MidiOutput output;
    ConfigKey controlKey;
    QString description;
};
typedef QList<MidiOutputMapping> MidiOutputMappings;
