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
typedef unsigned int bits_t;

struct slot {
    bits_t timecode;
    slot_no_t next; /* next slot with the same hash */
};

struct lut {
    struct slot *slot;
    slot_no_t *table, /* hash -> slot lookup */
        avail; /* next available slot */
};

int lut_init(struct lut *lut, int nslots);
void lut_clear(struct lut *lut);

void lut_push(struct lut *lut, bits_t timecode);
unsigned int lut_lookup(struct lut *lut, bits_t timecode);

#endif
