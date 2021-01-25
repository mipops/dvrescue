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
}

void DataModel::reset(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve *videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve *audioCurve2)
{
    videoCurve->data().clear();
    videoCurve2->data().clear();
    audioCurve->data().clear();
    audioCurve2->data().clear();

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

        m_videoValues.clear();
        m_audioValues.clear();
    }

    qDebug() << "DataModel::populate: " << QThread::currentThread();

    m_lastFrame = 0;
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

    connect(m_parser, &XmlParser::gotFrameAttributes, [this](auto frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes) {
        m_lastFrame = frameNumber;
        m_total = m_lastFrame + 1;

        onGotFrame(frameNumber, framesAttributes, frameAttributes);
        Q_EMIT totalChanged(m_total);
    });

    connect(m_parser, &XmlParser::gotSta, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        // qDebug() << "got sta: " << frameNumber << t << n << n_even;
        if(t == 10) {
            GraphStats value = {
                int(frameNumber),
                float(n_even) / den * 100,
                -float(n - n_even) / den * 100,
                float(den)
            };

            qDebug() << "video frame: " << frameNumber << value.evenValue << value.oddValue;
            m_videoValues.append(std::make_tuple(frameNumber, value));
        }
    });
    connect(m_parser, &XmlParser::gotAud, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        Q_UNUSED(t);
        // qDebug() << "got aud: " << frameNumber << t << n << n_even;

        // if(t == 10) {
            GraphStats value = {
                int(frameNumber),
                float(n_even) / den * 100,
                -float(n - n_even) / den * 100,
                float(den)
            };

            // qDebug() << "audio frame: " << frameNumber;
            m_audioValues.append(std::make_tuple(frameNumber, value));
        // }
    });

    m_thread->start();
}

void DataModel::onGotFrame(int frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes)
{
    // qDebug() << "DataModel::onGotFrame: " << QThread::currentThread();

    QVariantMap map;
    map["Frame #"] = frameNumber;

    auto fillAttribute = [&](const QString& mapKeyName, const QXmlStreamAttributes& attributes, const QString& name, const QString& defaultValue = "") {
        map[mapKeyName] = attributes.hasAttribute(name) ? attributes.value(name).toString() : defaultValue;
    };

    fillAttribute("Byte Offset", frameAttributes, "pos");
    fillAttribute("Timestamp", frameAttributes, "pts");
    fillAttribute("Timecode", frameAttributes, "tc");
    fillAttribute("Timecode Repeat", frameAttributes, "tc_r");
    fillAttribute("Timecode Jump", frameAttributes, "tc_nc");
    fillAttribute("Recording Time", frameAttributes, "rdt");
    fillAttribute("Recording Time Repeat", frameAttributes, "rdt_r");
    fillAttribute("Recording Time Jump", frameAttributes, "rdt_nc");

    fillAttribute("Recording Start", frameAttributes, "rec_start");
    fillAttribute("Recording End", frameAttributes, "rec_end");
    fillAttribute("Arbitrary Bits", frameAttributes, "arb");
    fillAttribute("Arbitrary Bits Repeat", frameAttributes, "arb_r");
    fillAttribute("Arbitrary Bits Jump", frameAttributes, "arb_nc");

    fillAttribute("Captions", framesAttributes, "captions");
    fillAttribute("Caption Parity", frameAttributes, "caption-parity");
    fillAttribute("No Pack", frameAttributes, "no_pack");
    fillAttribute("No Subcode Pack", frameAttributes, "no_pack_sub");
    fillAttribute("No Video Pack", frameAttributes, "no_pack_vid");
    fillAttribute("No Audio Pack", frameAttributes, "no_pack_aud");

    fillAttribute("No Video Source or Control", frameAttributes, "no_sourceorcontrol_vid");
    fillAttribute("No Audio Source or Control", frameAttributes, "no_sourceorcontrol_audio");
    fillAttribute("Full Conceal", frameAttributes, "full_conceal");
    fillAttribute("Full Conceal Video", frameAttributes, "full_conceal_vid");
    fillAttribute("Full Conceal Audio", frameAttributes, "full_conceal_aud");

    fillAttribute("Video Size", framesAttributes, "size");
    fillAttribute("Video Rate", framesAttributes, "video_rate");
    fillAttribute("Chroma Subsampling", framesAttributes, "chroma_subsampling");
    fillAttribute("Aspect Ratio", framesAttributes, "aspect_ratio");
    fillAttribute("Audio Rate", framesAttributes, "audio_rate");
    fillAttribute("Channels", framesAttributes, "channels");

    Q_EMIT dataRowCreated(map);
}

void DataModel::onDataRowCreated(const QVariantMap &map)
{
    // qDebug() << "DataModel::dataRowCreated: " << QThread::currentThread();

    assert(engine);
    auto variant = QVariant::fromValue(engine->toScriptValue(map));

    Q_EMIT gotDataRow(variant);
}