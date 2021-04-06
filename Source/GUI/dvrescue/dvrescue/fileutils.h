#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>

class FileUtils : public QObject
{
    Q_OBJECT
public:
    explicit FileUtils(QObject *parent = nullptr);

    Q_INVOKABLE QStringList getFiles(const QString& dirPath, const QString& extension);
    Q_INVOKABLE QString getFile(const QString& dirPath, const QString& extension);
    Q_INVOKABLE QString getFileName(const QString& filePath);
    Q_INVOKABLE QString getFilePath(const QString& urlOrPath);
    Q_INVOKABLE QString getFileExtension(const QString& filePath);
    Q_INVOKABLE QString getCompleteFileExtension(const QString& filePath);
    Q_INVOKABLE QString toLocalUrl(const QString& filePath);
    Q_INVOKABLE bool exists(const QString& filePath);
    Q_INVOKABLE QString read(const QString& filePath);
    Q_INVOKABLE bool remove(const QString& filePath);
    Q_INVOKABLE bool copy(const QString& target, const QString& destination);
    Q_INVOKABLE QString find(const QString& what);
public Q_SLOTS:
};

#endif // FILEUTILS_H
