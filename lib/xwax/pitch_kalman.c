#include <math.h>

#include "pitch_kalman.h"

static bool kalman_debug_state = false;

#define kalman_debug(...)          \
    {                                     \
        if (kalman_debug_state) {                      \
            fprintf(stderr, __VA_ARGS__); \
        }                                 \
    }


enum { x = 0, v = 1 }; /* Matrix indices: x = position and v = velocity */

static inline double pow2(double val)
{
    return (val * val);
}

static inline double pow3(double val)
{
    return (val * val * val);
}

/*
 * Initialize the filter with dt, the coefficients and thresholds for
 * the different mode switches
 *
 * q: acceleration noise spectral density (tune up if motion is more jittery)
 * r: variance of dx measurement (tune up if observations are noisier)
 */

void pitch_kalman_init(struct pitch_kalman *p, double dt, struct kalman_coeffs stable,
                       struct kalman_coeffs adjust, struct kalman_coeffs reactive,
                       struct kalman_coeffs scratch, double adjust_threshold,
                       double reactive_threshold, double scratch_threshold, bool debug)
{
    const bool thresholds_well_ordered = scratch_threshold > reactive_threshold && reactive_threshold > adjust_threshold;
    if (!p || !thresholds_well_ordered) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    kalman_debug_state = debug;

    /* Sampling interval */

    p->dt = dt;

    /* State space (position and velocity)*/

    p->Xk[x] = 0.0;
    p->Xk[v] = 0.0;

    /*
     * Start with large uncertainty so the filter trusts early measurements.
     *
     * NOTE: P[v][x] is unused and therefore not initialized
     */

    p->P[x][x] = 1e6;
    p->P[x][v] = 0.0;
    p->P[v][v] = 1e6;

    /* Fixed thresholds for the mode switches */

    p->scratch_threshold = scratch_threshold;
    p->reactive_threshold = reactive_threshold;
    p->adjust_threshold = adjust_threshold;

    /* Q and R for the different modes */

    p->stable = stable;
    p->adjust = adjust;
    p->reactive = reactive;
    p->scratch = scratch;

    /* Initialize as reactive */

    kalman_tune_sensitivity(p, &p->reactive);
}

/*
 * Feed one observation: in the last dt seconds, position moved by dx
 */

