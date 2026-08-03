#include "library/autodj/track/trackordeckattributes.h"

#include "moc_trackordeckattributes.cpp"

TrackOrDeckAttributes::TrackOrDeckAttributes()
        : startPos(TrackOrDeckAttributes::kKeepPosition),
          fadeBeginPos(1.0),
          fadeEndPos(1.0),
          isFromDeck(false) {
}

TrackOrDeckAttributes::~TrackOrDeckAttributes() {
}
