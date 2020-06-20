#include <QList>
#include <QMap>
#include <QString>
#include <QtDebug>
#include <algorithm>

#include "analyzer/analyzerrhythm.h"
#include "analyzer/analyzerrhythmstats.h"
#include "util/math.h"

namespace {
constexpr bool sDebug = true;
constexpr int kHistogramDecimalPlaces = 2;
constexpr double kCorrectBeatLocalBpmEpsilon = 0.05; // 0.2;
const double kHistogramDecimalScale = pow(10.0, kHistogramDecimalPlaces);
constexpr double kBpmFilterTolerance = 1.0;
constexpr int kBeatsToCountTempo = 12;
constexpr double kMaxBpmError = 0.06;

} // namespace

double AnalyzerRhythm::medianTempo(const QVector<mixxx::audio::FramePos>& beats) {
    if (beats.size() < 2) {
        return 0;
    }
    auto [tempoList, tempoFrequencyTable] = computeRawTemposAndFrequency(beats);
    // Get the median BPM.
    std::sort(tempoList.begin(), tempoList.end());
    const double medianBpm = BeatStatistics::computeSampleMedian(tempoList);
    const double roundedBpm = floor(medianBpm + 0.5);
    const double bpmDiff = fabs(roundedBpm - medianBpm);
    bool performRounding = (bpmDiff <= kMaxBpmError);
    return performRounding ? roundedBpm : medianBpm;
}

std::tuple<QList<double>, QMap<double, int>> AnalyzerRhythm::computeRawTemposAndFrequency(
        const QVector<mixxx::audio::FramePos>& beats, const int beatWindow) {
    QList<double> tempoList;
    QMap<double, int> tempoFrequencyTable;
    for (int i = beatWindow; i < beats.size(); i += 1) {
        mixxx::audio::FramePos start_frame = beats.at(i - beatWindow);
        mixxx::audio::FramePos end_frame = beats.at(i);
        // Time needed to count beatWindow beats
        double time = (end_frame - start_frame) / m_sampleRate;
        if (time == 0)
            continue;
        double localBpm = 60.0 * beatWindow / time;
        // round BPM to have two decimal places
        double roundedBpm = floor(localBpm * kHistogramDecimalScale + 0.5) /
                kHistogramDecimalScale;
        tempoList << roundedBpm;
        tempoFrequencyTable[roundedBpm] += 1;
    }
    return std::make_tuple(tempoList, tempoFrequencyTable);
}

QMap<int, double> AnalyzerRhythm::findTempoChanges() {
    std::tie(m_rawTempos, m_rawTemposFrenquency) =
            computeRawTemposAndFrequency(m_resultBeats);
    auto sortedTempoList = m_rawTempos;
    std::sort(sortedTempoList.begin(), sortedTempoList.end());
    // we use the median as a guess first and last tempo
    const double median = BeatStatistics::computeSampleMedian(sortedTempoList);
    // The analyzer sometimes detect false beats that generate outliers
    // values for the tempo so we use a median filter to remove them
    MovingMedian filterTempo(kBeatsToCountTempo / 2 + 1); // odd samples for a precise median
    // The outliers values are eliminated by the median, but they might have made the tempo
    // to drift towards then, and thus we dont have a stable middle value, so we find the mode
    MovingMode stabilizeTempo(kBeatsToCountTempo * 2); // larger window to keep drift out
    int currentBeat = 0, lastBeatChange = 0;
    QMap<int, double> stableTemposAndPositions;
    stableTemposAndPositions[lastBeatChange] = median;
    // Here we are going to track the tempo changes over the track
    int nextDownbeatIndex = 0;
    double nextDownbeat = m_downbeats[nextDownbeatIndex];
    for (double tempo : m_rawTempos) {
        double newStableTempo = stabilizeTempo(filterTempo(tempo));
        // The analyzer has some jitter that causes a steady beat to fluctuate around
        // the correct value so we don't consider tempos +-1 from the median as changes
        if (newStableTempo == stableTemposAndPositions.last()) {
            ;
        } else if (stableTemposAndPositions.last() !=
                        m_rawTemposFrenquency.lastKey() and
                newStableTempo ==
                        (m_rawTemposFrenquency.find(
                                 stableTemposAndPositions.last()) +
                                1)
                                .key()) {
            ;
        } else if (stableTemposAndPositions.last() !=
                        m_rawTemposFrenquency.firstKey() and
                newStableTempo ==
                        (m_rawTemposFrenquency.find(
                                 stableTemposAndPositions.last()) -
                                1)
                                .key()) {
            ;
        } else {
            // this may not be case when our median window is even, we can't use it
            // because find will return an iterator pointing to end that we will *
            if (m_rawTemposFrenquency.contains(newStableTempo)) {
                // Our smallest unit of tempo change is a bar
                stableTemposAndPositions[static_cast<int>(nextDownbeat)] = newStableTempo;
            }
        }
        currentBeat += 1;
        if ((currentBeat - stabilizeTempo.lag() - filterTempo.lag()) >= nextDownbeat) {
            nextDownbeatIndex += 1;
            nextDownbeat = m_downbeats[nextDownbeatIndex];
        }
    }
    stableTemposAndPositions[m_rawTempos.count()] = median;
    return stableTemposAndPositions;
}

