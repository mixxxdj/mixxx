#include "util/rotary.h"

#include <QtDebug>

const int kiRotaryFilterMaxLen = 50;

Rotary::Rotary()
    : m_iFilterPos(0),
      m_dCalibration(1.0),
      m_dLastValue(0.0),
      m_iCalibrationCount(0) {
    m_iFilterLength = kiRotaryFilterMaxLen;
    m_pFilter = new double[m_iFilterLength];
    for (int i = 0; i < m_iFilterLength; ++i) {
        m_pFilter[i] = 0.;
    }
}

Rotary::~Rotary() {
    delete [] m_pFilter;
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
double Rotary::filter(double dValue) {
    // Update filter buffer
    m_pFilter[m_iFilterPos] = dValue/m_dCalibration;
    m_iFilterPos = (m_iFilterPos+1)%m_iFilterLength;

    double dMagnitude = 0.;
    for (int i=0; i<m_iFilterLength; i++)
    {
        dMagnitude += m_pFilter[i];
    }
    dMagnitude /= (double)m_iFilterLength;
    //qDebug() << "filter in " << dValue << ", out " << dMagnitude;

    m_dLastValue = dMagnitude;

    return dMagnitude;
}

double Rotary::fillBuffer(double dValue) {
    for (int i=0; i<m_iFilterLength; ++i)
    {
        m_pFilter[i] = dValue/m_dCalibration;
    }
    return dValue/m_dCalibration;
}

void Rotary::calibrate(double dValue) {
    m_dCalibration += dValue;
    m_iCalibrationCount += 1;
}

void Rotary::calibrateStart() {
    // Reset calibration data
    m_dCalibration = 0.;
    m_iCalibrationCount = 0;
}

double Rotary::calibrateEnd() {
    m_dCalibration /= (double)m_iCalibrationCount;

    qDebug() << "Calibration " << m_dCalibration << ", count " << m_iCalibrationCount;

    return m_dCalibration;
}

void Rotary::setCalibration(double c) {
    m_dCalibration = c;
}

double Rotary::getCalibration() {
    return m_dCalibration;
}

void Rotary::setFilterLength(int i) {
    if (i > kiRotaryFilterMaxLen) {
        m_iFilterLength = kiRotaryFilterMaxLen;
    } else if (i < 1) {
        m_iFilterLength = 1;
    } else {
        m_iFilterLength = i;
    }
}

int Rotary::getFilterLength() {
    return  m_iFilterLength;
}
