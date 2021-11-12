#include "filewriter.h"

#include <QThread>

FileWriter::FileWriter(QObject *parent) : QObject(parent)
{

}

FileWriter::~FileWriter()
{
    if(file.isOpen())
        file.close();
}

void FileWriter::write(const QByteArray &bytes)
{
    QThread::msleep(1000);

    if(file.isOpen())
        file.write(bytes);
}

QString FileWriter::fileName() const
{
    return file.fileName();
}

void FileWriter::setFileName(const QString &newFileName)
{
    if (file.fileName() == newFileName)
        return;
    file.setFileName(newFileName);
    Q_EMIT fileNameChanged();
}

bool FileWriter::open()
{
    return file.open(QIODevice::WriteOnly);
}

void FileWriter::close()
{
    file.close();
}
