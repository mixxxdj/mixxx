/*
 * Copyright (C) 2012 Mark Hills <mark@pogo.org.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#ifndef LUT_H
#define LUT_H

typedef unsigned int slot_no_t;

struct slot {
    unsigned int timecode;
    slot_no_t next; /* next slot with the same hash */
};

struct lut {
    struct slot *slot;
    slot_no_t *table, /* hash -> slot lookup */
        avail; /* next available slot */
};

int lut_init(struct lut *lut, int nslots);
void lut_clear(struct lut *lut);

void lut_push(struct lut *lut, unsigned int timecode);
unsigned int lut_lookup(struct lut *lut, unsigned int timecode);

#endif
