/*
 * Copyright (C) 2021 Mark Hills <mark@xwax.org>
 *
 * This file is part of "xwax".
 *
 * "xwax" is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 3 as
 * published by the Free Software Foundation.
 *
 * "xwax" is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LUT_H
#define LUT_H

#include "types.h"

typedef unsigned int slot_no_t;
typedef u128 mk2bits_t;

struct slot {
    unsigned int timecode;
    slot_no_t next; /* next slot with the same hash */
};

struct slot_mk2 {
    mk2bits_t timecode;
    slot_no_t next; /* next slot with the same hash */
};

struct lut {
    struct slot *slot;
    slot_no_t *table, /* hash -> slot lookup */
        avail; /* next available slot */
};

struct lut_mk2 {
    struct slot_mk2 *slot;
    slot_no_t *table, /* hash -> slot lookup */
        avail; /* next available slot */
};

int lut_init(struct lut *lut, int nslots);
void lut_clear(struct lut *lut);

void lut_push(struct lut *lut, unsigned int timecode);
unsigned int lut_lookup(struct lut *lut, unsigned int timecode);

int lut_init_mk2(struct lut_mk2 *lut, int nslots);
void lut_clear_mk2(struct lut_mk2 *lut);

void lut_push_mk2(struct lut_mk2 *lut, mk2bits_t *timecode);
slot_no_t lut_lookup_mk2(struct lut_mk2 *lut, mk2bits_t *timecode);

#endif
