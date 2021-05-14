#pragma once

#include <QApplication>

class ScopedOverrideCursor {
  public:
    inline explicit ScopedOverrideCursor(const QCursor& cursor) {
        QApplication::setOverrideCursor(cursor);
        QApplication::processEvents();
    }

    inline virtual ~ScopedOverrideCursor() {
        QApplication::restoreOverrideCursor();
    }
};

class ScopedWaitCursor : public ScopedOverrideCursor {
  public:
    ScopedWaitCursor()
        : ScopedOverrideCursor(Qt::WaitCursor)
    {
    }
};
