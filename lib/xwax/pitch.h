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

#ifndef PITCH_H
#define PITCH_H

/* Values for the filter concluded experimentally */

#define ALPHA (1.0/512)
#define BETA (ALPHA/256)

/* State of the pitch calculation filter */

struct pitch_filter {
    double dt, x, v;
};

/* Prepare the filter for observations every dt seconds */

static inline void pitch_init(struct pitch_filter *p, double dt)
{
    p->dt = dt;
    p->x = 0.0;
    p->v = 0.0;
}

/* Input an observation to the filter; in the last dt seconds the
 * position has moved by dx.
 *
 * Because the vinyl uses timestamps, the values for dx are discrete
 * rather than smooth. */

static inline void pitch_dt_observation(struct pitch_filter *p, double dx)
{
    double predicted_x, predicted_v, residual_x;

    predicted_x = p->x + p->v * p->dt;
    predicted_v = p->v;

    residual_x = dx - predicted_x;

    p->x = predicted_x + residual_x * ALPHA;
    p->v = predicted_v + residual_x * BETA / p->dt;

    p->x -= dx; /* relative to previous */
}

#endif
