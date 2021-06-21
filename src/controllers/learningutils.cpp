#include <QSet>
#include <QMap>
#include <QtDebug>

#include "controllers/learningutils.h"
#include "controllers/midi/midiutils.h"
#include "control/controlobject.h"

typedef QPair<MidiKey, unsigned char> MidiKeyAndValue;

struct MessageStats {
    MessageStats()
            : message_count(0),
              last_value(0) {
    }

    void addMessage(const MidiKeyAndValue& message) {
        // If we've never seen a message before, set last_value to this value.
        if (message_count == 0) {
            last_value = message.second;
        }

        MidiOpCode opcode = MidiUtils::opCodeFromStatus(message.first.status);
        unsigned char channel = MidiUtils::channelFromStatus(message.first.status);
        opcodes.insert(opcode);
        channels.insert(channel);
        controls.insert(message.first.control);

        message_count++;
        value_histogram[message.second]++;

        // Convert to integers first to get negative values.
        int absolute_difference = abs(static_cast<int>(message.second) -
                                      static_cast<int>(last_value));
        abs_diff_histogram[absolute_difference]++;
        last_value = message.second;
    }

    int message_count;
    QSet<MidiOpCode> opcodes;
    QSet<unsigned char> channels;
    QSet<unsigned char> controls;
    QMap<unsigned char, int> value_histogram;
    // The range of differences for unsigned char is -255 through 255.
    QMap<int, int> abs_diff_histogram;
    unsigned char last_value;
};

