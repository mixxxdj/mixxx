#ifndef LIBRARYVIEWFEAUTRE_H
#define LIBRARYVIEWFEAUTRE_H

#include <QWidget>

#include "library/libraryfeature.h"

// This is a more simple interface that the existing LibraryFeature
// every feature has two widgets (left and right)
class LibraryViewFeature : public LibraryFeature {
    Q_OBJECT

  public:

    LibraryViewFeature(UserSettingsPointer pConfig);

    virtual QVariant getTitle() = 0;
    virtual QIcon getIcon() = 0;
    
    // Must be a unique name for each feature type, it will be used in 
    // the button bar to distinguish one feature from another
    virtual QString getName() = 0; 

    // A LibraryViewFeature always has a right and a left pane
    virtual QWidget* getLeftPane() = 0;

    virtual QWidget* getRightPane() = 0;

    // If it has not the search option, the search bar must be hidden
    virtual bool hasSearch() = 0;

  public slots:

    virtual void onSearch(QString& text) = 0;

  signals:

    void focused();
};

#endif // LIBRARYVIEWFEAUTRE_H
