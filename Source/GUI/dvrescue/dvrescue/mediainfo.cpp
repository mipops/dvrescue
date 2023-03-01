#include "mediainfo.h"
#include "xmlparser.h"
#include <QDebug>
#include <QFileInfo>
#include <QPointF>
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

int MediaInfo::staCount() const
{
    return m_staCount;
}

int MediaInfo::staSum() const
{
    return m_staSum;
}

int MediaInfo::audSum() const
{
    return m_audSum;
}

QString MediaInfo::frameError() const
{
    return m_frameError;
}

QString MediaInfo::videoBlockError() const
{
    return m_videoBlockError;
}

QString MediaInfo::audioBlockError() const
{
    return m_audioBlockError;
}

int MediaInfo::totalVideoBlocks() const
{
    return m_totalVideoBlocks;
}

int MediaInfo::totalAudioBlocks() const
{
    return m_totalAudioBlocks;
}

int MediaInfo::evenStaSum() const
{
    return m_evenStaSum;
}

int MediaInfo::evenAudSum() const
{
    return m_evenAudSum;
}

QPointF MediaInfo::videoBlockErrorValue() const
{
    return m_videoBlockErrorValue;
}

QPointF MediaInfo::audioBlockErrorValue() const
{
    return m_audioBlockErrorValue;
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
        qDebug() << "MediaInfo::resolve thread finished";
        m_parser->deleteLater();
    });
    connect(m_thread.get(), &QThread::started, m_thread.get(), [this]() {
        setParsing(true);

        m_parser->exec(reportPath());
        qDebug() << "exiting loop";
    }, Qt::DirectConnection);
    connect(m_parser, &XmlParser::finished, m_parser, [this]() {
        setParsing(false);
        qDebug() << "MediaInfo::resolve parser finished: " << reportPath();
    }, Qt::DirectConnection);

    connect(m_parser, &XmlParser::gotMedia, m_parser, [this](auto ref, auto format, auto fileSize) {
        Q_UNUSED(ref);

        setFormat(format);
        setFileSize(fileSize);
    }, Qt::DirectConnection);

    connect(m_parser, &XmlParser::gotFrames, m_parser, [this](auto count, auto diff_seq_count) {
        setFrameCount(frameCount() + count);
        setCountOfFrameSequences(countOfFrameSequences() + 1);

        setTotalVideoBlocks(totalVideoBlocks() + diff_seq_count * video_blocks_per_diff_seq * count);
        setTotalAudioBlocks(totalAudioBlocks() + diff_seq_count * audio_blocks_per_diff_seq * count);
    }, Qt::DirectConnection);

    connect(m_parser, &XmlParser::bytesProcessed, m_parser, [this](auto bytesProcessed) {
       setBytesProcessed(bytesProcessed);
    }, Qt::DirectConnection);

    connect(m_parser, &XmlParser::gotFrameAttributes, m_parser, [this](auto frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes, int diff_seq_count,
            int staCount, int totalSta, int totalEvenSta, int totalAud, int totalEvenAud, bool captionOn, bool isSubstantial) {
        Q_UNUSED(frameNumber);
        Q_UNUSED(isSubstantial);
        Q_UNUSED(framesAttributes);
        Q_UNUSED(diff_seq_count);
        Q_UNUSED(totalSta);
        Q_UNUSED(totalEvenSta);
        Q_UNUSED(totalAud);
        Q_UNUSED(totalEvenAud);
        Q_UNUSED(captionOn);

        setStaCount(this->staCount() + staCount);
        setStaSum(this->staSum() + totalSta);
        setAudSum(this->audSum() + totalAud);
        setEvenStaSum(this->evenStaSum() + totalEvenSta);
        setEvenAudSum(this->evenAudSum() + totalEvenAud);

        if(frameCount() != 0) {
            setFrameError((QString::number(float(this->staCount()) / frameCount() * 100, 'f', 3) + QString("%")));
        }

        if(totalVideoBlocks() != 0) {
            setVideoBlockError((QString::number(float(this->staSum()) / totalVideoBlocks() * 100, 'f', 3) + QString("%")));
            setVideoBlockErrorValue(QPointF(float(this->evenStaSum()) / totalVideoBlocks() * 2, float(this->staSum() - this->evenStaSum()) / totalVideoBlocks() * 2));
        }

        if(totalAudioBlocks() != 0) {
            setAudioBlockError((QString::number(float(this->audSum()) / totalAudioBlocks() * 100, 'f', 3) + QString("%")));
            setAudioBlockErrorValue(QPointF(float(this->evenAudSum()) / totalAudioBlocks() * 2, float(this->audSum() - this->evenAudSum()) / totalAudioBlocks() * 2));
        }

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
    }, Qt::DirectConnection);

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

