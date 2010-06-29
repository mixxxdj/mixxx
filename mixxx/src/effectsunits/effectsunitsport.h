/*
 * effectsunitsport.h
 *
 *  Created on: Jun 29, 2010
 *      Author: bruno
 */

#ifndef EFFECTSUNITSPORT_H_
#define EFFECTSUNITSPORT_H_

typedef struct _EffectsUnitsPort{
		QString * Name;
        double Min;
        double Max;
        double Def;
        bool isAudio;
        bool isBound;
        ControlObject * bind;
} EffectsUnitsPort;

#endif /* EFFECTSUNITSPORT_H_ */
