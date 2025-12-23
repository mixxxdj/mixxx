#include "util/rotary.h"

#include <numeric>

Rotary::index_type Rotary::nextIndex(Rotary::index_type i) const {
    if (++i >= m_filterHistory.size()) {
        return 0;
    }
    return i;
}

void Rotary::append(double v) {
    m_headIndex = nextIndex(m_headIndex);
    m_filterHistory[m_headIndex] = v;
}

double Rotary::calculate() const {
    return std::accumulate(std::cbegin(m_filterHistory),
                   std::cend(m_filterHistory),
                   0.0) /
            static_cast<double>(m_filterHistory.size());
}

/* Note: There's probably a bug in this function (or this class) somewhere.
    The filter function seems to be the cause of the "drifting" bug in the Hercules stuff.
    What happens is that filter() gets called to do some magic to a value that's returned
    from the Hercules device, and that magic adds "momentum" to it's motion (ie. it doesn't
    stop dead when you stop spinning the jog wheels.) The problem with this "magic" is that
    when herculeslinux.cpp passes the filtered value off to the wheel ControlObject (or what
    have you), the ControlObject's internal value never goes back to zero properly.
    - Albert (March 13, 2007)
*/
double Rotary::filter(double v) {
    append(v);
    return calculate();
}
