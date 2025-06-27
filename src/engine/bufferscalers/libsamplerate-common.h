/**
 * FILE: libsamplerate-common.h
 * ----------------------------
 * Definitions of opaque
 * datatypes to build a POC for libsamplerate as a scratching library.
 * XXX DO NOT MERGE INTO UPSTREAM.
 */
#include <samplerate.h>

enum { SRC_FALSE = 0,
    SRC_TRUE = 1,
};

enum SRC_MODE {
    SRC_MODE_PROCESS = 0,
    SRC_MODE_CALLBACK = 1
};

typedef enum SRC_ERROR {
    SRC_ERR_NO_ERROR = 0,

    SRC_ERR_MALLOC_FAILED,
    SRC_ERR_BAD_STATE,
    SRC_ERR_BAD_DATA,
    SRC_ERR_BAD_DATA_PTR,
    SRC_ERR_NO_PRIVATE,
    SRC_ERR_BAD_SRC_RATIO,
    SRC_ERR_BAD_PROC_PTR,
    SRC_ERR_SHIFT_BITS,
    SRC_ERR_FILTER_LEN,
    SRC_ERR_BAD_CONVERTER,
    SRC_ERR_BAD_CHANNEL_COUNT,
    SRC_ERR_SINC_BAD_BUFFER_LEN,
    SRC_ERR_SIZE_INCOMPATIBILITY,
    SRC_ERR_BAD_PRIV_PTR,
    SRC_ERR_BAD_SINC_STATE,
    SRC_ERR_DATA_OVERLAP,
    SRC_ERR_BAD_CALLBACK,
    SRC_ERR_BAD_MODE,
    SRC_ERR_NULL_CALLBACK,
    SRC_ERR_NO_VARIABLE_RATIO,
    SRC_ERR_SINC_PREPARE_DATA_BAD_LEN,
    SRC_ERR_BAD_INTERNAL_STATE,

    /* This must be the last error number. */
    SRC_ERR_MAX_ERROR
} SRC_ERROR;

typedef struct SRC_STATE_VT_tag {
    /* Varispeed process function. */
    SRC_ERROR (*vari_process)(SRC_STATE* state, SRC_DATA* data);

    /* Constant speed process function. */
    SRC_ERROR (*const_process)(SRC_STATE* state, SRC_DATA* data);

    /* State reset. */
    void (*reset)(SRC_STATE* state);

    /* State clone. */
    SRC_STATE* (*copy)(SRC_STATE* state);

    /* State close. */
    void (*close)(SRC_STATE* state);
} SRC_STATE_VT;

struct SRC_STATE_tag {
    SRC_STATE_VT* vt;

    double last_ratio,
            last_position; // filter = (SINC_FILTER*) state->private_data ;
                           // last_position is wrong.

    SRC_ERROR error;
    int channels;

    /* SRC_MODE_PROCESS or SRC_MODE_CALLBACK */
    enum SRC_MODE mode;

    /* Data specific to SRC_MODE_CALLBACK. */
    src_callback_t callback_func;
    void* user_callback_data;
    long saved_frames;
    const float* saved_data;

    /* Pointer to data to converter specific data. */
    void* private_data;
};

double src_get_last_position(SRC_STATE* state);

// Linear Private data
typedef struct
{
    int linear_magic_marker;
    bool dirty;
    long in_count, in_used;
    long out_count, out_gen;
    float* last_value;
} LINEAR_DATA;

// sinc private data
#ifndef CONFIG_CHAN_NR
#define MAX_CHANNELS 128
#else
#define MAX_CHANNELS CONFIG_CHAN_NR
#endif

typedef float coeff_t;

typedef struct
{
    int sinc_magic_marker;

    long in_count, in_used;
    long out_count, out_gen;

    int coeff_half_len, index_inc;

    double src_ratio, input_index;

    coeff_t const* coeffs;

    int b_current, b_end, b_real_end, b_len;

    /* Sure hope no one does more than 128 channels at once. */
    double left_calc[MAX_CHANNELS], right_calc[MAX_CHANNELS];

    float* buffer;
} SINC_FILTER;
