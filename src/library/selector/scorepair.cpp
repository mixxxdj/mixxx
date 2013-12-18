#include "library/selector/scorepair.h"

// TODO (kain88) is there a reason here not to user QPair typedef?
ScorePair::ScorePair(int trackId, double similarityScore) {
    id = trackId;
    score = similarityScore;
}
