#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QObject>
#include <QThread>
#include <memory>

class XmlParser;
class MediaInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString reportPath READ reportPath WRITE setReportPath NOTIFY reportPathChanged)
    Q_PROPERTY(QString videoPath READ videoPath WRITE setVideoPath NOTIFY videoPathChanged)
    Q_PROPERTY(QString format READ format WRITE setFormat NOTIFY formatChanged)
    Q_PROPERTY(int fileSize READ fileSize WRITE setFileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(int frameCount READ frameCount WRITE setFrameCount NOTIFY frameCountChanged)
    Q_PROPERTY(int countOfFrameSequences READ countOfFrameSequences WRITE setCountOfFrameSequences NOTIFY countOfFrameSequencesChanged)
    Q_PROPERTY(QString firstTimeCode READ firstTimeCode WRITE setFirstTimeCode NOTIFY firstTimeCodeChanged)
    Q_PROPERTY(QString lastTimeCode READ lastTimeCode WRITE setLastTimeCode NOTIFY lastTimeCodeChanged)
    Q_PROPERTY(QString firstRecordingTime READ firstRecordingTime WRITE setFirstRecordingTime NOTIFY firstRecordingTimeChanged)
    Q_PROPERTY(QString lastRecordingTime READ lastRecordingTime WRITE setLastRecordingTime NOTIFY lastRecordingTimeChanged)

    Q_PROPERTY(qint64 reportFileSize READ reportFileSize NOTIFY reportFileSizeChanged)
    Q_PROPERTY(qint64 bytesProcessed READ bytesProcessed NOTIFY bytesProcessedChanged)
    Q_PROPERTY(bool parsing READ parsing NOTIFY parsingChanged)
public:
    explicit MediaInfo(QObject *parent = nullptr);
    ~MediaInfo();

    QString reportPath() const;
    QString videoPath() const;
    QString format() const;
    int fileSize() const;
    int frameCount() const;
    int countOfFrameSequences() const;
    QString firstTimeCode() const;
    QString lastTimeCode() const;
    QString firstRecordingTime() const;
    QString lastRecordingTime() const;

    int reportFileSize() const;
    int bytesProcessed() const;

    bool parsing() const;

private:
    void setReportFileSize(qint64 reportFileSize);
    void setBytesProcessed(qint64 bytesProcessed);
    void setParsing(bool parsing);

private:
    QString m_reportPath;
    QString m_videoPath;
    QString m_format;
    int m_fileSize { 0 };
    int m_frameCount { 0 };
    int m_countOfFrameSequences { 0 };
    QString m_firstTimeCode;
    QString m_lastTimeCode;
    QString m_firstRecordingTime;
    QString m_lastRecordingTime;

    XmlParser* m_parser { nullptr };
    std::unique_ptr<QThread> m_thread;

    qint64 m_reportFileSize { 0 };
    qint64 m_bytesProcessed { 0 };

    bool m_parsing { false };

public Q_SLOTS:
    void resolve();

    void setReportPath(QString reportPath);
    void setVideoPath(QString videoPath);
    void setFormat(QString format);
    void setFileSize(int fileSize);
    void setFrameCount(int frameCount);
    void setCountOfFrameSequences(int countOfFrameSequences);
    void setFirstTimeCode(QString firstTimeCode);
    void setLastTimeCode(QString lastTimeCode);
    void setFirstRecordingTime(QString firstRecordingTime);
    void setLastRecordingTime(QString lastRecordingTime);

Q_SIGNALS:
    void reportPathChanged(QString reportPath);
    void videoPathChanged(QString videoPath);
    void formatChanged(QString format);
    void fileSizeChanged(int fileSize);
    void frameCountChanged(int frameCount);
    void countOfFrameSequencesChanged(int countOfFrameSequences);
    void firstTimeCodeChanged(QString firstTimeCode);
    void lastTimeCodeChanged(QString lastTimeCode);
    void firstRecordingTimeChanged(QString firstRecordingTime);
    void lastRecordingTimeChanged(QString lastRecordingTime);
    void reportFileSizeChanged(int reportFileSize);
    void bytesProcessedChanged(int bytesProcessed);
    void parsingChanged(bool parsing);
};

#endif // MEDIAINFO_H
