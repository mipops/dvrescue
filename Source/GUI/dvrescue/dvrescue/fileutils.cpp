#include "fileutils.h"
#include <QDir>
#include <QStringList>
#include <QTextStream>
#include <QUrl>

FileUtils::FileUtils(QObject *parent) : QObject(parent)
{

}

QStringList FileUtils::getFiles(const QString &dirPath, const QString &extension)
{
    QString localPath = dirPath;

    QUrl url(dirPath);
    if(url.isLocalFile())
        localPath = url.toLocalFile();

    QDir dir(localPath);
    auto files = dir.entryInfoList(QStringList() << extension, QDir::Files | QDir::NoSymLinks);

    QStringList filePaths;
    for(auto file : files) {
        filePaths.append(file.absoluteFilePath());
    }

    return filePaths;
}

QString FileUtils::getFile(const QString &dirPath, const QString &extension)
{
    QString localPath = dirPath;

    QUrl url(dirPath);
    if(url.isLocalFile())
        localPath = url.toLocalFile();

    QDir dir(localPath);
    auto files = dir.entryInfoList(QStringList() << extension, QDir::Files | QDir::NoSymLinks);

    return files.empty() ? "" : files[0].absoluteFilePath();
}

QString FileUtils::getFileName(const QString &filePath)
{
    QFileInfo info(filePath);

    return info.fileName();
}

QString FileUtils::getFilePath(const QString &urlOrPath)
{
    QUrl url(urlOrPath);

    return url.toLocalFile();
}

QString FileUtils::getFileExtension(const QString &filePath)
{
    QFileInfo info(filePath);

    return info.suffix();
}

QString FileUtils::toLocalUrl(const QString &filePath)
{
    QUrl url(QUrl::fromLocalFile(filePath));

    return url.toString();
}

QString FileUtils::read(const QString &filePath)
{
    QString content;

    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        content = stream.readAll();
    }

    return content;
}

bool FileUtils::remove(const QString &filePath)
{
    QString path = filePath;
    if(path.startsWith("file://", Qt::CaseInsensitive)) {
        path = path.remove("file://", Qt::CaseInsensitive);
    }

    return QFile(path).remove();
}

bool FileUtils::copy(const QString &target, const QString &destination)
{
    return QFile::copy(target, destination);
}
