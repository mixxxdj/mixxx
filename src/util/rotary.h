#pragma once

class Rotary {
  public:
    Rotary();
    ~Rotary();

    // Start calibration measurement
    void calibrateStart();
    // End calibration measurement
    double calibrateEnd();
    // Set calibration
    void setCalibration(double c);
    // Get calibration
    double getCalibration();
    // Low pass filtered rotary event
    double filter(double dValue);
    // Hard set event value
    double fillBuffer(double dValue);
    // Collect calibration data
    void calibrate(double dValue);
    // Set filter length
    void setFilterLength(int i);
    // Get filter length
    int getFilterLength();

  protected:
    // Length of filter
    int m_iFilterLength;
    // Update position in filter
    int m_iFilterPos;
    // Pointer to rotary filter buffer
    double *m_pFilter;
    // Calibration value
    double m_dCalibration;
    // Last value
    double m_dLastValue;
    int m_iCalibrationCount;
};
