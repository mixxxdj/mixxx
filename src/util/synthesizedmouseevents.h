/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Adopted from Qt 5.3
// qtbase/src/plugins/platforms/windows/qwindowsmousehandler.cpp

#ifndef SYNTHESIZEDMOUSEEVENTS_H
#define SYNTHESIZEDMOUSEEVENTS_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {

#ifdef Q_OS_WIN

enum : quint64 { signatureMask = 0xffffff00, miWpSignature = 0xff515700 };

bool isMouseEventSynthesized() {
    // Check for events synthesized from touch. Lower 7 bits are touch/pen index, bit 8 indicates touch.
    // However, when tablet support is active, extraInfo is a packet serial number. This is not a problem
    // since we do not want to ignore mouse events coming from a tablet.
    const quint64 extraInfo = quint64(GetMessageExtraInfo());
    if ((extraInfo & signatureMask) == miWpSignature) {
        if (extraInfo & 0x80) { // Bit 7 indicates touch event, else tablet pen.
            return true;
        }
    }
    return false;
}

#else

bool isMouseEventSynthesized() {
    return false;
}

#endif

} // anomymous namespace

#endif /* SYNTHESIZEDMOUSEEVENTS_H */