void pitch_kalman_update(struct pitch_kalman* p, double dx)
{
    if (!p) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    /* Sampling interval */

    const double dt = p->dt;
    /*

     * =================== Prediction step =====================
     *
     * State transition for constant-velocity model:
     *   F = [ 1  dt
     *         0  1 ]
     *
     * Discretizing, the process noise covariance is:
     *   Q = q x [ dt^3/3  dt^2/2
     *             dt^2/2  dt     ]
     *
     * Here p->coeffs->Q is 'q', the acceleration noise variance.
     *
     * Q[v][x] is not needed for the computations, therefore we put 0.0.
     */

    const double F[2][2] = { { 1.0,   dt },
                             { 0.0,  1.0 }};

    const double Q[2][2] = { { p->coeffs->Q * (pow3(dt) / 3.0),  p->coeffs->Q * (pow2(dt) / 2.0) },
                             {                             0.0,  p->coeffs->Q * dt               }};

    /* Predict state */

    const double X_pred[2] = { p->Xk[x] + p->Xk[v] * dt, p->Xk[v] };

    /*
     * Predict covariance: P_pred = F x P x F^T + Q
     * For 2×2 P and F, expand each term explicitly:
     *
     *   P00_pred = F00 * (F00 * P00 + F01 * P01) + F01 * (F00 * P01 + F01 * P11) + Q00
     *   P01_pred = F10 * (F00 * P00 + F01 * P01) + F11 * (F00 * P01 + F01 * P11) + Q01
     *   P11_pred = F10 * (F10 * P00 + F11 * P01) + F11 * (F10 * P01 + F11 * P11) + Q11
     *
     * This preserves symmetry while avoiding full matrix multiplication.
     */

    const double P[2][2] = { {p->P[x][x], p->P[x][v]},
                             {       0.0, p->P[v][v]} };

    double P_pred[2][2];

    P_pred[x][x] = F[x][x] * (F[x][x] * P[x][x] + F[x][v] * P[x][v]) +
                   F[x][v] * (F[x][x] * P[x][v] + F[x][v] * P[v][v]) + Q[x][x];

    P_pred[x][v] = F[v][x] * (F[x][x] * P[x][x] + F[x][v] * P[x][v]) +
                   F[v][v] * (F[x][x] * P[x][v] + F[x][v] * P[v][v]) + Q[x][v];

    P_pred[v][v] = F[v][x] * (F[v][x] * P[x][x] + F[v][v] * P[x][v]) +
                   F[v][v] * (F[v][x] * P[x][v] + F[v][v] * P[v][v]) + Q[v][v];

    /*
     * =================== Update step =======================
     *
     *   Measurement model: z = Hx + noise
     *   Here H = [1 0], so z = x + noise
     *
     *   Innovation (residual): y = z - H * X_pred[x]
     *   Since Hx = 1, Hv = 0, this reduces to: y = dx - X_pred[x]
     */

    const double y = dx - X_pred[x];

    /*
     * Toggle mode switches for stable playback and scratching. When the
     * innovation quantity hits a certain treshold the filter sensitivity
     * is tuned.
     *
     * The innovation is similar to the residual in the alpha-beta filter,
     * calculated as
     *
     *          residual = measurement - prediction
     *
     * It represents how much the actual measurement deviates from the
     * predicted value.
     *
     * NOTE: The innovation is a noisy quantity. The switches have been worked
     *       out to ensure proper playback. If we should ever decide to use a
     *       continuous curve instead of hard switches, we should probably
     *       smooth the innovation before.
     */

    const double y_abs = fabs(y);

    kalman_debug("innovation: %+f, ", y);
    if (y_abs > p->scratch_threshold) {
        kalman_debug("                                                SCRATCH MODE\n");
        kalman_tune_sensitivity(p, &p->scratch);
    } else if (y_abs > p->reactive_threshold) {
        kalman_debug("                               REACTIVE MODE\n");
        kalman_tune_sensitivity(p, &p->reactive);
    } else if (y_abs > p->adjust_threshold) {
        kalman_debug("                ADJUST MODE\n");
        kalman_tune_sensitivity(p, &p->adjust);
    } else {
        kalman_debug("STABLE MODE\n");
        kalman_tune_sensitivity(p, &p->stable);
    }

    /* Ensure reactivity and quick decay after standstill */

    if (fabs(p->Xk[v]) < 5e-2) {
        kalman_tune_sensitivity(p, &p->scratch);
    }

    /*
     * Innovation covariance: S = H * P_pred * H^T + R
     * With H = [1 0], this reduces to: S = P_pred[x][x] + R
     */

    const double S = P_pred[x][x] + p->coeffs->R;

    /*
     * Kalman gain: K = P_pred * H^T * S^-1
     *
     * With H = [1 0]:
     *   K[x] = P_pred[x][x] / S
     *   K[v] = P_pred[x][v] / S
     */

    const double invS = 1.0 / S;
    const double K[2] = {P_pred[x][x] * invS, P_pred[x][v] * invS};

    /*
     * Update state:
     *
     * X_upd[x] = X_pred[x] + K[x] * y
     * X_upd[v] = X_pred[v] + K[v] * y
     */

    double X_upd[2] = {X_pred[x] + K[x] * y, X_pred[v] + K[v] * y};

    /*
     * Update covariance: P = (I - K H) * P_pred
     *
     * H = [1 0], so:
     *
     *   I - K x H = [ 1-K[x]  0
     *                  -K[v]  1 ]
     */

    double P_upd[2][2];

    P_upd[x][x] = (1.0 - K[x]) * P_pred[x][x];
    P_upd[x][v] = (1.0 - K[x]) * P_pred[x][v]; /* equals P_upd[x] */
    P_upd[v][v] = P_pred[v][v] - K[v] * P_pred[x][v];

    /* Keep the same “relative position” convention as the Alpha-Beta filter */

    X_upd[x] -= dx;

    /* Store updated state and covariance */

    p->Xk[x] = X_upd[x];
    p->Xk[v] = X_upd[v];

    p->P[x][x] = P_upd[x][x];
    p->P[x][v] = P_upd[x][v];
    p->P[v][v] = P_upd[v][v];
}
