/***************************************************************************
                        alphabetafilter.h  -  description
                        -------------------
begin                : Wed May 12 2010
copyright            : (C) 2010 by Sean M. Pappalardo
email                : spappalardo@mixxx.org

This is essentially just a C++ version of xwax's pitch.h,
    which is Copyright (C) 2010 Mark Hills <mark@pogo.org.uk>
***************************************************************************/

#ifndef ALPHABETAFILTER_H
#define ALPHABETAFILTER_H

// This is a simple alpha-beta filter. It follows the example from Wikipedia
// closely: http://en.wikipedia.org/wiki/Alpha_beta_filter
//
// Given an initial position and velocity, learning parameters alpha and beta,
// and a series of input observations of the distance travelled, the filter
// predicts the real position and velocity.
class AlphaBetaFilter {
  public:
    AlphaBetaFilter()
            : m_initialized(false),
              m_dt(0.0),
              m_x(0.0),
              m_v(0.0),
              m_alpha(0.0),
              m_beta(0.0) {
    }

    // Prepare the filter for observations every dt seconds. Default filter
    // values were concluded experimentally for time code vinyl.
    void init(double dt, double v, double alpha = 1.0/512, double beta = (1.0/512)/1024) {
        m_initialized = true;
        m_dt = dt;
        m_x = 0.0;
        m_v = v;
        m_alpha = alpha;
        m_beta = beta;
    }

    // Input an observation to the filter; in the last dt seconds the position
    // has moved by dx.
    //
    // Because the values come from a digital controller, the values for dx are
    // discrete rather than smooth.
    void observation(double dx) {
        if (!m_initialized) {
            return;
        }

        double predicted_x = m_x + m_v * m_dt;
        double predicted_v = m_v;
        double residual_x = dx - predicted_x;

        m_x = predicted_x + residual_x * m_alpha;
        m_v = predicted_v + residual_x * m_beta / m_dt;

        // relative to previous
        m_x -= dx;
    }

    // Get the velocity after filtering.
    double predictedVelocity() const {
        return m_v;
    }

    // Get the position after filtering.
    double predictedPosition() const {
        return m_x;
    }

  private:
    // Whether init() has been called.
    bool m_initialized;
    // State of the rate calculation filter
    double m_dt, m_x, m_v, m_alpha, m_beta;
};

#endif
