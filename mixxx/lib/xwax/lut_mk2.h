#ifndef LUT_MK2_H

#define LUT_MK2_H

#include "lut.h"

typedef u128 mk2bits_t;

struct slot_mk2 {
    mk2bits_t timecode;
    slot_no_t next; /* next slot with the same hash */
};

struct lut_mk2 {
    struct slot_mk2 *slot;
    slot_no_t *table, /* hash -> slot lookup */
        avail; /* next available slot */
};

int lut_init_mk2(struct lut_mk2 *lut, int nslots);
void lut_clear_mk2(struct lut_mk2 *lut);
void lut_push_mk2(struct lut_mk2 *lut, mk2bits_t *timecode);
slot_no_t lut_lookup_mk2(struct lut_mk2 *lut, mk2bits_t *timecode);

#endif /* end of include guard LUT_MK2_H */
