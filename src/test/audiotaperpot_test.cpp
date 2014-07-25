#include <gtest/gtest.h>

#include <QTestEventList>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "util/math.h"

class AudioTaperPotTest : public MixxxTest {
  public:
    AudioTaperPotTest() {
    }

  protected:
    virtual void SetUp() {
    }
};

TEST_F(AudioTaperPotTest, ScaleTest) {
    {
        const double minDB = -6;
        const double maxDB = 6;
        const double neutralParameter = 0.5;
        ControlAudioTaperPotBehavior* catpb = new ControlAudioTaperPotBehavior(minDB, maxDB, neutralParameter);
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(0.0, catpb->parameterToValue(0));
        // Parameter 0 is always 0 (-Infinity)
        ASSERT_EQ(1.0, catpb->parameterToValue(neutralParameter));
        // Parameter 1 is always maxDB
        ASSERT_EQ(db2ratio(maxDB), catpb->parameterToValue(1));
        // value 1 is always on a Integer midi value
        ASSERT_EQ(0.0, fmod(catpb->valueToMidiParameter(1),1));
        delete catpb;
    }
}

