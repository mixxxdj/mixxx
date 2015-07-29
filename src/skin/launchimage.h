#ifndef LAUNCHIMAGE_H_
#define LAUNCHIMAGE_H_

#include <QWidget>

class QProgressBar;

class LaunchImage: public QWidget {
    Q_OBJECT
  public:
    LaunchImage(QWidget* pParent, const QString& imagePath);
    virtual ~LaunchImage();
    void progress(int value);

  protected:
    virtual void paintEvent(QPaintEvent *);

  private:
    QProgressBar* m_pProgressBar;
};

#endif // LAUNCHIMAGE_H_
