#ifndef LUT_MK2_H

#define LUT_MK2_H

#include "lut.h"

#include <stdint.h>

#define MIXXX_LUT_MAGIC 0x54554c585858494duLL /* MIXXXLUT */
#define MIXXX_LUT_MAJOR 1
#define MIXXX_LUT_MINOR 0

typedef u128 mk2bits_t;

/*
 * NOTE: Should the member order of any of these structs ever be changed,
 * it is imperative that the MIXXX_LUT_MINOR gets incremented !!!
 */

struct lut_mk2_header {
    uint64_t magic;
    uint16_t major;
    uint16_t minor;
};

struct slot_mk2 {
    mk2bits_t timecode;
    slot_no_t next; /* next slot with the same hash */
};

struct lut_mk2 {
    struct lut_mk2_header *hdr;
    struct slot_mk2 *slot;
    slot_no_t *table, /* hash -> slot lookup */
        avail; /* next available slot */
};

int lut_init_mk2(struct lut_mk2 *lut, int nslots);
void lut_clear_mk2(struct lut_mk2 *lut);
void lut_push_mk2(struct lut_mk2 *lut, mk2bits_t *timecode);
slot_no_t lut_lookup_mk2(struct lut_mk2 *lut, mk2bits_t *timecode);

#endif /* end of include guard LUT_MK2_H */
