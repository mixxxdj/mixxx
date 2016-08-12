#include "layout.h"
#include <QDebug>

Layout::Layout(QString varName, QString name, KeyboardLayoutPointer pData) :
        varName(varName),
        name(name) {

    qDebug() << "Loading layout " << name;

    // Copy layout data
    for (int i = 0; i < LAYOUT_LEN; i++, pData++) {
        data[i][0] = pData[i][0]; // Unmodified KbdKeyChar
        data[i][1] = pData[i][1]; // Shift modified KbdKeyChar
    }
}

Layout::~Layout() {}
