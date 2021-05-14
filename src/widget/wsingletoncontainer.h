// WSingletonContainer defines widgets that should only be instantiated once
// but may appear in multiple places in a skin definition.  This is useful
// for complex widgets like the library, which are memory intensive. The
// container mostly looks similar to a WidgetGroup, but is very lightweight
// and only supports the ObjectName and Children tags, and only one child
// can be specified.  More complicated layout should be done inside the
// defined child itself or the SingletonContainer.
//
// Usage:
// First, the Singleton container is defined, meaning it is described to the
// skin system by name, and what the singleton consists of.  This definition
// must occur before the SingletonContainer elements which define where the
// singleton will appear.  Note that the singleton does not actually appear
// where it is defined.
//
// Example definition:
// <SingletonDefinition>
//   <ObjectName>LibrarySingleton</ObjectName>
//   <Children>
//     <Template src="skin:library.xml"/>
//   </Children>
// </SingletonDefinition>
//
// The ObjectName is used to identify this singleton elsewhere in the skin
// files.  The SingletonContainer does obey standard widget properties like
// SizePolicy and TooltipId.
//
// Example usage:
// <WidgetGroup>
//    <ObjectName>SomeUiElement</ObjectName>
//    <Layout>vertical</Layout>
//    <SizePolicy>min,i</SizePolicy>
//    <Children>
//      <SingletonContainer>
//        <SizePolicy>me,me</SizePolicy>
//        <ObjectName>LibrarySingleton</ObjectName>
//      </SingletonContainer
//      ...
//    </Children>
// </WidgetGroup>
//
// The skin system sees the Singleton tag, and any time the containing
// group gets a show event, the Singleton widget is reparented to this location
// in the skin.  Note that if a Singleton is visible twice at the same time,
// behavior is undefined and could be crashy.

#pragma once

#include <QPointer>

#include "widget/wwidgetgroup.h"

class WSingletonContainer : public WWidgetGroup {
    Q_OBJECT
  public:
    // Prepares the container and remembers the widget, but does not add the
    // widget to the container.
    explicit WSingletonContainer(QWidget* pParent=nullptr);

    void setup(const QDomNode& node, const SkinContext& context) override;

  public slots:
    void showEvent(QShowEvent* event) override;

  private:
    QPointer<QWidget> m_pWidget;
    QLayout* m_pLayout;
};

class SingletonMap {
  public:
    // Takes a constructed QWidget and inserts it in the map of available
    // singletons.  Checks that an object of that name hasn't already been
    // defined.
    void insertSingleton(const QString& objectName, QWidget* widget);

    // We don't want to end up with badly-constructed containers, so only
    // provide a factory function.  Returns NULL if the objectName is not in
    // the map.
    QWidget* getSingletonWidget(const QString& objectName) const;

  private:
    QMap<QString, QWidget*> m_singletons;
};
