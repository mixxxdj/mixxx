/***************************************************************************
                        pitchfilter.h  -  description
                        -------------------
begin                : Wed May 12 2010
copyright            : (C) 2010 by Sean M. Pappalardo
email                : spappalardo@mixxx.org

This is essentially just a C++ version of xwax's pitch.h,
    which is Copyright (C) 2010 Mark Hills <mark@pogo.org.uk>
***************************************************************************/

#ifndef PITCHFILTER_H
#define PITCHFILTER_H

class PitchFilter {
    public:
        /** Prepare the filter for observations every dt seconds
        * Default filter values were concluded experimentally for time code vinyl */
        void init(float dt, float v, float alpha = 1.0/512, float beta = (1.0/512)/1024) {
            m_dt = dt;
            m_x = 0.0;
            m_v = v;
            m_alpha = alpha;
            m_beta = beta;
        }

        /** Input an observation to the filter; in the last dt seconds the
        * position has moved by dx.
        *
        * Because the values come from a digital controller,
        *   the values for dx are discrete rather than smooth. */
        void observation(float dx) {
            float predicted_x, predicted_v, residual_x;

            predicted_x = m_x + m_v * m_dt;
            predicted_v = m_v;

            residual_x = dx - predicted_x;

            m_x = predicted_x + residual_x * m_alpha;
            m_v = predicted_v + residual_x * (m_beta / m_dt);

            m_x -= dx; /* relative to previous */
        }

        /** Get the pitch after filtering */
        float currentPitch() {
            return m_v;
        }

    protected:
        /** State of the pitch calculation filter */
        float m_dt, m_x, m_v, m_alpha, m_beta;
};

#endif