std::tuple<QVector<mixxx::audio::FramePos>, QMap<int, double>> AnalyzerRhythm::FixBeatsPositions() {
    m_stableTemposAndPositions = findTempoChanges();
    QVector<mixxx::audio::FramePos> fixedBeats;
    QMap<int, double> beatsWithNewTempo;
    // here we only care about where the change happened
    auto tempoChanges = m_stableTemposAndPositions.keys();
    for (int lastTempoChage = 0;
            lastTempoChage < tempoChanges.count() - 1;
            lastTempoChage++) {
        int beatStart = tempoChanges[lastTempoChage];
        int beatEnd = tempoChanges[lastTempoChage + 1];
        int partLenght = beatEnd - beatStart;
        // here we detect if the segment has a constant tempo or not
        double bpmDiff = 0;
        if (partLenght >= m_beatsPerBar * 2) {
            int middle = partLenght / 2;
            auto beatsAtLeft = QVector<mixxx::audio::FramePos>::fromStdVector(
                    std::vector<mixxx::audio::FramePos>(
                            m_resultBeats.begin() + beatStart,
                            m_resultBeats.begin() + beatStart + middle));
            auto beatsAtRight = QVector<mixxx::audio::FramePos>::fromStdVector(
                    std::vector<mixxx::audio::FramePos>(
                            m_resultBeats.begin() + beatStart + middle,
                            m_resultBeats.begin() + beatEnd));
            double beginBpm = medianTempo(beatsAtLeft);
            double endBpm = medianTempo(beatsAtRight);
            bpmDiff = fabs(beginBpm - endBpm);
        }
        // here we handle ramping or unsteady values by making a beatgrid for each bar
        if (bpmDiff >= kMaxBpmError) {
            while (beatStart <= beatEnd - m_beatsPerBar) {
                auto splittedAtMeasure =
                        QVector<mixxx::audio::FramePos>::fromStdVector(
                                std::vector<mixxx::audio::FramePos>(
                                        m_resultBeats.begin() + beatStart,
                                        m_resultBeats.begin() + beatStart +
                                                m_beatsPerBar));
                double measureBpm = medianTempo(splittedAtMeasure);
                beatsWithNewTempo[beatStart] = measureBpm;
                fixedBeats << calculateFixedTempoGrid(
                        splittedAtMeasure, measureBpm, false);
                beatStart += m_beatsPerBar;
            }
            int beatsLeftOut = beatEnd - beatStart;
            qDebug() << beatsLeftOut << beatStart;
        } else {
            // here we have only one bpm for whole segment
            auto splittedAtTempoChange =
                    QVector<mixxx::audio::FramePos>::fromStdVector(
                            std::vector<mixxx::audio::FramePos>(
                                    m_resultBeats.begin() + beatStart,
                                    m_resultBeats.begin() + beatEnd));
            double bpm = calculateBpm(splittedAtTempoChange);
            fixedBeats << calculateFixedTempoGrid(
                    splittedAtTempoChange, bpm, true);
            beatsWithNewTempo[beatStart] = bpm;
        }
    }
    return std::make_tuple(fixedBeats, beatsWithNewTempo);
}

