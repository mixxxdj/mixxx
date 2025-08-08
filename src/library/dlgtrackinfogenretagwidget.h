#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class GenreTagWidget : public QWidget {
    Q_OBJECT
  public:
    explicit GenreTagWidget(QWidget* parent = nullptr);

    void setTags(const QStringList& tags);
    QStringList tags() const;

  signals:
    void tagsChanged(const QStringList& tags);

  private slots:
    void onTagEntered();
    // void removeTag();

  private:
    void addTag(const QString& tagText);

    QHBoxLayout* m_layout;
    QLineEdit* m_input;
    QStringList m_tags;
};
