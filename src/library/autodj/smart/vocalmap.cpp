#include "library/autodj/smart/vocalmap.h"

#include <algorithm>
#include <cmath>

namespace vocalmap {
namespace {

/// Moving average over `window` values (centred).
std::vector<double> smooth(const std::vector<float>& values, int window) {
    const int n = static_cast<int>(values.size());
    std::vector<double> sums(n + 1, 0.0);
    for (int i = 0; i < n; ++i) {
        sums[i + 1] = sums[i] + std::max(0.0f, values[i]);
    }
    std::vector<double> out(n, 0.0);
    const int half = window / 2;
    for (int i = 0; i < n; ++i) {
        const int a = std::max(0, i - half);
        const int b = std::min(n, i + half + 1);
        out[i] = (sums[b] - sums[a]) / std::max(1, b - a);
    }
    return out;
}

double percentile(std::vector<double> values, double fraction) {
    if (values.empty()) {
        return 0.0;
    }
    const auto k = static_cast<std::size_t>(
            std::clamp(fraction, 0.0, 1.0) * static_cast<double>(values.size() - 1));
    std::nth_element(values.begin(), values.begin() + k, values.end());
    return values[k];
}

} // namespace

std::vector<Section> find(const std::vector<float>& vocal,
        const std::vector<float>& rest,
        double rate,
        const Settings& settings) {
    std::vector<Section> sections;
    const int n = static_cast<int>(vocal.size());
    if (n == 0 || !(rate > 0.0)) {
        return sections;
    }
    const int window = std::max(1, static_cast<int>(std::lround(settings.smoothSec * rate)));
    const std::vector<double> voice = smooth(vocal, window);
    const bool haveRest = rest.size() == vocal.size();
    const std::vector<double> band = haveRest ? smooth(rest, window) : std::vector<double>();

    const double floor = percentile(voice, 0.2);
    const double peak = percentile(voice, 0.95);
    if (peak < settings.minPeak || peak <= floor) {
        return sections; // an instrumental (or a silent vocal part)
    }
    const double threshold = floor + settings.level * (peak - floor);

    // Runs of singing.
    int start = -1;
    for (int i = 0; i <= n; ++i) {
        const bool singing = i < n && voice[i] >= threshold &&
                (!haveRest || voice[i] >= settings.leakRatio * band[i]);
        if (singing && start < 0) {
            start = i;
        } else if (!singing && start >= 0) {
            sections.push_back(Section{start / rate, i / rate});
            start = -1;
        }
    }
    // Breaths and short rests: one section.
    std::vector<Section> joined;
    for (const Section& s : sections) {
        if (!joined.empty() && s.startSec - joined.back().endSec < settings.joinGapSec) {
            joined.back().endSec = s.endSec;
        } else {
            joined.push_back(s);
        }
    }
    // Tiny bits are not singing.
    joined.erase(std::remove_if(joined.begin(),
                         joined.end(),
                         [&settings](const Section& s) {
                             return s.endSec - s.startSec < settings.minSectionSec;
                         }),
            joined.end());
    return joined;
}

bool singsBetween(const std::vector<Section>& sections, double fromSec, double toSec) {
    return std::any_of(sections.begin(), sections.end(), [fromSec, toSec](const Section& s) {
        return s.startSec < toSec && s.endSec > fromSec;
    });
}

double quietMoment(const std::vector<Section>& sections, double fromSec, double toSec) {
    double t = fromSec;
    for (const Section& s : sections) {
        if (s.endSec <= t) {
            continue;
        }
        if (s.startSec > t) {
            break; // t is between two sections: quiet
        }
        t = s.endSec; // inside a section: wait for its end
    }
    return t <= toSec ? t : -1.0;
}

double nextSinging(const std::vector<Section>& sections, double fromSec) {
    for (const Section& s : sections) {
        if (s.endSec > fromSec) {
            return std::max(s.startSec, fromSec);
        }
    }
    return -1.0;
}

} // namespace vocalmap
