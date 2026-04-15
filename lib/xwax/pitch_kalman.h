/*
 * Kalman filter pitch estimator with sensitivity mode switches
 *
 * Model: Constant velocity (x, v) with acceleration process noise
 *
 * State:
 *   x : Position (s) relative to the predicted trajectory
 *   v : Velocity (dimensionless) relative to the carrier frequency of the timecode (normalized)
 *
 * Assumption: We steadily advance with a constant velocity
 *
 *   dx: The displacement of the position by which we advanced (s).
 *       When crossing a zero this is 1/4 of cycle of the sinusoid
 *       (π/2) quantized to the resolution of the timecode.
 *
 *       When not crossing a zero, a displacement of 0.0 is passed,
 *       which signifies no movement and lets the velocity decay slowly
 *       until the next zero crossing is encountered.
 *
 * Observation:
 *   z = dx (new measurement after the interval dt)
 *
 * Modes:
 *   stable  : Low Q and high R for stable playback
 *   adjust  : Medium values for slight pitch changes
 *   reactive: High Q and low R for high reactivity (scratching)
 */

#ifndef PITCH_KALMAN_H
#define PITCH_KALMAN_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

/* Struct for the Kalman filter coefficients R and Q */

struct kalman_coeffs {
    double Q;
    double R;
};

#define KALMAN_COEFFS(q_arg, r_arg) \
    (struct kalman_coeffs) {.Q = (q_arg), .R = (r_arg)}

struct pitch_kalman {
    /*
     * NOTE: In discrete time dt is usually denoted as Ts = 1/Fs,
     * but xwax uses the continuous notation.
     */

    double dt; /* Sampling interval: (s) */

    double Xk[2]; /* Position and velocity state space */

    /* Covariance P (2x2, symmetric) */

    double P[2][2];

    /* Thresholds of the innovation quantity for the mode switches */

    double scratch_threshold, reactive_threshold, adjust_threshold;

    /* Currently used coefficients*/

    struct kalman_coeffs* coeffs;

    /* Stable, adjust reactive coefficients for the mode switch */

    struct kalman_coeffs stable;
    struct kalman_coeffs adjust;
    struct kalman_coeffs reactive;
    struct kalman_coeffs scratch;
};

void pitch_kalman_init(struct pitch_kalman *p, double dt, struct kalman_coeffs stable,
                       struct kalman_coeffs adjust, struct kalman_coeffs reactive, struct kalman_coeffs scratch,
                       double adjust_threshold, double reactive_threshold, double scratch_threshold, bool debug);
void pitch_kalman_update(struct pitch_kalman *p, double dx);

/*
 * Retune noise sensitivity without resetting state
 */

static inline void kalman_tune_sensitivity(struct pitch_kalman* p, struct kalman_coeffs* coeffs)
{
    if (!p || !coeffs) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    p->coeffs = coeffs;
}


/*
 * Get the current pitch (velocity estimate)
 */

static inline double pitch_kalman_current(const struct pitch_kalman* p)
{
    if (!p) {
        errno = EINVAL;
        perror(__func__);
        return 0.0;
    }

    /* Return the velocity Xk[v] relative to carrier frequency of the timecode (normalized) */

    return p->Xk[1];
}

#endif /* PITCH_KALMAN_H */