QVector<mixxx::audio::FramePos> AnalyzerRhythm::calculateFixedTempoGrid(
        const QVector<mixxx::audio::FramePos>& rawbeats, const double localBpm, bool correctFirst) {
    if (rawbeats.size() == 0) {
        return rawbeats;
    }
    // Length of a beat at localBpm in mono samples.
    const double beat_length = floor(((60.0 * m_sampleRate) / localBpm) + 0.5);
    mixxx::audio::FramePos firstCorrectBeat = rawbeats.first();
    if (correctFirst) {
        firstCorrectBeat = findFirstCorrectBeat(rawbeats, localBpm);
    }
    // We start building a fixed beat grid at localBpm and the first beat from
    // rawbeats that matches localBpm.
    mixxx::audio::FramePos beatOffset = firstCorrectBeat;
    QVector<mixxx::audio::FramePos> fixedBeats;

    for (int i = 0; i < rawbeats.count(); i++) {
        fixedBeats << beatOffset;
        beatOffset += beat_length;
    }
    // Here we add the beats that were before our first "correct" beat to the grid
    beatOffset = firstCorrectBeat - beat_length;
    while (beatOffset > rawbeats.first()) {
        // this runs in O(n) for each call
        // TODO(Cristiano) Use QList instead of QVector?
        fixedBeats.prepend(beatOffset);
        beatOffset -= beat_length;
    }
    // qDebug() << fixedBeats;
    return fixedBeats;
}

mixxx::audio::FramePos AnalyzerRhythm::findFirstCorrectBeat(
        const QVector<mixxx::audio::FramePos> rawbeats, const double global_bpm) {
    for (int i = kBeatsToCountTempo; i < rawbeats.size(); i++) {
        // get start and end sample of the beats
        mixxx::audio::FramePos start_sample = rawbeats.at(i - kBeatsToCountTempo);
        mixxx::audio::FramePos end_sample = rawbeats.at(i);
        // The time in seconds represented by this sample range.
        double time = (end_sample - start_sample) / m_sampleRate;
        // Average BPM within this sample range.
        double avg_bpm = 60.0 * kBeatsToCountTempo / time;
        // If the local BPM is within kCorrectBeatLocalBpmEpsilon of the global
        // BPM then use this window as the first beat.
        if (fabs(global_bpm - avg_bpm) <= kCorrectBeatLocalBpmEpsilon) {
            // qDebug() << "Using beat " << (i-N) << " as first beat";
            return start_sample;
        }
    }
    // If we didn't find any beat that matched the window, return the first
    // beat.
    return !rawbeats.empty() ? rawbeats.first() : mixxx::audio::FramePos(0.0);
}

