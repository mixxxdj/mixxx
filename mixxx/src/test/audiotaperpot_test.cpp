#include <gtest/gtest.h>

#include <QTestEventList>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "control/controlobject.h"
#include "util/math.h"

class AudioTaperPotTest : public MixxxTest {};

TEST_F(AudioTaperPotTest, ScaleTest) {
    {
        constexpr double minDB = -6;
        constexpr double maxDB = 6;
        constexpr double neutralParameter = 0.5;
        ControlAudioTaperPotBehavior catpb(minDB, maxDB, neutralParameter);
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_DOUBLE_EQ(0.0, catpb.parameterToValue(0));
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_DOUBLE_EQ(1.0, catpb.parameterToValue(neutralParameter));
        // Parameter 1 is always maxDB
        ASSERT_DOUBLE_EQ(db2ratio(maxDB), catpb.parameterToValue(1));
        // value 1 is always on a Integer midi value
        double neutralMidi = catpb.valueToMidiParameter(1);
        ASSERT_DOUBLE_EQ(0.0, fmod(neutralMidi, 1));
        // Midi value 64 should result in 0,5
        ASSERT_DOUBLE_EQ(neutralParameter, catpb.midiToParameter(neutralMidi));
        // roundtrip check
        ASSERT_DOUBLE_EQ(0.25, catpb.parameterToValue(catpb.midiToParameter(catpb.valueToMidiParameter(0.25))));
        ASSERT_DOUBLE_EQ(0.75, catpb.parameterToValue(catpb.midiToParameter(catpb.valueToMidiParameter(0.75))));
    }

    {
        constexpr double minDB = 0;
        constexpr double maxDB = 6;
        constexpr double neutralParameter = 0.5;
        ControlAudioTaperPotBehavior catpb(minDB, maxDB, neutralParameter);
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_DOUBLE_EQ(0.0, catpb.parameterToValue(0));
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_DOUBLE_EQ(1.0, catpb.parameterToValue(neutralParameter));
        // Parameter 1 is always maxDB
        ASSERT_DOUBLE_EQ(db2ratio(maxDB), catpb.parameterToValue(1));
        // value 1 is always on a Integer midi value
        double neutralMidi = catpb.valueToMidiParameter(1);
        ASSERT_DOUBLE_EQ(0.0, fmod(neutralMidi, 1));
        // Midi value 64 should result in 0,5
        ASSERT_DOUBLE_EQ(neutralParameter, catpb.midiToParameter(neutralMidi));
        // roundtrip check
        ASSERT_DOUBLE_EQ(0.25, catpb.parameterToValue(catpb.midiToParameter(catpb.valueToMidiParameter(0.25))));
        ASSERT_DOUBLE_EQ(0.75, catpb.parameterToValue(catpb.midiToParameter(catpb.valueToMidiParameter(0.75))));
    }

    {
        constexpr double minDB = -6;
        constexpr double maxDB = 0;
        constexpr double neutralParameter = 1;
        ControlAudioTaperPotBehavior catpb(minDB, maxDB, neutralParameter);
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_DOUBLE_EQ(0.0, catpb.parameterToValue(0));
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_DOUBLE_EQ(1.0, catpb.parameterToValue(neutralParameter));
        // Parameter 1 is always maxDB
        ASSERT_DOUBLE_EQ(db2ratio(maxDB), catpb.parameterToValue(1));
        // value 1 is always on a Integer midi value
        double neutralMidi = catpb.valueToMidiParameter(1);
        ASSERT_DOUBLE_EQ(0.0, fmod(neutralMidi, 1));
        // Midi value 64 should result in 0,5
        ASSERT_DOUBLE_EQ(neutralParameter, catpb.midiToParameter(neutralMidi));
        // roundtrip checkx
        ASSERT_DOUBLE_EQ(0.25, catpb.parameterToValue(catpb.midiToParameter(catpb.valueToMidiParameter(0.25))));
        ASSERT_DOUBLE_EQ(0.75, catpb.parameterToValue(catpb.midiToParameter(catpb.valueToMidiParameter(0.75))));
    }
}
