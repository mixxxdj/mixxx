/*
 * Kalman filter pitch estimator with sensitivity mode switches
 *
 * Model: Constant velocity (x, v) with acceleration process noise
 *
 * State:
 *   x: The current position on the carrier (sinusoid)
 *   v: Velocity (pitch)
 *
 * Observation:
 *   z = dx (new measurement after the interval dt)
 *
 * Modes:
 *   stable  : Low Q and high R for stable playback
 *   medium  : Medium values for slight pitch changes
 *   reactive: High Q and low R for high reactivity (scratching)
 */

#ifndef PITCH_KALMAN_H
#define PITCH_KALMAN_H

#include <errno.h>
#include <math.h>
#include <stdio.h>


/* Struct for the Kalman filter coefficients R and Q */

struct kalman_coeffs {
    double q;
    double r;
};

#define KALMAN_COEFFS(q_arg, r_arg) \
    (struct kalman_coeffs) {.q = (q_arg), .r = (r_arg)}

struct pitch_kalman {

    /*
     * NOTE: In discrete time dt is usually denoted as Ts = 1/Fs,
     * but xwax uses the continuous notation.
     */

    double dt; /* Sampling interval: (s) */

    double x; /* Position (relative; we subtract dx each step) */
    double v; /* Velocity (pitch) */

    /* Covariance P (2x2, symmetric) */
    double Pxx, Pxv, Pvv;

    /* Tresholds of the innovation quantity for the mode switches */
    double scratch_threshold, medium_threshold;

    /* Currently used coefficients*/
    struct kalman_coeffs* coeffs;

    /* Stable, medium reactive coefficients for the mode switch */
    struct kalman_coeffs stable;
    struct kalman_coeffs medium;
    struct kalman_coeffs reactive;
};

/*
 * Retune noise sensitivity without resetting state
 */

static inline void kalman_tune_sensitivity(struct pitch_kalman* p, struct kalman_coeffs* coeffs) {
    if (!p || !coeffs) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    p->coeffs = coeffs;
}

/*
 * Initialize the filter with dt, the coefficients and thresholds for
 * the different mode switches
 *
 * q: acceleration noise spectral density (tune up if motion is more jittery)
 * r: variance of dx measurement (tune up if observations are noisier)
 */

static inline void pitch_kalman_init(struct pitch_kalman* p, double dt,
        struct kalman_coeffs stable, struct kalman_coeffs medium, struct kalman_coeffs reactive,
        double medium_threshold, double scratch_threshold)
{
    if (!p || medium_threshold > scratch_threshold) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    p->dt = dt;

    /* State */
    p->x = 0.0;
    p->v = 0.0;

    /* Start with large uncertainty so the filter trusts early measurements */
    p->Pxx = 1e6;
    p->Pxv = 0.0;
    p->Pvv = 1e6;

    /* Fixed thresholds for the mode switches */
    p->scratch_threshold = scratch_threshold;
    p->medium_threshold = medium_threshold;

    /* Q and R for the different modes */
    p->stable = stable;
    p->medium = medium;
    p->reactive = reactive;

    /* Initialize as reactive */
    kalman_tune_sensitivity(p, &p->reactive);
}

/*
 * Feed one observation: in the last dt seconds, position moved by dx
 */

static inline void pitch_kalman_update(struct pitch_kalman* p, double dx) {

    if (!p) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    const double dt = p->dt;

    /*
     * ==== Predict step ====
     * State transition F = [1 dt; 0 1]
     * Process noise for constant-velocity model with white accel (G * q * G^T):
     *   Q = q * [dt^3/3  dt^2/2;
     *            dt^2/2  dt]
     */

    const double F[2][2] = { {1.0, dt }, {0.0, 1.0} };

    const double dt2 = dt * dt;
    const double dt3 = dt2 * dt;

    const double q11 = p->coeffs->q * (dt3 / 3.0);
    const double q12 = p->coeffs->q * (dt2 / 2.0);
    const double q22 = p->coeffs->q * (dt);

    /* Predict state */
    const double x_pred = p->x + p->v * dt;
    const double v_pred = p->v;

    /*
     * Predict covariance: P = F * P * F^T + Q
     * Compute with minimal temporaries and keep symmetry
     */

    const double Pxx = p->Pxx;
    const double Pxv = p->Pxv;
    const double Pvv = p->Pvv;

    const double Pxx_pred = F[0][0] * (F[0][0] * Pxx + F[0][1] * Pxv) + F[0][1] * (F[0][0] * Pxv + F[0][1] * Pvv) + q11;
    const double Pxv_pred = F[1][0] * (F[0][0] * Pxx + F[0][1] * Pxv) + F[1][1] * (F[0][0] * Pxv + F[0][1] * Pvv) + q12;
    const double Pvv_pred = F[1][0] * (F[1][0] * Pxx + F[1][1] * Pxv) + F[1][1] * (F[1][0] * Pxv + F[1][1] * Pvv) + q22;

    /*
     * ==== Update step ====
     * Measurement z = H x + noise, with H = [1 0], R = r
     * Innovation: y = z - H * x_pred = dx - x_pred
     */

    const double Hx = 1.0, Hv = 0.0;
    const double y = dx - (Hx * x_pred + Hv * v_pred);

    /*
     * Toggle mode switches for stable playback and scratching. When predicted
     * position and measured position differ 
     */

    const double y_abs = fabs(y);

    if (y_abs > p->scratch_threshold)
        kalman_tune_sensitivity(p, &p->reactive);
    else if (y_abs > p->medium_threshold && y_abs < p->scratch_threshold)
        kalman_tune_sensitivity(p, &p->medium);
    else
        kalman_tune_sensitivity(p, &p->stable);

    /* Innovation covariance: S = H P H^T + R = Pxx_pred + r */
    const double S = Pxx_pred + p->coeffs->r;

    /* Kalman gain: K = P H^T S^-1 = [Kx; Kv] */
    const double invS = 1.0 / S;
    const double Kx = Pxx_pred * invS;
    const double Kv = Pxv_pred * invS;

    /* Update state */
    double x_upd = x_pred + Kx * y;
    const double v_upd = v_pred + Kv * y;

    /*
     * Update covariance: P = (I - K H) P
     * With H = [1 0], (I-KH) = [[1-Kx,   0],
     *                           [-Kv,    1]]
     * Compute symmetric entries explicitly
     */

    const double Pxx_upd = (1.0 - Kx) * Pxx_pred;
    const double Pxv_upd = (1.0 - Kx) * Pxv_pred; /* equals Pvx_upd */
    const double Pvv_upd = Pvv_pred - Kv * Pxv_pred;

    /* Keep the same “relative position” convention the α-β filter */
    x_upd -= dx;

    /* Store back */
    p->x = x_upd;
    p->v = v_upd;
    p->Pxx = Pxx_upd;
    p->Pxv = Pxv_upd;
    p->Pvv = Pvv_upd;
}

/*
 * Get the current pitch (velocity estimate)
 */

static inline double pitch_kalman_current(const struct pitch_kalman* p) {

    if (!p) {
        errno = EINVAL;
        perror(__func__);
        return 0.0;
    }

    return p->v;
}

#endif /* PITCH_KALMAN_H */
