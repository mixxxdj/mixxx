#ifndef PIXMAPSOURCE_H
#define PIXMAPSOURCE_H

// A class representing an image source for a pixmap
// A bundle of a file path, raw data or inline svg
class PixmapSource {
  public:
    PixmapSource();
    virtual ~PixmapSource();

    bool isEmpty();
    void setSVG( QByteArray content );
    void setSVG( QString filepath );
    void setBitmap( QString filepath );
    void setPath( QString newPath );
    QString getPath() const;
    QString getType() const;
    QByteArray getData() const;
    QString getId() const;
    
  private:
	QString path;
	QString type;
	QByteArray data;
};

#endif /* PIXMAPSOURCE_H */
