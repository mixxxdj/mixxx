#include "library/autodj/track/fadeabletrackordeckattributes.h"

#include "library/autodj/autodjconstants.h"
#include "moc_fadeabletrackordeckattributes.cpp"

FadeableTrackOrDeckAttributes::FadeableTrackOrDeckAttributes()
        : startPos(AutoDJConstants::kKeepPosition),
          fadeBeginPos(1.0),
          fadeEndPos(1.0),
          adjustDurationSeconds(0.0),
          isFromDeck(false) {
}
