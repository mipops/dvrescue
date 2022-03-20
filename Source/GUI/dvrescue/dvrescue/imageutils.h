#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QObject>
#include <QImage>

class ImageUtils : public QObject
{
    Q_OBJECT
public:
    explicit ImageUtils(QObject *parent = nullptr);
    Q_INVOKABLE QImage toImage(const QByteArray& buffer, const QString& format);
    Q_INVOKABLE QString toDataUri(const QByteArray& buffer, const QString& format);

Q_SIGNALS:

};

#endif // IMAGEUTILS_H
