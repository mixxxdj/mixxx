#pragma once

#include <QLineEdit>
#include <QPushButton>

#include "track/cue.h"
#include "track/track.h"
#include "widget/colormenu.h"

class CueMenu : public QWidget {
    Q_OBJECT
  public:
    CueMenu(QWidget* parent = nullptr);
    ~CueMenu() override;

    void setCue(CuePointer pCue) {
        m_pCue = pCue;
        if (m_pCue) {
            m_pEditLabel->setText(m_pCue->getLabel());
        }
    }

    void setTrack(TrackPointer pTrack) {
        m_pTrack = pTrack;
    }

    void useColorSet(PredefinedColorsRepresentation* pColorRepresentation) {
        if (m_pColorMenu != nullptr) {
            m_pColorMenu->useColorSet(pColorRepresentation);
        }
    }

    void popup(const QPoint& p, QAction* atAction = nullptr) {
        Q_UNUSED(atAction);
        qDebug() << "Showing menu at" << p;
        move(p);
        show();
    }

    void hide() {
        emit(aboutToHide());
        QWidget::hide();
    }

    void show() {
        emit(aboutToShow());
        QWidget::show();
    }

  signals:
    void aboutToHide();
    void aboutToShow();

  private slots:
    void slotEditLabel();
    void slotRemoveCue();
    void slotChangeCueColor(PredefinedColorPointer pColor);

  private:
    CuePointer m_pCue;
    TrackPointer m_pTrack;

    QLineEdit* m_pEditLabel;
    ColorMenu* m_pColorMenu;
    QPushButton* m_pRemoveCue;
};
