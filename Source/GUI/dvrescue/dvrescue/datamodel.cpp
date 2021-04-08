#include "datamodel.h"
#include "qwtquick2plot.h"
#include <QThread>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtAlgorithms>
#include <QJSEngine>

DataModel::DataModel(QObject *parent) : QObject(parent)
{
    connect(this, &DataModel::dataRowCreated, this, &DataModel::onDataRowCreated);
}

DataModel::~DataModel()
{
    if(m_thread)
    {
        qDebug() << "request interruption";
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();
    }
}

int DataModel::total() const
{
    return m_total;
}

QString DataModel::videoInfo(float x, float y)
{
    Q_UNUSED(y);

    auto frameOffset = qRound(x);
    int closestFrame = -1;
    float evenValue = 0;
    float oddValue = 0;

    getVideoInfo(x, y, closestFrame, oddValue, evenValue);
    if(closestFrame == -1)
        return QString();

    if(frameOffset == closestFrame)
        return QString("frame: %1\n").arg(frameOffset) + QString("%1% (even DIF sequences %2%, odd %3%)").arg(evenValue + abs(oddValue)).arg(evenValue).arg(oddValue);

    return QString("frame: %1, closest frame: %2\n").arg(frameOffset).arg(closestFrame) + QString("%1% (even DIF sequences %2%, odd %3%)").arg(evenValue + abs(oddValue)).arg(evenValue).arg(oddValue);
}

QString DataModel::audioInfo(float x, float y)
{
    Q_UNUSED(y);

    auto frameOffset = qRound(x);
    int closestFrame = -1;
    float evenValue = 0;
    float oddValue = 0;

    getAudioInfo(x, y, closestFrame, oddValue, evenValue);
    if(closestFrame == -1)
        return QString();

    if(frameOffset == closestFrame)
        return QString("frame: %1\n").arg(frameOffset) + QString("%1% (even DIF sequences %2%, odd %3%)").arg(evenValue + abs(oddValue)).arg(evenValue).arg(oddValue);

    return QString("frame: %1, closest frame: %2\n").arg(frameOffset).arg(closestFrame) + QString("%1% (even DIF sequences %2%, odd %3%)").arg(evenValue + abs(oddValue)).arg(evenValue).arg(oddValue);
}

QVariantList DataModel::getMarkers()
{
    QVariantList markers;

    for(auto & frameTuple : m_frames) {
        auto frameIndex = std::get<0>(frameTuple);
        auto& frameInfo = std::get<1>(frameTuple);

        if(frameInfo.markers.empty())
            continue;

        for(auto& key : frameInfo.markers.keys()) {
            auto type = key;
            auto nameAndIcon = frameInfo.markers.value(key);

            auto markerInfo = MarkerInfo();
            markerInfo.frameNumber = frameIndex;
            markerInfo.name = nameAndIcon.first;
            markerInfo.type = type;
            markerInfo.icon = nameAndIcon.second;
            markerInfo.recordingTime = frameInfo.recordingTime;
            markerInfo.timecode = frameInfo.timecode;

            markers.append(QVariant::fromValue(markerInfo));
        }
    }

    return markers;
}

int DataModel::frameByIndex(int index)
{
    if(index < 0 || index >= m_frames.size())
        return index;

    return std::get<0>(m_frames[index]);
}

bool DataModel::isSubstantialFrame(int index)
{
    if(index < 0 || index >= m_frames.size())
        return false;

    return std::get<1>(m_frames[index]).isSubstantial;
}

int DataModel::getLastSubstantialFrame(int index)
{
    if(index < 0 || index >= m_frames.size())
        return -1;

    return std::get<1>(m_frames[index]).lastSubstantialFrame;
}

