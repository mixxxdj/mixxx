#ifndef SCOREPAIR_H
#define SCOREPAIR_H

class ScorePair {
  public:
    ScorePair(int trackId, double similarityScore);
    int id;
    double score;
};

#endif // SCOREPAIR_H
