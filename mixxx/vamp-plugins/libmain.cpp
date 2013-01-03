/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    QM Vamp Plugin Set

    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "vamp/vamp.h"
#include "vamp-sdk/PluginAdapter.h"
#include "plugins/BeatTrack.h"
#include "plugins/BarBeatTrack.h"
#include "plugins/MixxxBpmDetection.h"

static Vamp::PluginAdapter<BeatTracker> beatTrackerAdapter;
static Vamp::PluginAdapter<BarBeatTracker> barBeatTrackPluginAdapter;
static Vamp::PluginAdapter<MixxxBpmDetection> MixxxBpmDetection;

const VampPluginDescriptor *vampGetPluginDescriptor(unsigned int vampApiVersion,
                                                    unsigned int index)
{
    if (vampApiVersion < 1) return 0;

    switch (index) {
    // cases have to start at 0, to have mixxxbpmdetection as a fallback
    // it should always be the first one, kain88 12/2012
    case  0: return MixxxBpmDetection.getDescriptor();
    case  1: return beatTrackerAdapter.getDescriptor();
    case  2: return barBeatTrackPluginAdapter.getDescriptor();
    default: return 0;
    }
}

