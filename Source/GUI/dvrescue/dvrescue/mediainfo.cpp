#include "mediainfo.h"
#include "xmlparser.h"
#include <QDebug>
#include <QFileInfo>
#include <QXmlStreamAttribute>

MediaInfo::MediaInfo(QObject *parent) : QObject(parent)
{
    qDebug() << "MediaInfo";
}

MediaInfo::~MediaInfo()
{
    qDebug() << "~MediaInfo";

    if(m_thread)
    {
        qDebug() << "request interruption";
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();
    }
}

QString MediaInfo::reportPath() const
{
    return m_reportPath;
}

QString MediaInfo::videoPath() const
{
    return m_videoPath;
}

QString MediaInfo::format() const
{
    return m_format;
}

int MediaInfo::fileSize() const
{
    return m_fileSize;
}

int MediaInfo::frameCount() const
{
    return m_frameCount;
}

int MediaInfo::countOfFrameSequences() const
{
    return m_countOfFrameSequences;
}

QString MediaInfo::firstTimeCode() const
{
    return m_firstTimeCode;
}

QString MediaInfo::lastTimeCode() const
{
    return m_lastTimeCode;
}

QString MediaInfo::firstRecordingTime() const
{
    return m_firstRecordingTime;
}

QString MediaInfo::lastRecordingTime() const
{
    return m_lastRecordingTime;
}

int MediaInfo::reportFileSize() const
{
    return m_reportFileSize;
}

int MediaInfo::bytesProcessed() const
{
    return m_bytesProcessed;
}

bool MediaInfo::parsing() const
{
    return m_parsing;
}

void MediaInfo::setReportPath(QString reportPath)
{
    if (m_reportPath == reportPath)
        return;

    m_reportPath = reportPath;
    qDebug() << "reportPath: " << reportPath;
    Q_EMIT reportPathChanged(m_reportPath);

    QFileInfo info(this->reportPath());
    setReportFileSize(info.size());
}

void MediaInfo::setVideoPath(QString videoPath)
{
    if (m_videoPath == videoPath)
        return;

    m_videoPath = videoPath;
    qDebug() << "videoPath" << videoPath;
    Q_EMIT videoPathChanged(m_videoPath);
}

void MediaInfo::resolve()
{
    qDebug() << "resolving media info: " << reportPath();

    if(m_thread)
    {
        qDebug() << "request interruption";
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();
    }

    m_parser = new XmlParser();
    m_thread.reset(new QThread());

    m_parser->moveToThread(m_thread.get());
    connect(m_thread.get(), &QThread::finished, [this]() {
        qDebug() << "finished";
        m_parser->deleteLater();
    });
    connect(m_thread.get(), &QThread::started, [this]() {
        setParsing(true);

        m_parser->exec(reportPath());
        qDebug() << "exiting loop";
    });
    connect(m_parser, &XmlParser::finished, [this]() {
        setParsing(false);

        qDebug() << "parser finished: " << reportPath();
    });
    connect(m_parser, &XmlParser::gotMedia, [this](auto ref, auto format, auto fileSize) {
        setFormat(format);
        setFileSize(fileSize);
    });

    connect(m_parser, &XmlParser::gotFrames, [this](auto count) {
        setFrameCount(frameCount() + count);
        setCountOfFrameSequences(countOfFrameSequences() + 1);
    });

    connect(m_parser, &XmlParser::bytesProcessed, [this](auto bytesProcessed) {
       setBytesProcessed(bytesProcessed);
    });

    connect(m_parser, &XmlParser::gotFrameAttributes, [this](auto frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes, int diff_seq_count,
            int totalSta, int totalEvenSta, int totalAud, int totalEvenAud, bool captionOn, bool isSubstantial) {
        Q_UNUSED(isSubstantial);

        auto tc = frameAttributes.hasAttribute("tc") ? frameAttributes.value("tc").toString() : QString();

        if(m_firstTimeCode.isEmpty()) {
            setFirstTimeCode(tc);
        }

        setLastTimeCode(tc);

        auto rdt = frameAttributes.hasAttribute("rdt") ? frameAttributes.value("rdt").toString() : QString();

        if(m_firstRecordingTime.isEmpty()) {
            setFirstRecordingTime(rdt);
        }

        setLastRecordingTime(rdt);
    });

    m_thread->start();
}

void MediaInfo::setFormat(QString format)
{
    if (m_format == format)
        return;

    m_format = format;
    Q_EMIT formatChanged(m_format);
}

void MediaInfo::setFileSize(int fileSize)
{
    if (m_fileSize == fileSize)
        return;

    m_fileSize = fileSize;
    Q_EMIT fileSizeChanged(m_fileSize);
}

void MediaInfo::setFrameCount(int frameCount)
{
    if (m_frameCount == frameCount)
        return;

    m_frameCount = frameCount;
    Q_EMIT frameCountChanged(m_frameCount);
}

void MediaInfo::setCountOfFrameSequences(int countOfFrameSequences)
{
    if (m_countOfFrameSequences == countOfFrameSequences)
        return;

    m_countOfFrameSequences = countOfFrameSequences;
    Q_EMIT countOfFrameSequencesChanged(m_countOfFrameSequences);
}

void MediaInfo::setFirstTimeCode(QString firstTimeCode)
{
    if (m_firstTimeCode == firstTimeCode)
        return;

    m_firstTimeCode = firstTimeCode;
    Q_EMIT firstTimeCodeChanged(m_firstTimeCode);
}

void MediaInfo::setLastTimeCode(QString lastTimeCode)
{
    if (m_lastTimeCode == lastTimeCode)
        return;

    m_lastTimeCode = lastTimeCode;
    Q_EMIT lastTimeCodeChanged(m_lastTimeCode);
}

void MediaInfo::setFirstRecordingTime(QString firstRecordingTime)
{
    if (m_firstRecordingTime == firstRecordingTime)
        return;

    m_firstRecordingTime = firstRecordingTime;
    Q_EMIT firstRecordingTimeChanged(m_firstRecordingTime);
}

void MediaInfo::setLastRecordingTime(QString lastRecordingTime)
{
    if (m_lastRecordingTime == lastRecordingTime)
        return;

    m_lastRecordingTime = lastRecordingTime;
    Q_EMIT lastRecordingTimeChanged(m_lastRecordingTime);
}

void MediaInfo::setParsing(bool parsing)
{
    if (m_parsing == parsing)
        return;

    m_parsing = parsing;
    Q_EMIT parsingChanged(m_parsing);
}

void MediaInfo::setReportFileSize(qint64 reportFileSize)
{
    if (m_reportFileSize == reportFileSize)
        return;

    m_reportFileSize = reportFileSize;
    Q_EMIT reportFileSizeChanged(m_reportFileSize);
}

void MediaInfo::setBytesProcessed(qint64 bytesProcessed)
{
    if (m_bytesProcessed == bytesProcessed)
        return;

    m_bytesProcessed = bytesProcessed;
    Q_EMIT bytesProcessedChanged(m_bytesProcessed);
}