double AnalyzerRhythm::calculateBpm(const QVector<mixxx::audio::FramePos>& beats) {
    // Just to demonstrate how you would count the beats manually
    //    Beat numbers:   1  2  3  4   5  6  7  8    9
    //    Beat positions: ?  ?  ?  ?  |?  ?  ?  ?  | ?
    // Usually one measures the time of N beats. One stops the timer just before
    // the (N+1)th beat begins.  The BPM is then computed by 60*N/<time needed
    // to count N beats (in seconds)>
    if (beats.size() < 2) {
        return 0;
    }
    // If we don't have enough beats for our regular approach, just divide the #
    // of beats by the duration in minutes.
    if (beats.size() <= kBeatsToCountTempo) {
        return 60.0 * (beats.size() - 1) * m_sampleRate / (beats.last() - beats.first());
    }
    auto [tempoList, tempoFrequency] = computeRawTemposAndFrequency(beats, kBeatsToCountTempo);
    double median;
    // Okay, let's consider the median an estimation of the BPM. To not solely
    // rely on the median, we build the average weighted value of all bpm values
    // being at most +-1 BPM from the median away.
    median = medianTempo(beats);
    auto [filterWeightedAverageBpm, filtered_bpm_frequency_table] =
            computeFilteredWeightedAverage(tempoFrequency, median);
    if (sDebug) {
        qDebug() << "Statistical median BPM: " << median;
        qDebug() << "Weighted Avg of BPM values +- 1BPM from the media"
                 << filterWeightedAverageBpm;
    }
    // Although we have a minimal deviation of about +- 0.05 BPM units compared
    // to Traktor, this deviation may cause the beat grid to look unaligned,
    // especially at the end of a track.  Let's try to get the BPM 'perfect' :-)

    // Idea: Iterate over the original beat set where some detected beats may be
    // wrong. The beat is considered 'correct' if the beat position is within
    // epsilon of a beat grid obtained by the global BPM.

    // If the beat turns out correct, we can compute the error in BPM units.
    // E.g., we can check the original beat position after 60 seconds. Ideally,
    // the approached beat is just a couple of samples away, i.e., not worse
    // than 0.05 BPM units.  The distance between these two samples can be used
    // for BPM error correction
    double perfect_bpm = 0;
    mixxx::audio::FramePos firstCorrectBeatSample = beats.first();
    bool foundFirstCorrectBeat = false;
    int counter = 0;
    int perfectBeats = 0;
    for (int i = kBeatsToCountTempo; i < beats.size(); i += 1) {
        // get start and end sample of the beats
        mixxx::audio::FramePos beat_start = beats.at(i - kBeatsToCountTempo);
        mixxx::audio::FramePos beat_end = beats.at(i);
        // Time needed to count a bar (N beats)
        double time = (beat_end - beat_start) / m_sampleRate;
        if (time == 0)
            continue;
        double local_bpm = 60.0 * kBeatsToCountTempo / time;
        // round BPM to have two decimal places
        local_bpm = floor(local_bpm * kHistogramDecimalScale + 0.5) / kHistogramDecimalScale;
        // qDebug() << "Local BPM beat " << i << ": " << local_bpm;
        if (!foundFirstCorrectBeat and
                filtered_bpm_frequency_table.contains(local_bpm) and
                fabs(local_bpm - filterWeightedAverageBpm) < kMaxBpmError) {
            firstCorrectBeatSample = beat_start;
            foundFirstCorrectBeat = true;
            if (sDebug) {
                qDebug() << "Beat #" << (i - kBeatsToCountTempo)
                         << "is considered as reference beat with BPM:"
                         << local_bpm;
            }
        }
        if (foundFirstCorrectBeat) {
            if (counter == 0) {
                counter = kBeatsToCountTempo;
            } else {
                counter += 1;
            }
            double time2 = (beat_end - firstCorrectBeatSample) / m_sampleRate;
            double correctedBpm = 60 * counter / time2;
            if (fabs(correctedBpm - filterWeightedAverageBpm) <= kMaxBpmError) {
                perfect_bpm += correctedBpm;
                perfectBeats += 1;
                if (sDebug) {
                    qDebug() << "Beat #" << (i - kBeatsToCountTempo)
                             << "is considered as correct -->BPM improved to:"
                             << correctedBpm;
                }
            }
        }
    }
    const double perfectAverageBpm = perfectBeats > 0
            ? perfect_bpm / perfectBeats
            : filterWeightedAverageBpm;
    // Round values that are within BPM_ERROR of a whole number.
    const double rounded_bpm = floor(perfectAverageBpm + 0.5);
    const double bpm_diff = fabs(rounded_bpm - perfectAverageBpm);
    bool perform_rounding = (bpm_diff <= kMaxBpmError);
    // Finally, restrict the BPM to be within min_bpm and max_bpm.
    const double maybeRoundedBpm = perform_rounding ? rounded_bpm : perfectAverageBpm;
    if (sDebug) {
        qDebug() << "SampleMedianBpm=" << median;
        qDebug() << "FilterWeightedAverageBpm=" << filterWeightedAverageBpm;
        qDebug() << "Perfect BPM=" << perfectAverageBpm;
        qDebug() << "Rounded Perfect BPM=" << rounded_bpm;
        qDebug() << "Rounded difference=" << bpm_diff;
        qDebug() << "Perform rounding=" << perform_rounding;
    }
    return maybeRoundedBpm;
}

std::tuple<double, QMap<double, int>> AnalyzerRhythm::computeFilteredWeightedAverage(
        const QMap<double, int>& tempoFrequency, const double filterCenter) {
    double avarageAccumulator = 0.0;
    int totalWeight = 0;
    QMap<double, int> filteredTempoFrequency;
    QMapIterator<double, int> tempoIterator(tempoFrequency);

    while (tempoIterator.hasNext()) {
        tempoIterator.next();
        double tempo = tempoIterator.key();
        int frequency = tempoIterator.value();

        if (fabs(tempo - filterCenter) <= kBpmFilterTolerance) {
            // TODO(raffitea): Why > 1 ?
            if (frequency > 1) {
                totalWeight += frequency;
                avarageAccumulator += tempo * frequency;
                filteredTempoFrequency[tempo] = frequency;
                if (sDebug) {
                    qDebug() << "Filtered Table:" << tempo
                             << "Frequency:" << frequency;
                }
            }
        }
    }
    if (sDebug) {
        qDebug() << "Sum of filtered frequencies: " << totalWeight;
    }
    double filteredWeightedAverage;
    if (totalWeight == 0) {
        filteredWeightedAverage = filterCenter;
    } else {
        filteredWeightedAverage = avarageAccumulator / static_cast<double>(totalWeight);
    }
    return std::make_tuple(filteredWeightedAverage, filteredTempoFrequency);
}
