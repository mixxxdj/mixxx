#include <QSet>
#include <QMap>
#include <QtDebug>

#include "controllers/learningutils.h"

typedef QPair<MidiKey, unsigned char> MidiKeyAndValue;

// static
QList<QPair<MidiKey, MidiOptions> > LearningUtils::guessMidiInputMappings(
    const QList<QPair<MidiKey, unsigned char> >& messages) {

    QSet<MidiOpCode> opcodes;
    QSet<unsigned char> channels;
    QSet<unsigned char> controls;
    QMap<unsigned char, int> value_histogram;
    // The range of differences for unsigned char is -255 through 255.
    QMap<int, int> abs_diff_histogram;

    // Analyze the message
    unsigned char lastValue = !messages.isEmpty() ? messages.at(0).second : 0;
    foreach (const MidiKeyAndValue& message, messages) {
        MidiOpCode opcode = opCodeFromStatus(message.first.status);
        unsigned char channel = channelFromStatus(message.first.status);
        opcodes.insert(opcode);
        channels.insert(channel);
        controls.insert(message.first.control);
        value_histogram[message.second]++;

        // Convert to integers first to get negative values.
        int absolute_difference = abs(static_cast<int>(message.second) -
                                      static_cast<int>(lastValue));
        abs_diff_histogram[absolute_difference]++;
        lastValue = message.second;
    }

    qDebug() << "LearningUtils guessing MIDI mapping from" << messages.size() << "messages.";

    if (channels.size() > 1) {
        qDebug() << "More than one channel. Maybe you hit multiple controls?";
    }

    if (controls.size() > 1) {
        qDebug() << "More than one control. Maybe you hit multiple controls?";
    }

    foreach (MidiOpCode opcode, opcodes) {
        qDebug() << "Opcode:" << opcode;
    }

    foreach (unsigned char channel, channels) {
        qDebug() << "Channel:" << channel;
    }

    foreach (unsigned char control, controls) {
        qDebug() << "Control:" << control;
    }

    for (QMap<unsigned char, int>::const_iterator it = value_histogram.begin();
         it != value_histogram.end(); ++it) {
        qDebug() << "Value:" << it.key()
                 << "count" << it.value();
    }

    QList<QPair<MidiKey, MidiOptions> > mappings;

    bool one_control = controls.size() == 1;
    bool one_channel = controls.size() == 1;
    bool only_note_on = opcodes.size() == 1 && opcodes.contains(MIDI_NOTE_ON);
    bool only_note_on_and_note_off = opcodes.size() == 2 &&
            opcodes.contains(MIDI_NOTE_ON) &&
            opcodes.contains(MIDI_NOTE_OFF);
    bool only_cc = opcodes.size() == 1 && opcodes.contains(MIDI_CC);

    bool two_values_7bit_max_and_min = value_histogram.size() == 2 &&
            value_histogram.contains(0x00) && value_histogram.contains(0x7F);
    bool one_value_7bit_max_or_min = value_histogram.size() == 1 &&
            (value_histogram.contains(0x00) || value_histogram.contains(0x7F));
    bool multiple_one_or_7f_values = value_histogram.value(0x01, 0x00) > 1 ||
            value_histogram.value(0x7F, 0x00) > 1;

    // QMap keys are sorted so we can check this easily by checking the last key
    // is <= 0x7F.
    bool only_7bit_values = !value_histogram.isEmpty() &&
            (value_histogram.end() - 1).key() <= 0x7F;

    // A 7-bit two's complement ticker swinging from +1 to -1 can generate
    // unsigned differences of up to 126 (0x7E). If we see differences in
    // individual messages above 96 (0x60) that's a good hint that we're looking
    // at a two's complement ticker.
    bool abs_differences_above_60 = !abs_diff_histogram.isEmpty() &&
            (abs_diff_histogram.end() - 1).key() >= 0x60;

    if (one_control && one_channel &&
        two_values_7bit_max_and_min &&
        only_note_on_and_note_off) {
        // A standard button that sends NOTE_ON commands with 0x7F for
        // down-press and NOTE_OFF commands with 0x00 for release.
        MidiOptions options;

        MidiKey note_on;
        note_on.status = MIDI_NOTE_ON | *channels.begin();
        note_on.control = *controls.begin();
        mappings.append(qMakePair(note_on, options));

        MidiKey note_off;
        note_off.status = MIDI_NOTE_OFF | *channels.begin();
        note_off.control = note_on.control;
        mappings.append(qMakePair(note_off, options));
    } else if (one_control && one_channel &&
               two_values_7bit_max_and_min &&
               only_note_on) {
        // A standard button that only sends NOTE_ON commands with 0x7F for
        // down-press and 0x00 for release.
        MidiOptions options;

        MidiKey note_on;
        note_on.status = MIDI_NOTE_ON | *channels.begin();
        note_on.control = *controls.begin();
        mappings.append(qMakePair(note_on, options));
    } else if (one_control && one_channel &&
               one_value_7bit_max_or_min &&
               only_note_on) {
        // This looks like a toggle switch. If we only got one value and it's
        // either min or max then this behaves like hard-coded toggle buttons on
        // the VCI-400.
        MidiOptions options;
        options.sw = true;

        MidiKey note_on;
        note_on.status = MIDI_NOTE_ON | *channels.begin();
        note_on.control = *controls.begin();
        mappings.append(qMakePair(note_on, options));
    } else if (one_control && one_channel &&
               only_cc && only_7bit_values && (abs_differences_above_60 ||
                                               multiple_one_or_7f_values)) {
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
        options.selectknob = true;

        MidiKey knob;
        knob.status = MIDI_CC | *channels.begin();
        knob.control = *controls.begin();
        mappings.append(qMakePair(knob, options));
    } else if (one_control && one_channel &&
               only_cc && only_7bit_values) {
        // A simple 7-bit knob.
        MidiOptions options;

        MidiKey knob;
        knob.status = MIDI_CC | *channels.begin();
        knob.control = *controls.begin();
        mappings.append(qMakePair(knob, options));
    }

    if (mappings.isEmpty() && !messages.isEmpty()) {
        // Fall back. Map the first message we got. By dumb luck this might work
        // for 14-bit faders, for example if the high-order byte is first (it
        // will just be a 7-bit fader).
        MidiOptions options;

        // TODO(rryan): Feedback to the user that we didn't do anything
        // intelligent here.
        mappings.append(qMakePair(messages.first().first, options));
    }

    return mappings;
}
