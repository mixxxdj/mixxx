#ifndef WANALYSISLIBRARYTABLEVIEW_H
#define WANALYSISLIBRARYTABLEVIEW_H

#include <QWidget>

#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class Library;

class WAnalysisLibraryTableView : public WTrackTableView
{
    public:
        WAnalysisLibraryTableView(QWidget* parent, UserSettingsPointer pConfig,
                                 Library* pLibrary);

        void onSearch(const QString& text) override;
};

#endif
