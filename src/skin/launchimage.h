#ifndef LAUNCHIMAGE_H_
#define LAUNCHIMAGE_H_

#include <QWidget>

class QProgressBar;

class LaunchImage: public QWidget {
  public:
    LaunchImage(QWidget* pParent);
    virtual ~LaunchImage();
    void progress(int value);

  private:
    QProgressBar* m_pProgressBar;
};

#endif // LAUNCHIMAGE_H_