QString DataModel::getLastSubstantialFrameTransition(int index)
{
    if(index < 0 || index >= m_frames.size())
        return QString();

    auto frameInfoTuple = m_frames.at(index);
    auto frameNumber = std::get<0>(frameInfoTuple);

    auto lastSubstantialFrameInfoTuple = m_frames.at(m_rowByFrame[std::get<1>(frameInfoTuple).lastSubstantialFrame]);
    auto lastSubstantialFrameNumber = std::get<0>(lastSubstantialFrameInfoTuple);

    qDebug() << "frameNumber: " << frameNumber << "lastSubstantialFrameNumber: " << lastSubstantialFrameNumber;

    auto frameInfo = std::get<1>(frameInfoTuple);
    auto lastSubstantialFrameInfo = std::get<1>(lastSubstantialFrameInfoTuple);

    return QString("From ") + lastSubstantialFrameInfo.videoInfo + " " + lastSubstantialFrameInfo.audioInfo + " to "
        + frameInfo.videoInfo + " " + frameInfo.audioInfo;
}

int DataModel::rowByFrame(int frame)
{
    return m_rowByFrame.contains(frame) ? m_rowByFrame[frame] : -1;
}

void DataModel::getVideoInfo(float x, float y, int &frame, float &oddValue, float &evenValue)
{
    return getInfo(m_videoValues, x, y, frame, oddValue, evenValue);
}

void DataModel::getAudioInfo(float x, float y, int &frame, float &oddValue, float &evenValue)
{
    return getInfo(m_audioValues, x, y, frame, oddValue, evenValue);
}

QJSEngine* engine = nullptr;

void DataModel::setEngine(QJSEngine *jsEngine)
{
    engine = jsEngine;
}

void DataModel::getInfo(QList<std::tuple<int, DataModel::GraphStats> > &stats, float x, float y, int &closestFrame, float &oddValue, float &evenValue)
{
    Q_UNUSED(y);

    if(stats.empty())
        return;

    auto frameOffset = qRound(x);
    auto frameTuple = std::make_tuple(frameOffset, GraphStats());

    auto lower = std::lower_bound(stats.rbegin(), stats.rend(), frameTuple, [&](const std::tuple<int, GraphStats>& first, const std::tuple<int, GraphStats>& second) {
        auto& f = std::get<0>(first);
        auto& s = std::get<0>(second);

        return f > s;
    });

    auto higher = std::upper_bound(stats.begin(), stats.end(), frameTuple, [&](const std::tuple<int, GraphStats>& first, const std::tuple<int, GraphStats>& second) {
        auto& f = std::get<0>(first);
        auto& s = std::get<0>(second);

        return f < s;
    });

    // qDebug() << "frameOffset: " << frameOffset;

    if(higher != stats.end() && lower != stats.rend())
    {
        auto& higherValue = std::get<1>(*higher);
        auto& lowerValue = std::get<1>(*lower);

        // qDebug() << "higherValue.frameNumber: " << higherValue.frameNumber;
        // qDebug() << "lowerValue.frameNumber: " << lowerValue.frameNumber;

        auto hdx = higherValue.frameNumber - frameOffset;
        auto ldx = lowerValue.frameNumber - frameOffset;

        if(abs(hdx) < abs(ldx)) {
            closestFrame = higherValue.frameNumber;
            evenValue = higherValue.evenValue;
            oddValue = higherValue.oddValue;
        } else {
            closestFrame = lowerValue.frameNumber;
            evenValue = lowerValue.evenValue;
            oddValue = lowerValue.oddValue;
        }
    }
    else if(higher != stats.end())
    {
        auto& higherValue = std::get<1>(*higher);

        // qDebug() << "higherValue.frameNumber: " << higherValue.frameNumber;

        closestFrame = higherValue.frameNumber;
        evenValue = higherValue.evenValue;
        oddValue = higherValue.oddValue;
    }
    else if(lower != stats.rend())
    {
        auto& lowerValue = std::get<1>(*lower);

        // qDebug() << "lowerValue.frameNumber: " << lowerValue.frameNumber;

        closestFrame = lowerValue.frameNumber;
        evenValue = lowerValue.evenValue;
        oddValue = lowerValue.oddValue;
    }
}

