#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "timecoder_mk2.h"

#define REF_PEAKS_AVG 48 /* in wave cycles */

/*
 * Compute the LFSR bit (Traktor MK2 version)
 */

static inline mk2bits_t lfsr_mk2(mk2bits_t code, mk2bits_t taps)
{
    mk2bits_t taken;
    mk2bits_t xrs;

    taken = u128_and(code, taps);
    xrs = U128_ZERO;

    while (u128_neq(taken, U128_ZERO)) {
        xrs = u128_add(xrs, u128_and(taken, U128_ONE));
        taken = u128_rshift(taken, 1);
    }

    return u128_and(xrs, U128_ONE);
}

/*
 * Linear Feedback Shift Register in the forward direction. New values
 * are generated at the least-significant bit. (Traktor MK2 version)
 */

inline mk2bits_t fwd_mk2(mk2bits_t current, struct timecode_def *def)
{
    if (!def) {
        errno = -EINVAL;
        perror(__func__);
        return U128_ZERO;
    }

    mk2bits_t l;

    /* New bits are added at the MSB; shift right by one */
    l = lfsr_mk2(current, u128_or(def->taps_mk2,  U128_ONE));
    return u128_or(u128_rshift(current, 1), u128_lshift(l, (def->bits - 1)));
}

/*
 * Linear Feedback Shift Register in the reverse direction
 * (Traktor MK2 version)
 */

inline mk2bits_t rev_mk2(mk2bits_t current, struct timecode_def *def)
{
    if (!def) {
        errno = -EINVAL;
        perror(__func__);
        return U128_ZERO;
    }

    mk2bits_t l, mask;

    /* New bits are added at the LSB; shift left one and mask */
    mask = u128_sub(u128_lshift(U128_ONE,  def->bits), U128_ONE);
    l = lfsr_mk2(current,
         u128_or(u128_rshift(def->taps_mk2, 1),
         u128_lshift(U128_ONE, (def->bits - 1))));

    return u128_or(u128_and(u128_lshift(current, 1), mask), l);
}

/*
 * Where necessary, build the lookup table required for this timecode
 * (Traktor MK2 version)
 *
 * Return: -1 if not enough memory could be allocated, otherwise 0
 */

int build_lookup_mk2(struct timecode_def *def)
{
    if (!def) {
        errno = -EINVAL;
        perror(__func__);
        return -1;
    }

    unsigned int n;
    mk2bits_t current, next;

    if (def->lookup)
        return 0;

    fprintf(stderr, "Building LUT for %d bit %dHz timecode (%s)\n",
            def->bits, def->resolution, def->desc);

    if (lut_init_mk2(&def->lut_mk2, def->length) == -1)
        return -1;

    current = def->seed_mk2;

    for (n = 0; n < def->length; n++) {

        /* timecode must not wrap */
        assert(lut_lookup_mk2(&def->lut_mk2, &current) == (unsigned)-1);
        lut_push_mk2(&def->lut_mk2, &current);

        /* check symmetry of the lfsr functions */
        next = fwd_mk2(current, def);
        assert(u128_eq(rev_mk2(next, def), current));

        current = next;
    }

    def->lookup = true;

    return 0;
}

/*
 * Caches the generated LUT on the disk.
 *
 * This is only necessary for the Traktor MK2, since the size of its hash
 * table is quite large.
 */

int lut_store_mk2(struct timecode_def *def, const char *lut_dir_path)
{
    if (!def || !lut_dir_path)
        return -1;

    struct slot_mk2 *slot;
    slot_no_t *hash;

    int i, j, len, hashes;
    char path[1024];
    FILE *fp = NULL;
    int r = 0;
    int size;

    sprintf(path, "%s/%s%s", lut_dir_path, def->name, ".lut");

    fprintf(stdout, "Storing LUT at %s\n", path);
    fp = fopen(path, "wb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    for (i = 0; i < def->length; i++) {
        slot = &def->lut_mk2.slot[i];

        if (!slot) {
            printf("slot_no: %d doesn't exist'\n", i);
            r = -1;
            goto out;
        }
        size = fwrite(slot, sizeof(struct slot_mk2), 1, fp);

        if (!size) {
            perror("fwrite");
            r = -1;
            goto out;
        }
    }

    hashes = 1 << 16;
    for (j = 0; j < hashes; j++) {
        hash = &def->lut_mk2.table[j];

        size = fwrite(hash, sizeof(slot_no_t), 1, fp);
        if (!size) {
            perror("fwrite");
            r = -1;
            goto out;
        }
    }

    size = fwrite(&def->lut_mk2.avail, sizeof(slot_no_t), 1, fp);
    if (!size) {
        perror("fwrite");
        r = -1;
        goto out;
    }

out:
    fclose(fp);

    if (hashes != j || def->length != i)
        fprintf(stderr, "Something went wrong: ");

    fprintf(stderr, "Wrote %d hashes and %d slots to disk\n", j, i);

    return r;
}


