/**
* @file midicontroller_name_regexps.h
* @author Ilkka Tuohela hile@iki.fi
* @date Tue 24 Jul 2013
* @brief List of regexps to match certain MIDI IN/OUT port names
*/

#ifndef MIDICONTROLLER_NAME_REGEXPS
#define MIDICONTROLLER_NAME_REGEXPS

#include <QRegExp>

// Linee Lemur Daemon input output ports (8 pairs)
const QRegExp LEMUR_INPUT_RE = QRegExp("^Daemon Input (\\d+)$");
const QRegExp LEMUR_OUTPUT_RE = QRegExp("^Daemon Output (\\d+)$");

#endif
