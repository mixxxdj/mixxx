#pragma once

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QProgressBar);
QT_FORWARD_DECLARE_CLASS(QLabel);

// This is a widget that is shown in the Mixxx main window
// until the skin is ready to use.
// It shows a centered Image and a progress bar below.
// By default a symbolic Mixxx icon and logo is shown.
// It can be modified in the skin.xml file at <skin> section,
// to match the skin like that:
//    <LaunchImageStyle>
//        LaunchImage { background-color: #202020; }
//        QLabel {
//            image: url(skin:/style/mixxx-icon-logo-symbolic.svg);
//            padding:0;
//            margin:0;
//            border:none;
//            min-width: 208px;
//            min-height: 48px;
//            max-width: 208px;
//            max-height: 48px;
//        }
//        QProgressBar {
//            background-color: #202020;
//            border:none;
//            min-width: 208px;
//            min-height: 3px;
//            max-width: 208px;
//            max-height: 3px;
//        }
//        QProgressBar::chunk { background-color: #ec4522; }
//    </LaunchImageStyle>

class LaunchImage: public QWidget {
    Q_OBJECT
  public:
    LaunchImage(QWidget* pParent, const QString& styleSheet);
    virtual ~LaunchImage();
    void progress(int value, const QString& serviceName);

  protected:
    virtual void paintEvent(QPaintEvent *);

  private:
    QProgressBar* m_pProgressBar;
};
