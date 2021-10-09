#include "fileutils.h"
#include <QDir>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

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
    if(urlOrPath.isEmpty())
        return urlOrPath;

    QUrl url(urlOrPath);

    return url.toLocalFile();
}

QString FileUtils::getFileDir(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    return fileInfo.absoluteDir().absolutePath();
}

QString FileUtils::getFileExtension(const QString &filePath)
{
    QFileInfo info(filePath);

    return info.suffix();
}

QString FileUtils::getCompleteFileExtension(const QString &filePath)
{
    QFileInfo info(filePath);

    return info.completeSuffix();
}

QString FileUtils::toLocalUrl(const QString &filePath)
{
    QUrl url(QUrl::fromLocalFile(filePath));

    return url.toString();
}

bool FileUtils::exists(const QString &filePath)
{
    QFileInfo info(filePath);
    return info.exists();
}

QStringList FileUtils::ls(const QString &dirPath)
{
    QDir dir(dirPath);
    return dir.entryList();
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

QString FileUtils::find(const QString &what)
{
    qDebug() << "FileUtils::find: " << what;

    QByteArray pEnv = qgetenv("PATH");
    const QStringList rawPaths = QString::fromLocal8Bit(pEnv.constData()).split(QDir::listSeparator(), QString::SkipEmptyParts);

    QStringList searchPaths;
    searchPaths.reserve(rawPaths.size());
    for (const QString &rawPath : rawPaths) {
        QString cleanPath = QDir::cleanPath(rawPath);
        if (cleanPath.size() > 1 && cleanPath.endsWith(QLatin1Char('/')))
            cleanPath.truncate(cleanPath.size() - 1);
        searchPaths.push_back(cleanPath);
    }

    const QDir currentDir = QDir::current();
    for (const QString &searchPath : searchPaths) {
        const QString candidate = currentDir.absoluteFilePath(searchPath + QLatin1Char('/') + what);
        qDebug() << "checking path: " << candidate;
        if (QFileInfo::exists(candidate)) {
            qDebug() << "FileUtils::find finished: " << what;
            return QUrl::fromLocalFile(candidate).toString();
        }
    }

    qDebug() << "FileUtils::find finished: nothing found";
    return QString();
}
