/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

//
// This is a fork of QPerformanceTimer just without the Q prefix
// To fix interface changes issues in different QT versions
// Added restart() function.
// Returns time in nanosecond resolution.
//

#include <QtCore/qglobal.h>

#include "util/duration.h"

class PerformanceTimer
{
public:
    PerformanceTimer() {
      t1 = 0;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
      t2 = 0;
#endif
    };

    void start();
    mixxx::Duration elapsed() const;
    mixxx::Duration restart();
    mixxx::Duration difference(const PerformanceTimer& timer) const;

private:
    qint64 t1;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    qint64 t2;
#endif
};
