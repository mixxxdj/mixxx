/* 
 * Copyright (C) 2009 Mark Hills <mark@pogo.org.uk>
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

#include <stdio.h>

#include "pitch.h"

/* Values for the filter concluded experimentally */

#define ALPHA (1.0/32)
#define BETA (ALPHA/256)
#define GAMMA (BETA/8192)

#define SQ(x) ((x)*(x))

/* Prepare the filter for observations every dt seconds */

void pitch_init(struct pitch_t *p, float dt)
{
    p->dt = dt;
    p->x = 0.0;
    p->v = 0.0;
    p->a = 0.0;
}

/* Input an observation to the filter; in the last dt seconds the
 * position has moved by dx.
 *
 * Because the vinyl uses timestamps, the values for dx are discrete
 * rather than smooth, hence the use of the gamma extension */

void pitch_dt_observation(struct pitch_t *p, float dx)
{
    float predicted_x, predicted_v, predicted_a, residual_x;

    predicted_x = p->x + p->v * p->dt + 0.5 * p->a * SQ(p->dt);
    predicted_v = p->v + p->a * p->dt;
    predicted_a = p->a;

    residual_x = dx - predicted_x;

    p->x = predicted_x + residual_x * ALPHA;
    p->v = predicted_v + residual_x * BETA / p->dt;
    p->a = predicted_a + residual_x * GAMMA / (2.0 * SQ(p->dt));

    p->x -= dx; /* relative to previous */
}
