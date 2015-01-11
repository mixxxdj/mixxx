#ifndef WLIBRARYCONTAINER_H
#define WLIBRARYCONTAINER_H

#include "widget/wwidgetgroup.h"

class WLibrary;

class WLibraryContainer : public WWidgetGroup {

    Q_OBJECT

  public:
    WLibraryContainer(WLibrary* library,
                      QDomNode node,
                      const SkinContext& context,
                      QWidget* pParent=NULL);
    virtual ~WLibraryContainer() { }

  public slots:
    virtual void setVisible(bool visible);

  private:
    WLibrary* m_pLibrary;
};

#endif  // WLIBRARYCONTAINER_H
