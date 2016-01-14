#ifndef DLGTRACKEXPORT_H
#define DLGTRACKEXPORT_H

#include <QDialog>
#include <QString>

#include "library/export/ui_dlgtrackexport.h"

class DlgTrackExport {
    Q_OBJECT
public:
    enum class OverwriteMode {
        ASK,
        OVERWRITE_ALL,
        SKIP_ALL,
    };

    virtual ~DlgTrackExport();

private:
};

#endif DLGTRACKEXPORT_H
