// WSingletonContainer defines widgets that should only be instantiated once
// but may appear in multiple places in a skin definition.  This is useful
// for complex widgets like the library, which are memory intensive. The
// container mostly looks like a special WidgetGroup which is defined in
// special ways.
//
// Usage:
// First, the Singleton container is defined, meaning it is described to the
// skin system by name, and what the singleton consists of.  This definition
// should be very early in the skin file.  Note that the singleton does not
// actually appear where it is defined.
//
// Example definition:
// <DefineSingleton>
//   <ObjectName>LibrarySingleton</ObjectName>
//   <Layout>horizontal</Layout>
//   <SizePolicy>me,me</SizePolicy>
//   <Children>
//     <Template src="skin:library.xml"/>
//   </Children>
//  </DefineSingleton>
//
// The ObjectName is used to identify this singleton elsewhere in the skin
// files.
//
// Example usage:
// <WidgetGroup>
//    <ObjectName>SomeUiElement</ObjectName>
//    <Layout>vertical</Layout>
//    <SizePolicy>min,i</SizePolicy>
//    <Children>
//      <Singleton objectName="LibrarySingleton"/>
//      ...
//    </Children>
// </WidgetGroup>
//
// The skin system sees the Singleton tag, and any time the containing
// group gets a show event, the Singleton widget is reparented to this location
// in the skin.  Note that if a Singleton is visible twice at the same time,
// behavior is undefined and could be crashy.

#ifndef WSINGLETONCONTAINER_H
#define WSINGLETONCONTAINER_H

#include "widget/wwidgetgroup.h"

class WSingletonContainer : public WWidgetGroup {

    Q_OBJECT


  public:
    typedef QMap<QString, QWidget*> WidgetMap;

    virtual ~WSingletonContainer() { }

    // We don't want to end up with badly-constructed containers, so only
    // provide a factory function.
    static WSingletonContainer* getSingleton(QString objectname,
                                             QWidget* pParent=NULL);

    // Takes a constructed QWidget and inserts it in the map of available
    // singletons.
    static void defineSingleton(QString objectname, QWidget* widget);

  public slots:
    virtual void showEvent(QShowEvent* event);

  private:
    WSingletonContainer(QString objectname, QWidget* pParent=NULL);

    static WidgetMap m_singletons;
    QWidget* m_pWidget;
    QLayout* m_pLayout;
};

#endif  // WSINGLETONCONTAINER_H
