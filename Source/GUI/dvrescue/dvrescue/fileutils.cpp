#include "fileutils.h"

#if defined(Q_OS_MAC)
#include "machelpers.h"
#endif
#include <QDir>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
#include <QImage>
#include <QFileDialog>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using SplitBehaviour = QString::SplitBehavior;
#else
using SplitBehaviour = Qt::SplitBehaviorFlags;
#endif //

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

QString FileUtils::getFilePath(const QString &urlOrPath, bool convertToNative)
{
    if(urlOrPath.isEmpty())
        return urlOrPath;

    QUrl url(urlOrPath);
    auto filePath = QFile::exists(urlOrPath) ? urlOrPath : url.toLocalFile();

    if(convertToNative)
        filePath = QDir::toNativeSeparators(filePath);

    return filePath;
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

bool FileUtils::isWritable(const QString& dirPath)
{
    QDir dir(dirPath);
    if (dir.exists())
    {
        QFile file(dir.absoluteFilePath(".dvrescue_write_test"));
        if(file.open(QIODevice::WriteOnly))
        {
            file.remove();
            return true;
        }
    }

    return false;
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

QByteArray FileUtils::readBinary(const QString &filePath)
{
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        return file.readAll();
    }

    return QByteArray();
}

void FileUtils::write(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << content;
    }
}

void FileUtils::write(const QString &filePath, const QByteArray &content)
{
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(content);
    }
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
    const QStringList rawPaths = QString::fromLocal8Bit(pEnv.constData()).split(QDir::listSeparator(), SplitBehaviour::SkipEmptyParts);

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

QString FileUtils::find(const QString &path, const QString &what)
{
    QStringList filters;
    filters << what;

    QDir dir(path);
    auto entries = dir.entryInfoList(filters);

    return entries.empty() ? QString() : entries[0].absoluteFilePath();
}

bool FileUtils::requestRWPermissionsForPath(const QString& dirPath, const QString& message)
{
    QString dir;
    #if defined(Q_OS_MAC)
    dir = sandboxQueryRWPermissionForPath(dirPath, message);
    #else
    dir = QFileDialog::getExistingDirectory(nullptr, message, dirPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    #endif
    qInfo() << "requestRWPermissionsForPath request: " << dirPath << " response: " << dir;
    return QFileInfo(dir)==QFileInfo(dirPath);
}
