#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QObject>
#include <QPointF>
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

    Q_PROPERTY(int staCount READ staCount WRITE setStaCount NOTIFY staCountChanged)
    Q_PROPERTY(int staSum READ staSum WRITE setStaSum NOTIFY staSumChanged)
    Q_PROPERTY(int evenStaSum READ evenStaSum WRITE setEvenStaSum NOTIFY evenStaSumChanged)
    Q_PROPERTY(int audSum READ audSum WRITE setAudSum NOTIFY audSumChanged)
    Q_PROPERTY(int evenAudSum READ evenAudSum WRITE setEvenAudSum NOTIFY evenAudSumChanged)
    Q_PROPERTY(int totalVideoBlocks READ totalVideoBlocks WRITE setTotalVideoBlocks NOTIFY totalVideoBlocksChanged)
    Q_PROPERTY(int totalAudioBlocks READ totalAudioBlocks WRITE setTotalAudioBlocks NOTIFY totalAudioBlocksChanged)

    Q_PROPERTY(QString frameError READ frameError WRITE setFrameError NOTIFY frameErrorChanged)
    Q_PROPERTY(QString videoBlockError READ videoBlockError WRITE setVideoBlockError NOTIFY videoBlockErrorChanged)
    Q_PROPERTY(QPointF videoBlockErrorValue READ videoBlockErrorValue WRITE setVideoBlockErrorValue NOTIFY videoBlockErrorValueChanged)
    Q_PROPERTY(QString audioBlockError READ audioBlockError WRITE setAudioBlockError NOTIFY audioBlockErrorChanged)
    Q_PROPERTY(QPointF audioBlockErrorValue READ audioBlockErrorValue WRITE setAudioBlockErrorValue NOTIFY audioBlockErrorValueChanged)

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

    int staCount() const;
    int staSum() const;
    int audSum() const;

    QString frameError() const;
    QString videoBlockError() const;
    QString audioBlockError() const;

    int totalVideoBlocks() const;
    int totalAudioBlocks() const;

    int evenStaSum() const;
    int evenAudSum() const;

    QPointF videoBlockErrorValue() const;
    QPointF audioBlockErrorValue() const;

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

    QString m_frameError;
    QString m_videoBlockError;
    QString m_audioBlockError;

    int m_staCount { 0 };
    int m_staSum { 0 };
    int m_audSum { 0 };

    int m_totalVideoBlocks { 0 };
    int m_totalAudioBlocks { 0 };

    int m_evenStaSum { 0 };
    int m_evenAudSum { 0 };

    QPointF m_videoBlockErrorValue;
    QPointF m_audioBlockErrorValue;

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

    void setStaCount(int staCount);
    void setStaSum(int staSum);
    void setAudSum(int audSum);

    void setFrameError(QString frameError);
    void setVideoBlockError(QString videoBlockError);
    void setAudioBlockError(QString audioBlockError);

    void setTotalVideoBlocks(int totalVideoBlocks);
    void setTotalAudioBlocks(int totalAudioBlocks);

    void setEvenStaSum(int evenStaSum);
    void setEvenAudSum(int evenAudSum);

    void setVideoBlockErrorValue(QPointF videoBlockErrorValue);
    void setAudioBlockErrorValue(QPointF audioBlockErrorValue);

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
    void staCountChanged(int staCount);
    void staSumChanged(int staSum);
    void audSumChanged(int audSum);
    void frameErrorChanged(QString frameError);
    void videoBlockErrorChanged(QString videoBlockError);
    void audioBlockErrorChanged(QString audioBlockError);
    void totalVideoBlocksChanged(int totalVideoBlocks);
    void totalAudioBlocksChanged(int totalAudioBlocks);
    void evenStaSumChanged(int evenStaSum);
    void evenAudSumChanged(int evenAudSum);
    void videoBlockErrorValueChanged(QPointF videoBlockErrorValue);
    void audioBlockErrorValueChanged(QPointF audioBlockErrorValue);
};

#endif // MEDIAINFO_H
