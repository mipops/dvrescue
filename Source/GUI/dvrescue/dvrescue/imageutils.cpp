#include "imageutils.h"

ImageUtils::ImageUtils(QObject *parent)
    : QObject{parent}
{

}

QImage ImageUtils::toImage(const QByteArray &buffer, const QString &format)
{
    return QImage::fromData(buffer, format.toLatin1());
}

QString ImageUtils::toDataUri(const QByteArray &buffer, const QString &format)
{
    return QString("data:image/%1;base64,%2").arg(format).arg(QString(buffer.toBase64()));
}