/*
 * Loads the stored LUT from the disk.
 *
 * This is only necessary for the Traktor MK2, since the size of its hash
 * table is quite large.
 */

int lut_load_mk2(struct timecode_def *def, const char *lut_dir_path)
{
    if (!def || !lut_dir_path)
        return -1;

    struct slot_mk2 *slot;

    char path[1024];
    int i, j, hashes;
    int r = 0;
    int size;
    FILE *fp;
    int len;

    sprintf(path, "%s/%s%s", lut_dir_path, def->name, ".lut");

    fprintf(stdout, "Loading LUT from %s\n", path);
    fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "LUT for %s not found on disk\n", def->desc);
        return -1;
    }

    r = lut_init_mk2(&def->lut_mk2, def->length);
    if (r) {
        fprintf(stderr, "Couldn't initialise LUT\n");
        goto out;
    }

    fprintf(stdout, "Loading LUT from %s\n", path);
    for (i = 0; i < def->length; i++) {
        slot = &def->lut_mk2.slot[i];

        size = fread(slot, sizeof(struct slot_mk2), 1, fp);
        if (!size) {
            perror("fread");
            r = -1;
            goto out;
        }
    }

    hashes = 1 << 16;
    for (j = 0; j < hashes; j++) {

        slot_no_t *hash = &def->lut_mk2.table[j];

        size = fread(hash, sizeof(slot_no_t), 1, fp);
        if (!size) {
            perror("fread");
            r = -1;
            goto out;
        }
    }

    size = fread(&def->lut_mk2.avail, sizeof(slot_no_t), 1, fp);
    if (!size) {
        perror("fwrite");
        r = -1;
    }


out:
    fclose(fp);

    if (hashes == j && def->length == i)
        def->lookup = true;
    else
        fprintf(stderr, "Something went wrong: ");

    fprintf(stderr, "Loaded %d hashes and %d slots from disk\n", j, i);

    return r;
}

/*
 * Do Traktor-MK2-specific processing of the carrier wave
 *
 * Pushes samples into a delayline, computes the derivative, computes RMS
 * values and scales the derivative back up to the original signal's level.
 * Afterards the upscaled derivative can by processed by the pitch detection
 * algorithm.
 *
 * NOTE: Ideally the gain compensation should be done in the derivative and lowpass
 * filter structures by determining the amplitude response. I had this
 * implemented previously, but chose gain compensation by using the RMS value,
 * since it is easier to understand for developers not trained in signal
 * processing. Additionally it's nice to have the dB level at hand.
 *
 */

void mk2_process_carrier(struct timecoder *tc, signed int primary, signed int secondary)
{
    if (!tc) {
        errno = -EINVAL;
        perror(__func__);
        return;
    }

    /* Push the samples into the ringbuffer */
    delayline_push(&tc->primary.mk2.delayline, primary);
    delayline_push(&tc->secondary.mk2.delayline, secondary);

    /* Compute the discrete derivative */
    tc->primary.mk2.deriv = derivative(&tc->primary.mk2.differentiator,
            ema(&tc->primary.mk2.ema_filter, primary));
    tc->secondary.mk2.deriv = derivative(&tc->secondary.mk2.differentiator,
            ema(&tc->secondary.mk2.ema_filter, secondary));

    /* Compute the smoothed RMS value */
    tc->primary.mk2.rms = rms(&tc->primary.mk2.rms_filter, primary);
    tc->secondary.mk2.rms = rms(&tc->secondary.mk2.rms_filter, secondary);

    /* Compute the smoothed RMS value for the derivative */
    tc->primary.mk2.rms_deriv = rms(&tc->primary.mk2.rms_deriv_filter, tc->primary.mk2.deriv);
    tc->secondary.mk2.rms_deriv = rms(&tc->secondary.mk2.rms_deriv_filter, tc->secondary.mk2.deriv);

    /* Compute the gain compensation for the derivative*/
    tc->gain_compensation = (double)tc->secondary.mk2.rms / tc->secondary.mk2.rms_deriv;

    /* Without this limit pitch becomes too sensitive */
    if (tc->gain_compensation > 30.0)
        tc->gain_compensation = 30.0;

    tc->dB = 20 * log10((double)tc->secondary.mk2.rms / INT_MAX);

    /* Compute the scaled derivative */
    tc->primary.mk2.deriv_scaled = tc->primary.mk2.deriv * tc->gain_compensation;
    tc->secondary.mk2.deriv_scaled = tc->secondary.mk2.deriv * tc->gain_compensation;
}

