#pragma once

#include <QGuiApplication>

class ScopedOverrideCursor {
  public:
    inline explicit ScopedOverrideCursor(const QCursor& cursor) {
        QGuiApplication::setOverrideCursor(cursor);
        QCoreApplication::processEvents();
    }

    inline virtual ~ScopedOverrideCursor() {
        QGuiApplication::restoreOverrideCursor();
    }
};

class ScopedWaitCursor : public ScopedOverrideCursor {
  public:
    ScopedWaitCursor()
        : ScopedOverrideCursor(Qt::WaitCursor)
    {
    }
};
