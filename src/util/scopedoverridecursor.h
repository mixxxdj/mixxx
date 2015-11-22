#ifndef SCOPEDOVERRIDECURSOR_H
#define SCOPEDOVERRIDECURSOR_H

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

#endif // SCOPEDOVERRIDECURSOR_H
