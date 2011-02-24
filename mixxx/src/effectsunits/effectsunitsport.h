/*
 * effectsunitsport.h
 *
 *  Created on: Jun 29, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPORT_H
#define EFFECTSUNITSPORT_H

#include "controlobject.h"

typedef struct _EffectsUnitsPort{
		QString* Name;
    float Min;
    float Max;
    float Def;
    bool isAudio;
} EffectsUnitsPort;

#endif /* EFFECTSUNITSPORT_H */
