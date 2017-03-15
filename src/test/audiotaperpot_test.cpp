#include <gtest/gtest.h>

#include <QTestEventList>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "control/controlobject.h"
#include "util/math.h"

class AudioTaperPotTest : public MixxxTest {};

TEST_F(AudioTaperPotTest, ScaleTest) {
    {
        const double minDB = -6;
        const double maxDB = 6;
        const double neutralParameter = 0.5;
        ControlAudioTaperPotBehavior catpb(minDB, maxDB, neutralParameter);
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(0.0, catpb.parameterToValue(0));
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(1.0, catpb.parameterToValue(neutralParameter));
        // Parameter 1 is always maxDB
        ASSERT_EQ(db2ratio(maxDB), catpb.parameterToValue(1));
        // value 1 is always on a Integer midi value
        double neutralMidi = catpb.valueToMidiParameter(1);
        ASSERT_EQ(0.0, fmod(neutralMidi, 1));
        // Midi value 64 should result in 0,5
        ASSERT_EQ(neutralParameter, catpb.midiValueToParameter(neutralMidi));
        // roundtrip check
        ASSERT_DOUBLE_EQ(0.25, catpb.parameterToValue(catpb.midiValueToParameter(catpb.valueToMidiParameter(0.25))));
        ASSERT_DOUBLE_EQ(0.75, catpb.parameterToValue(catpb.midiValueToParameter(catpb.valueToMidiParameter(0.75))));
    }

    {
        const double minDB = 0;
        const double maxDB = 6;
        const double neutralParameter = 0.5;
        ControlAudioTaperPotBehavior catpb(minDB, maxDB, neutralParameter);
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(0.0, catpb.parameterToValue(0));
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(1.0, catpb.parameterToValue(neutralParameter));
        // Parameter 1 is always maxDB
        ASSERT_EQ(db2ratio(maxDB), catpb.parameterToValue(1));
        // value 1 is always on a Integer midi value
        double neutralMidi = catpb.valueToMidiParameter(1);
        ASSERT_EQ(0.0, fmod(neutralMidi, 1));
        // Midi value 64 should result in 0,5
        ASSERT_EQ(neutralParameter, catpb.midiValueToParameter(neutralMidi));
        // roundtrip check
        ASSERT_DOUBLE_EQ(0.25, catpb.parameterToValue(catpb.midiValueToParameter(catpb.valueToMidiParameter(0.25))));
        ASSERT_DOUBLE_EQ(0.75, catpb.parameterToValue(catpb.midiValueToParameter(catpb.valueToMidiParameter(0.75))));
    }

    {
        const double minDB = -6;
        const double maxDB = 0;
        const double neutralParameter = 1;
        ControlAudioTaperPotBehavior catpb(minDB, maxDB, neutralParameter);
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(0.0, catpb.parameterToValue(0));
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(1.0, catpb.parameterToValue(neutralParameter));
        // Parameter 1 is always maxDB
        ASSERT_EQ(db2ratio(maxDB), catpb.parameterToValue(1));
        // value 1 is always on a Integer midi value
        double neutralMidi = catpb.valueToMidiParameter(1);
        ASSERT_EQ(0.0, fmod(neutralMidi, 1));
        // Midi value 64 should result in 0,5
        ASSERT_EQ(neutralParameter, catpb.midiValueToParameter(neutralMidi));
        // roundtrip checkx
        ASSERT_DOUBLE_EQ(0.25, catpb.parameterToValue(catpb.midiValueToParameter(catpb.valueToMidiParameter(0.25))));
        ASSERT_DOUBLE_EQ(0.75, catpb.parameterToValue(catpb.midiValueToParameter(catpb.valueToMidiParameter(0.75))));
    }
}