void DataModel::update(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve *videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve *audioCurve2)
{
    videoCurve->plot()->plot()->setUpdatesEnabled(false);
    videoCurve->plot()->plot()->setAxisScale(QwtPlot::yLeft, -50, 50);
    videoCurve->plot()->setXBottomAxisRange(QVector2D(0, m_lastFrame));

    auto videoCount = videoCurve->data().count();
    for(auto i = videoCount; i < m_videoValues.count(); ++i) {
        auto& valueTuple = m_videoValues.at(i);

        auto frameNumber = std::get<0>(valueTuple);
        auto value = std::get<1>(valueTuple);

        videoCurve->data().append(QPointF(frameNumber, value.evenValue));
        videoCurve2->data().append(QPointF(frameNumber, value.oddValue));
    }

    videoCurve->plot()->plot()->setUpdatesEnabled(true);
    videoCurve->plot()->replotAndUpdate();

    audioCurve->plot()->plot()->setUpdatesEnabled(false);
    audioCurve->plot()->plot()->setAxisScale(QwtPlot::yLeft, -50, 50);
    audioCurve->plot()->setXBottomAxisRange(QVector2D(0, m_lastFrame));

    auto audioCount = audioCurve->data().count();
    for(auto i = audioCount; i < m_audioValues.count(); ++i) {
        auto& valueTuple = m_audioValues.at(i);

        auto frameNumber = std::get<0>(valueTuple);
        auto value = std::get<1>(valueTuple);

        audioCurve->data().append(QPointF(frameNumber, value.evenValue));
        audioCurve2->data().append(QPointF(frameNumber, value.oddValue));
    }

    audioCurve->plot()->plot()->setUpdatesEnabled(true);
    audioCurve->plot()->replotAndUpdate();

    Q_EMIT updated();
}

void DataModel::reset(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve *videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve *audioCurve2)
{
    videoCurve->data().clear();
    videoCurve2->data().clear();
    audioCurve->data().clear();
    audioCurve2->data().clear();

    videoCurve->plot()->replotAndUpdate();
    audioCurve->plot()->replotAndUpdate();

    Q_EMIT clearModel();
}

void DataModel::bind(QAbstractTableModel *model)
{
    if(m_model) {
        disconnect(this, SIGNAL(gotDataRow(const QVariant&)), m_model, SLOT(appendRow(const QVariant&)));
        disconnect(this, SIGNAL(clearModel()), m_model, SLOT(clear()));
    }
    m_model = model;
    if(m_model) {
        connect(this, SIGNAL(gotDataRow(const QVariant&)), m_model, SLOT(appendRow(const QVariant&)));
        connect(this, SIGNAL(clearModel()), m_model, SLOT(clear()));
    }
}