/*
 * Detect if the upward or downward slope hits a threshold.
 * Upwards signifies a one and downwards a zero.
 */

static inline void detect_bit_flip(const int slope[2], int rms, int reading, int avg_reading,
        mk2bits_t *bit, bool *bit_flipped, bool forwards, mk2bits_t one)
{
    static const double reverse_factor = 1.75;
    static const double forward_factor = 1.5;
    double threshold;

    if (*bit_flipped == false) {
        if (forwards) {
            threshold = rms / forward_factor;
        } else {
            threshold = rms / reverse_factor;
            one = u128_not(one);
        }

        if (u128_eq(*bit, u128_not(one)) && slope[0] > threshold && slope[1] > threshold) {
            *bit = one;
            *bit_flipped = true;
        } else if (u128_eq(*bit, one) && slope[0] < -threshold && slope[1] < -threshold) {
            *bit = u128_not(one);
            *bit_flipped = true;
        }
    } else {
        *bit_flipped = false;
    }
}

/*
 * Verify the new LFSR state in the forward or reverse direction.
 */

static inline bool lfsr_verify(struct timecode_def *def, mk2bits_t *timecode, mk2bits_t *bitstream,
        mk2bits_t bit, bool forwards)
{
    if (forwards) {
        *timecode = fwd_mk2(*timecode, def);
        *bitstream = u128_add(u128_rshift(*bitstream, 1), u128_lshift(bit, (def->bits - 1)));
    } else {
        mk2bits_t mask = u128_sub(u128_lshift(U128_ONE, def->bits), U128_ONE);
        *timecode = rev_mk2(*timecode, def);
        *bitstream = u128_add(u128_and(u128_lshift(*bitstream, 1), mask), bit);
    }

    if (u128_eq(*timecode, *bitstream))
        return true;
    else
        return false;
}

/*
 * Process the upper or lower bitstream contained in the Traktor MK2 signal
 */

inline static void mk2_process_bitstream(struct timecoder *tc, struct mk2_subcode *sc,
        signed int reading)
{
    int current_slope[2];

    delayline_push(&sc->readings, reading);
    sc->avg_reading = ema(&sc->ema_reading, reading);

    /* Calculate absolute of average slope */
    sc->avg_slope = ema(&sc->ema_slope, abs(reading - *delayline_at(&sc->readings, 1)));

    /* Calculate current and last slope */
    current_slope[0] =  (reading - *delayline_at(&sc->readings, 1));
    current_slope[1] =  (reading - *delayline_at(&sc->readings, 2));

    /* The bits only change when an offset jump occurs. Else the previous bit is taken  */
    detect_bit_flip(current_slope, tc->secondary.mk2.rms, reading, sc->avg_reading, &sc->bit,
                    &sc->recent_bit_flip, tc->forwards, U128(0x0, !tc->secondary.positive));

    if (lfsr_verify(tc->def, &sc->timecode, &sc->bitstream, sc->bit, tc->forwards)) {
        (sc->valid_counter)++;
    } else {
        sc->timecode = sc->bitstream;
        sc->valid_counter = 0;
    }
}

/*
 * Process the upper or lower bitstream contained in the Traktor MK2 signal
 */

void mk2_process_timecode(struct timecoder *tc, signed int reading)
{
    /*
     * Detect if the offset jumps on upper and lower bitstream
     */

    if (tc->secondary.positive)
        mk2_process_bitstream(tc, &tc->upper_bitstream, reading);
    else if (!tc->secondary.positive)
        mk2_process_bitstream(tc, &tc->lower_bitstream, reading);

    if (tc->lower_bitstream.valid_counter > tc->upper_bitstream.valid_counter) {
        tc->mk2_bitstream = tc->lower_bitstream.bitstream;
        tc->mk2_timecode = tc->lower_bitstream.timecode;
    } else {
        tc->mk2_bitstream = tc->upper_bitstream.bitstream;
        tc->mk2_timecode = tc->upper_bitstream.timecode;
    }

    if (u128_eq(tc->mk2_timecode, tc->mk2_bitstream)) {
        tc->valid_counter++;
    } else {
        tc->timecode = tc->bitstream;
        tc->valid_counter = 0;
    }
    /* Take note of the last time we read a valid timecode */

    tc->timecode_ticker = 0;

    tc->ref_level -= tc->ref_level / REF_PEAKS_AVG;
    tc->ref_level += abs((int)(tc->secondary.mk2.rms_deriv * tc->gain_compensation))
        / REF_PEAKS_AVG;

    debug("upper.valid_counter: %d, lower.valid_counter %d, forwards: %b\n", */
           tc->upper.valid_counter,
           tc->lower.valid_counter,
           tc->forwards);
}