// static
MidiInputMappings LearningUtils::guessMidiInputMappings(
        const ConfigKey& control,
        const QList<QPair<MidiKey, unsigned char> >& messages) {
    QMap<unsigned char, MessageStats> stats_by_control;
    MessageStats stats;

    // Analyze the message
    foreach (const MidiKeyAndValue& message, messages) {
        stats.addMessage(message);
        stats_by_control[message.first.control].addMessage(message);
    }

    qDebug() << "LearningUtils guessing MIDI mapping from" << messages.size() << "messages.";

    foreach (MidiOpCode opcode, stats.opcodes) {
        qDebug() << "Opcode:" << opcode;
    }

    foreach (unsigned char channel, stats.channels) {
        qDebug() << "Channel:" << channel;
    }

    foreach (unsigned char control, stats.controls) {
        qDebug() << "Control:" << control;
    }

    for (auto it = stats.value_histogram.constBegin();
         it != stats.value_histogram.constEnd(); ++it) {
        qDebug() << "Overall Value:" << it.key()
                 << "count" << it.value();
    }

    for (auto control_it = stats_by_control.constBegin();
         control_it != stats_by_control.constEnd(); ++control_it) {
        QString controlName = QString("Control %1").arg(control_it.key());
        for (auto it = control_it->value_histogram.constBegin();
             it != control_it->value_histogram.constEnd(); ++it) {
            qDebug() << controlName << "Value:" << it.key()
                     << "count" << it.value();
        }
    }

    MidiInputMappings mappings;

    bool one_control = stats.controls.size() == 1;
    bool one_channel = stats.channels.size() == 1;
    bool only_note_on = stats.opcodes.size() == 1 && stats.opcodes.contains(MidiOpCode::NoteOn);
    bool only_note_on_and_note_off = stats.opcodes.size() == 2 &&
            stats.opcodes.contains(MidiOpCode::NoteOn) &&
            stats.opcodes.contains(MidiOpCode::NoteOff);

    bool has_cc = stats.opcodes.contains(MidiOpCode::ControlChange);
    bool only_cc = stats.opcodes.size() == 1 && has_cc;
    int num_cc_controls = 0;
    for (auto it = stats_by_control.constBegin();
         it != stats_by_control.constEnd(); ++it) {
        if (it->opcodes.contains(MidiOpCode::ControlChange)) {
            num_cc_controls++;
        }
    }

    bool two_values_7bit_max_and_min = stats.value_histogram.size() == 2 &&
            stats.value_histogram.contains(0x00) && stats.value_histogram.contains(0x7F);
    bool one_value_7bit_max_or_min = stats.value_histogram.size() == 1 &&
            (stats.value_histogram.contains(0x00) || stats.value_histogram.contains(0x7F));
    bool multiple_one_or_7f_values = stats.value_histogram.value(0x01, 0) > 1 ||
            stats.value_histogram.value(0x7F, 0) > 1;
    bool under_8_distinct_values = stats.value_histogram.size() < 8;
    bool no_0x00_value = !stats.value_histogram.contains(0x00);
    bool no_0x40_value = !stats.value_histogram.contains(0x40);
    bool multiple_values_around_0x40 = stats.value_histogram.value(0x41, 0) > 1 &&
            stats.value_histogram.value(0x3F, 0) > 1 && no_0x40_value &&
            under_8_distinct_values;

    // QMap keys are sorted so we can check this easily by checking the last key
    // is <= 0x7F.
    bool only_7bit_values = !stats.value_histogram.isEmpty() &&
            (stats.value_histogram.end() - 1).key() <= 0x7F;

    // A 7-bit two's complement ticker swinging from +1 to -1 can generate
    // unsigned differences of up to 126 (0x7E). If we see differences in
    // individual messages above 96 (0x60) that's a good hint that we're looking
    // at a two's complement ticker.
    bool abs_differences_above_60 = !stats.abs_diff_histogram.isEmpty() &&
            (stats.abs_diff_histogram.end() - 1).key() >= 0x60;

    if (one_control && one_channel &&
        two_values_7bit_max_and_min &&
        only_note_on_and_note_off) {
        // A standard button that sends NOTE_ON commands with 0x7F for
        // down-press and NOTE_OFF commands with 0x00 for release.
        MidiOptions options;

        MidiKey note_on;
        note_on.status = MidiUtils::statusFromOpCodeAndChannel(
                MidiOpCode::NoteOn, *stats.channels.begin());
        note_on.control = *stats.controls.begin();
        mappings.append(MidiInputMapping(note_on, options, control));

        MidiKey note_off;
        note_off.status = MidiUtils::statusFromOpCodeAndChannel(
                MidiOpCode::NoteOff, *stats.channels.begin());
        note_off.control = note_on.control;
        mappings.append(MidiInputMapping(note_off, options, control));
    } else if (one_control && one_channel &&
               two_values_7bit_max_and_min &&
               only_note_on) {
        // A standard button that only sends NOTE_ON commands with 0x7F for
        // down-press and 0x00 for release.
        MidiOptions options;

        MidiKey note_on;
        note_on.status = MidiUtils::statusFromOpCodeAndChannel(
                MidiOpCode::NoteOn, *stats.channels.begin());
        note_on.control = *stats.controls.begin();
        mappings.append(MidiInputMapping(note_on, options, control));
    } else if (one_control && one_channel &&
               one_value_7bit_max_or_min &&
               (only_note_on || only_cc)) {
        // This looks like a toggle switch. If we only got one value and it's
        // either min or max then this behaves like hard-coded toggle buttons on
        // the VCI-400. The opcode can be MidiOpCode::NoteOn or MidiOpCode::ControlChange.
        // Examples:
        // - VCI-400 vinyl toggle button (NOTE_ON)
        // - Korg nanoKontrol switches (CC)
        MidiOptions options;
        options.setFlag(MidiOption::Switch);

        MidiKey note_on;
        // The predicate ensures only NOTE_ON or CC messages can trigger this
        // logic.
        MidiOpCode code = only_note_on ? MidiOpCode::NoteOn : MidiOpCode::ControlChange;
        note_on.status = MidiUtils::statusFromOpCodeAndChannel(code, *stats.channels.begin());
        note_on.control = *stats.controls.begin();
        mappings.append(MidiInputMapping(note_on, options, control));
    } else if (one_control && one_channel &&
               only_cc && only_7bit_values &&
               no_0x00_value && (abs_differences_above_60 ||
                                 (under_8_distinct_values &&
                                  multiple_one_or_7f_values))) {
        // A two's complement +/- ticker (e.g. selector knobs and some jog
        // wheels). Values are typically +1 (0x01) and -1 (0x7F) but rapid
        // changes on some controllers can produce multiple ticks per
        // message. This must come before the standard knob CC block because it
        // looks like a standard CC knob other than the large swings in value
        // and repeats of 0x01 and 0x7F values.

        // We have a dedicated MidiOption for processing two's complement (even
        // though it is called 'selectknob' it is actually only two's complement
        // processing).
        MidiOptions options;
        options.setFlag(MidiOption::SelectKnob);
        MidiKey knob;
        knob.status = MidiUtils::statusFromOpCodeAndChannel(
                MidiOpCode::ControlChange, *stats.channels.begin());
        knob.control = *stats.controls.begin();
        mappings.append(MidiInputMapping(knob, options, control));
    } else if (one_control && one_channel && multiple_values_around_0x40) {
        // A "spread 64" ticker, where 0x40 is zero, positive jog values are
        // 0x41 and above, and negative jog values are 0x3F and below.
        MidiOptions options;
        options.setFlag(MidiOption::Spread64);

        MidiKey knob;
        knob.status = MidiUtils::statusFromOpCodeAndChannel(
                MidiOpCode::ControlChange, *stats.channels.begin());
        knob.control = *stats.controls.begin();
        mappings.append(MidiInputMapping(knob, options, control));
    } else if (one_channel && has_cc && num_cc_controls == 1 && only_7bit_values) {
        // A simple 7-bit knob that may have other messages mixed in. Some
        // controllers (e.g. the VCI-100) emit a center-point NOTE_ON (with
        // value 0x7F or 0x00 depending on if you are arriving at or leaving the
        // center point) instead of a CC value of 0x40. If the control we are
        // mapping has a reset control then we map the NOTE_ON messages to the
        // reset control.
        ConfigKey resetControl = control;
        resetControl.item.append("_set_default");
        bool hasResetControl = ControlObject::getControl(resetControl,
                                       ControlFlag::NoWarnIfMissing) != nullptr;

        // Find the CC control (based on the predicate one must exist) and add a
        // binding for it.
        for (auto it = stats_by_control.constBegin();
             it != stats_by_control.constEnd(); ++it) {
            if (it->opcodes.contains(MidiOpCode::ControlChange)) {
                MidiKey knob;
                knob.status = MidiUtils::statusFromOpCodeAndChannel(
                        MidiOpCode::ControlChange, *stats.channels.begin());
                knob.control = it.key();
                mappings.append(MidiInputMapping(knob, MidiOptions(), control));
            }

            // If we found a NOTE_ON, map it to reset if the control exists.
            // TODO(rryan): We need to modularize each recognizer here so we can
            // run the button recognizer on these messages minus the CC
            // messages.
            if (hasResetControl && it->opcodes.contains(MidiOpCode::NoteOn)) {
                MidiKey note_on;
                note_on.status = MidiUtils::statusFromOpCodeAndChannel(
                        MidiOpCode::NoteOn, *stats.channels.begin());
                note_on.control = it.key();
                mappings.append(MidiInputMapping(note_on, MidiOptions(), resetControl));
            }
        }
    } else if (one_channel && only_cc && stats.controls.size() == 2 &&
            stats_by_control.begin()->message_count > 10 &&
            stats_by_control.begin()->message_count ==
                    (++stats_by_control.begin())->message_count) {
        // If there are two CC controls with the same number of messages then we
        // assume this is a 14-bit CC knob. Now we need to determine which
        // control is the LSB and which is the MSB.

        // When observing an MSB/LSB sweep, the LSB will be very high frequency
        // compared to the MSB. Instead of doing an actual FFT on the two
        // signals, we can hack this by looking at the absolute differences
        // between messages. We expect to see many high/low wrap-arounds for the
        // LSB.
        int control1 = *stats.controls.begin();
        int control2 = *(++stats.controls.begin());

        int control1_max_abs_diff =
                (stats_by_control[control1].abs_diff_histogram.end() - 1).key();
        int control2_max_abs_diff =
                (stats_by_control[control2].abs_diff_histogram.end() - 1).key();

        // The control with the larger abs difference in messages is the LSB. If
        // they are equal we choose one arbitrarily (depends on QSet iteration
        // order which is undefined).
        int lsb_control = control1_max_abs_diff > control2_max_abs_diff ? control1 : control2;
        int msb_control = control1_max_abs_diff > control2_max_abs_diff ? control2 : control1;

        // NOTE(rryan): There is an industry convention that a 14-bit CC control
        // is a pair of controls offset by 32 (the lower is the MSB, the higher
        // is the LSB). My VCI-400 follows this convention, for example. I don't
        // use that convention here because it's not universal and we should be
        // able to come up with reasonable heuristics to identify an LSB and an
        // MSB.

        MidiKey msb;
        msb.status = MidiUtils::statusFromOpCodeAndChannel(
                MidiOpCode::ControlChange, *stats.channels.begin());
        msb.control = msb_control;
        MidiOptions msb_option;
        msb_option.setFlag(MidiOption::FourteenBitMSB);
        mappings.append(MidiInputMapping(msb, msb_option, control));

        MidiKey lsb;
        lsb.status = MidiUtils::statusFromOpCodeAndChannel(
                MidiOpCode::ControlChange, *stats.channels.begin());
        lsb.control = lsb_control;
        MidiOptions lsb_option;
        lsb_option.setFlag(MidiOption::FourteenBitLSB);
        mappings.append(MidiInputMapping(lsb, lsb_option, control));
    }

    if (mappings.isEmpty() && !messages.isEmpty()) {
        // Fall back. Map the first message we got. By dumb luck this might work
        // for 14-bit faders, for example if the high-order byte is first (it
        // will just be a 7-bit fader).
        MidiOptions options;

        // TODO(rryan): Feedback to the user that we didn't do anything
        // intelligent here.
        mappings.append(MidiInputMapping(messages.first().first, options, control));
    }

    // Add control and description info to each learned input mapping.
    for (MidiInputMappings::iterator it = mappings.begin();
         it != mappings.end(); ++it) {
        MidiInputMapping& mapping = *it;
        mapping.description = QString("MIDI Learned from %1 messages.")
                .arg(messages.size());
    }

    return mappings;
}