void DataModel::populate(const QString &fileName)
{
    if(m_thread)
    {
        qDebug() << "request interruption";
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();

        m_frames.clear();
        m_rowByFrame.clear();
        m_videoValues.clear();
        m_audioValues.clear();
    }

    qDebug() << "DataModel::populate: " << QThread::currentThread();

    m_lastFrame = 0;
    m_lastSubstantialFrame = -1;
    m_total = 0;
    m_parser = new XmlParser();
    m_thread.reset(new QThread());

    m_parser->moveToThread(m_thread.get());
    connect(m_thread.get(), &QThread::finished, [this]() {
        qDebug() << "finished";
        m_parser->deleteLater();
    });
    connect(m_thread.get(), &QThread::started, [this, fileName]() {
        m_parser->exec(fileName);
        qDebug() << "exiting loop";
    });
    connect(m_parser, &XmlParser::finished, [this]() {
        qDebug() << "parser finished";
        Q_EMIT populated();
    });

    connect(m_parser, &XmlParser::gotFrame, [this](auto frameNumber) {
        m_frames.append(std::make_tuple(frameNumber, FrameStats() ));
    });

    connect(m_parser, &XmlParser::gotFrameAttributes, [this](auto frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes, int diff_seq_count,
            int staCount, int totalSta, int totalEvenSta, int totalAud, int totalEvenAud, bool captionOn, bool isSubstantial) {
        Q_UNUSED(staCount);
        Q_UNUSED(isSubstantial);

        m_lastFrame = frameNumber;

        if(m_lastSubstantialFrame == -1)
            m_lastSubstantialFrame = frameNumber;

        m_total = m_lastFrame + 1;
        auto& frameStats = std::get<1>(m_frames.back());
        frameStats.isSubstantial = isSubstantial;
        frameStats.lastSubstantialFrame = m_lastSubstantialFrame;
        m_rowByFrame[frameNumber] = m_frames.length() - 1;

        auto recStart = (frameAttributes.hasAttribute("rec_start") ? frameAttributes.value("rec_start").toInt() : 0);
        auto recEnd = (frameAttributes.hasAttribute("rec_end") ? frameAttributes.value("rec_end").toInt() : 0);
        if(recStart && recEnd) {
            frameStats.markers["rec"] = std::pair("Recording Start&End", "icons/record-marker-stop+start-graph.svg");
        } else if(recStart) {
            frameStats.markers["rec"] = std::pair("Recording Start", "icons/record-marker-start-graph.svg");
        } else if(recEnd) {
            frameStats.markers["rec"] = std::pair("Recording End", "icons/record-marker-stop-graph.svg");
        }

        if(isSubstantial)
            m_lastSubstantialFrame = frameNumber;

        onGotFrame(frameNumber, framesAttributes, frameAttributes, diff_seq_count, totalSta, totalEvenSta, totalAud, totalEvenAud, captionOn, isSubstantial);
        Q_EMIT totalChanged(m_total);
    });

    connect(m_parser, &XmlParser::gotSta, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        GraphStats value = {
            int(frameNumber),
            float(n_even) / den * 100,
            -float(n - n_even) / den * 100,
            float(den)
        };

        m_videoValues.append(std::make_tuple(frameNumber, value));
    });
    connect(m_parser, &XmlParser::gotAud, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        Q_UNUSED(t);
        GraphStats value = {
            int(frameNumber),
            float(n_even) / den * 100,
            -float(n - n_even) / den * 100,
            float(den)
        };

        m_audioValues.append(std::make_tuple(frameNumber, value));
    });

    m_thread->start();
}

