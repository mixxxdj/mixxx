#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QWidgetAction>

#include "widget/wcolorpicker.h"

class WColorPickerAction : public QWidgetAction {
  public:
    WColorPickerAction(QWidget* parent)
            : QWidgetAction(parent) {
        m_pColorPicker = new WColorPicker(parent, true);

        QHBoxLayout* pLayout = new QHBoxLayout();
        pLayout->addWidget(m_pColorPicker);

        QWidget* pWidget = new QWidget();
        pWidget->setLayout(pLayout);
        setDefaultWidget(pWidget);
    }

    ~WColorPickerAction() {
        delete m_pColorPicker;
    }

    WColorPicker* colorPicker() {
        return m_pColorPicker;
    }

  private:
    WColorPicker* m_pColorPicker;
};