void MediaInfo::setStaCount(int staCount)
{
    if (m_staCount == staCount)
        return;

    m_staCount = staCount;
    Q_EMIT staCountChanged(m_staCount);
}

void MediaInfo::setStaSum(int staSum)
{
    if (m_staSum == staSum)
        return;

    m_staSum = staSum;
    Q_EMIT staSumChanged(m_staSum);
}

void MediaInfo::setAudSum(int audSum)
{
    if (m_audSum == audSum)
        return;

    m_audSum = audSum;
    Q_EMIT audSumChanged(m_audSum);
}

void MediaInfo::setFrameError(QString frameError)
{
    if (m_frameError == frameError)
        return;

    m_frameError = frameError;
    Q_EMIT frameErrorChanged(m_frameError);
}

void MediaInfo::setVideoBlockError(QString videoBlockError)
{
    if (m_videoBlockError == videoBlockError)
        return;

    m_videoBlockError = videoBlockError;
    Q_EMIT videoBlockErrorChanged(m_videoBlockError);
}

void MediaInfo::setAudioBlockError(QString audioBlockError)
{
    if (m_audioBlockError == audioBlockError)
        return;

    m_audioBlockError = audioBlockError;
    Q_EMIT audioBlockErrorChanged(m_audioBlockError);
}

void MediaInfo::setTotalVideoBlocks(int totalVideoBlocks)
{
    if (m_totalVideoBlocks == totalVideoBlocks)
        return;

    m_totalVideoBlocks = totalVideoBlocks;
    Q_EMIT totalVideoBlocksChanged(m_totalVideoBlocks);
}

void MediaInfo::setTotalAudioBlocks(int totalAudioBlocks)
{
    if (m_totalAudioBlocks == totalAudioBlocks)
        return;

    m_totalAudioBlocks = totalAudioBlocks;
    Q_EMIT totalAudioBlocksChanged(m_totalAudioBlocks);
}

void MediaInfo::setEvenStaSum(int evenStaSum)
{
    if (m_evenStaSum == evenStaSum)
        return;

    m_evenStaSum = evenStaSum;
    Q_EMIT evenStaSumChanged(m_evenStaSum);
}

void MediaInfo::setEvenAudSum(int evenAudSum)
{
    if (m_evenAudSum == evenAudSum)
        return;

    m_evenAudSum = evenAudSum;
    Q_EMIT evenAudSumChanged(m_evenAudSum);
}

void MediaInfo::setVideoBlockErrorValue(QPointF videoBlockErrorValue)
{
    if (m_videoBlockErrorValue == videoBlockErrorValue)
        return;

    m_videoBlockErrorValue = videoBlockErrorValue;
    Q_EMIT videoBlockErrorValueChanged(m_videoBlockErrorValue);
}

void MediaInfo::setAudioBlockErrorValue(QPointF audioBlockErrorValue)
{
    if (m_audioBlockErrorValue == audioBlockErrorValue)
        return;

    m_audioBlockErrorValue = audioBlockErrorValue;
    Q_EMIT audioBlockErrorValueChanged(m_audioBlockErrorValue);
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