void DataModel::onGotFrame(int frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes, int diff_seq_count,
                           int totalSta, int totalEvenSta, int totalAud, int totalEvenAud, bool captionOn, bool isSubstantional)
{
    Q_UNUSED(isSubstantional);
    auto& frameStats = std::get<1>(m_frames.back());

    // qDebug() << "DataModel::onGotFrame: " << QThread::currentThread();

    QVariantMap map;
    map["Frame #"] = frameNumber;

    auto fillAttribute = [&](const QString& mapKeyName, const QXmlStreamAttributes& attributes, const QString& name, const QString& defaultValue = "") {
        map[mapKeyName] = attributes.hasAttribute(name) ? attributes.value(name).toString() : defaultValue;
    };

    auto getStringAttribute = [&](const QString& name, const QXmlStreamAttributes& attributes) {
        return attributes.hasAttribute(name) ? attributes.value(name).toString() : QString();
    };

    fillAttribute("Byte Offset", frameAttributes, "pos");

    QString timestamp;
    if(frameAttributes.hasAttribute("pts")) {
        auto splitted = frameAttributes.value("pts").toString().split(".");
        timestamp = splitted[0] + "." + splitted[1].mid(0, 2);
    }
    map["Timestamp"] = timestamp;

    fillAttribute("Timecode", frameAttributes, "tc");
    frameStats.timecode = map["Timecode"].toString();

    int timecodeRepeat = 0;
    if(frameAttributes.hasAttribute("tc_r"))
        timecodeRepeat = frameAttributes.value("tc_r").toInt();

    auto timecodeJump = 0;
    if(frameAttributes.hasAttribute("tc_nc")) {
        timecodeJump = frameAttributes.value("tc_nc").toInt();
        frameStats.markers["tc_n"] = std::make_pair("Timecode: Jump", "icons/record-marker-stop+start-graph.svg");
    }

    map["Timecode: Jump/Repeat"] = QPoint(timecodeJump, timecodeRepeat);

    fillAttribute("Recording Time", frameAttributes, "rdt");
    frameStats.recordingTime = map["Recording Time"].toString();
    int recordingTimeRepeat = 0;
    if(frameAttributes.hasAttribute("rdt_r"))
        recordingTimeRepeat = frameAttributes.value("rdt_r").toInt();

    auto recordingTimeJump = 0;
    if(frameAttributes.hasAttribute("rdt_nc"))
        recordingTimeJump = frameAttributes.value("rdt_nc").toInt();

    map["Recording Time: Jump/Repeat"] = QPoint(recordingTimeJump, recordingTimeRepeat);

    auto recStart = (frameAttributes.hasAttribute("rec_start") ? frameAttributes.value("rec_start").toInt() : 0);
    auto recEnd = (frameAttributes.hasAttribute("rec_end") ? frameAttributes.value("rec_end").toInt() : 0);

    map["Recording Marks"] = QPoint(recStart, recEnd);

    fillAttribute("Arbitrary Bits", frameAttributes, "arb");
    int arbitraryBitsRepeat = 0;
    if(frameAttributes.hasAttribute("arb_r"))
        arbitraryBitsRepeat = frameAttributes.value("arb_r").toInt();

    auto arbitraryBitsJump = 0;
    if(frameAttributes.hasAttribute("arb_nc"))
        arbitraryBitsJump = frameAttributes.value("arb_nc").toInt();

    map["Arbitrary Bits: Jump/Repeat"] = QPoint(arbitraryBitsJump, arbitraryBitsRepeat);

    map["CC"] = "";
    if(framesAttributes.hasAttribute("captions"))
    {
        if(framesAttributes.value("captions").toString() == "y")
        {
            map["CC"] = "y";
        }
        else if(framesAttributes.value("captions").toString() == "p")
        {
            if(frameAttributes.hasAttribute("caption"))
            {
                auto caption = frameAttributes.value("caption");
                if(caption == "on")
                {
                    map["CC"] = "┬";
                }
                else if(caption == "off")
                {
                    map["CC"] = "┴";
                }
            }
            else
            {
                if(captionOn)
                {
                    map["CC"] = "│";
                }
            }
        }
    }

    map["CC/Mismatch"] = false;
    if(frameAttributes.hasAttribute("caption-parity")) {
        if(frameAttributes.value("caption-parity").toString() == "mismatch") {
            map["CC/Mismatch"] = true;
        }
    }

    QStringList missingPacks;
    auto no_pack = frameAttributes.hasAttribute("no_pack") && frameAttributes.value("no_pack").toInt() == 1;
    auto no_pack_sub = frameAttributes.hasAttribute("no_pack_sub") && frameAttributes.value("no_pack_sub").toInt() == 1;
    auto no_pack_vid = frameAttributes.hasAttribute("no_pack_vid") && frameAttributes.value("no_pack_vid").toInt() == 1;
    auto no_pack_aud = frameAttributes.hasAttribute("no_pack_aud") && frameAttributes.value("no_pack_aud").toInt() == 1;

    if(no_pack) {
        missingPacks << "Subcode" << "Video" << "Audio";
    } else {
        if(no_pack_sub)
            missingPacks << "Subcode";
        if(no_pack_vid)
            missingPacks << "Video";
        if(no_pack_aud)
            missingPacks << "Audio";
    }

    auto no_sourceorcontrol_vid = frameAttributes.hasAttribute("no_sourceorcontrol_vid") && frameAttributes.value("no_sourceorcontrol_vid").toInt() == 1;
    auto no_sourceorcontrol_aud = frameAttributes.hasAttribute("no_sourceorcontrol_aud") && frameAttributes.value("no_sourceorcontrol_aud").toInt() == 1;

    if(no_sourceorcontrol_vid)
        missingPacks << "No Video Source or Control";
    if(no_sourceorcontrol_aud)
        missingPacks << "No Audio Source or Control";

    map["Missing Packs"] = missingPacks.join(", ");

    QStringList fullConcealment;
    auto full_conceal = frameAttributes.hasAttribute("full_conceal") && frameAttributes.value("full_conceal").toInt() == 1;
    auto full_conceal_vid = frameAttributes.hasAttribute("full_conceal_vid") && frameAttributes.value("full_conceal_vid").toInt() == 1;
    auto full_conceal_aud = frameAttributes.hasAttribute("full_conceal_aud") && frameAttributes.value("full_conceal_aud").toInt() == 1;

    map["Video Error/Full Concealment"] = false;
    map["Audio Error/Full Concealment"] = false;

    if(full_conceal) {
        fullConcealment << "Video" << "Audio";

        map["Video Error/Full Concealment"] = true;
        map["Audio Error/Full Concealment"] = true;
    } else {
        if(full_conceal_vid) {
            fullConcealment << "Video";
            map["Video Error/Full Concealment"] = true;
        }

        if(full_conceal_aud) {
            fullConcealment << "Audio";
            map["Audio Error/Full Concealment"] = true;
        }
    }

    map["Full Concealment"] = fullConcealment.join(", ");

    QString videoRate;
    QString videoSize;
    QString aspectRatio;
    QString chromaSubsampling;

    if(framesAttributes.hasAttribute("video_rate")) {
        auto videoRateValue = framesAttributes.value("video_rate").toString();
        if(videoRateValue == "30000/1001") {
            videoRate = "NTSC";
        } else if(videoRateValue == "25") {
            videoRate = "PAL";
        } else {
            videoRate = videoRateValue;
        }
    }

    videoSize = getStringAttribute("size", framesAttributes);
    aspectRatio = getStringAttribute("aspect_ratio", framesAttributes);
    chromaSubsampling = getStringAttribute("chroma_subsampling", framesAttributes);

    auto video = (QStringList() << videoRate << videoSize << aspectRatio << chromaSubsampling).join(" ");

    QString channels;
    QString audioRate;
    if(framesAttributes.hasAttribute("audio_rate"))
    {
        auto rate = framesAttributes.value("audio_rate").toInt();
        if((rate % 1000) == 0)
            audioRate = QString::number(rate / 1000) + "k";
        else
            audioRate = QString::number(float(rate) / 1000, 'f', 1) + "k";
    }

    if(framesAttributes.hasAttribute("channels"))
        channels = framesAttributes.value("channels").toString() + "ch";

    auto audio = channels + " " + audioRate;

    frameStats.videoInfo = video;
    frameStats.audioInfo = audio;

    map["Video/Audio"] = video + " " + audio;

    auto video_block_count = diff_seq_count * video_blocks_per_diff_seq;
    auto audio_block_count = diff_seq_count * audio_blocks_per_diff_seq;

    auto video_error_concealment_percent = double(totalSta) / video_block_count * 100;
    map["Video Error %"] = QString::number(video_error_concealment_percent, 'f', 2) + QString("%");

    auto video_error_concealment = QPointF(totalEvenSta, totalSta - totalEvenSta) / video_block_count * 2;
    map["Video Error"] = video_error_concealment;

    auto audio_error_concealment_percent = double(totalAud) / audio_block_count * 100;
    map["Audio Error %"] = QString::number(audio_error_concealment_percent, 'f', 2) + QString("%");

    auto audio_error_concealment = QPointF(totalEvenAud, totalAud - totalEvenAud) / audio_block_count * 2;
    map["Audio Error"] = audio_error_concealment;

    fillAttribute("Absolute Track Number", frameAttributes, "abst");
    int abstRepeat = 0;
    if(frameAttributes.hasAttribute("abst_r"))
        abstRepeat = frameAttributes.value("abst_r").toInt();

    auto abstJump = 0;
    if(frameAttributes.hasAttribute("abst_nc"))
        abstJump = frameAttributes.value("abst_nc").toInt();

    map["Absolute Track Number: Jump/Repeat"] = QPoint(abstJump, abstRepeat);

    Q_EMIT dataRowCreated(map);
}

void DataModel::onDataRowCreated(const QVariantMap &map)
{
    // qDebug() << "DataModel::dataRowCreated: " << QThread::currentThread();

    assert(engine);
    auto variant = QVariant::fromValue(engine->toScriptValue(map));

    Q_EMIT gotDataRow(variant);
}
