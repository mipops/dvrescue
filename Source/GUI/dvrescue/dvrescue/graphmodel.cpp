#include "graphmodel.h"
#include "qwtquick2plot.h"
#include <QThread>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>

GraphModel::GraphModel(QObject *parent) : QObject(parent)
{

}

GraphModel::~GraphModel()
{
    if(m_thread)
    {
        qDebug() << "request interruption";
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();
    }
}

void GraphModel::update(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve *videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve *audioCurve2)
{
    videoCurve->plot()->plot()->setAxisScale(QwtPlot::yLeft, -50, 50);
    videoCurve->plot()->plot()->setAxisScale(QwtPlot::xBottom, 0, m_lastFrame);

    auto videoCount = videoCurve->data().count();
    for(auto i = videoCount; i < m_videoValues.count(); ++i) {
        auto& value = m_videoValues.at(i);
        videoCurve->data().append(QPointF(std::get<0>(value), std::get<1>(value)));
        videoCurve2->data().append(QPointF(std::get<0>(value), std::get<2>(value)));
    }

    videoCurve->plot()->replotAndUpdate();

    audioCurve->plot()->plot()->setAxisScale(QwtPlot::yLeft, -50, 50);
    audioCurve->plot()->plot()->setAxisScale(QwtPlot::xBottom, 0, m_lastFrame);

    auto audioCount = audioCurve->data().count();
    for(auto i = audioCount; i < m_audioValues.count(); ++i) {
        auto& value = m_audioValues.at(i);
        audioCurve->data().append(QPointF(std::get<0>(value), std::get<1>(value)));
        audioCurve2->data().append(QPointF(std::get<0>(value), std::get<2>(value)));
    }

    audioCurve->plot()->replotAndUpdate();
}

void GraphModel::populate(const QString &fileName)
{
    if(m_thread)
    {
        qDebug() << "request interruption";
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();

        m_videoValues.clear();
    }

    qDebug() << QThread::currentThread();

    m_lastFrame = 0;
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
        m_lastFrame = frameNumber;
    });

    connect(m_parser, &XmlParser::gotSta, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        // qDebug() << "got sta: " << frameNumber << t << n << n_even;
        if(t == 10) {
            std::tuple<int, float, float> value(frameNumber, float(n_even) / den * 100, -float(n - n_even) / den * 100);
            m_videoValues.append(value);
        }
    });
    connect(m_parser, &XmlParser::gotAud, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        Q_UNUSED(t);
        // qDebug() << "got aud: " << frameNumber << t << n << n_even;

        // if(t == 10) {
            std::tuple<int, float, float> value(frameNumber, float(n_even) / den * 100, -float(n - n_even) / den * 100);
            m_audioValues.append(value);
        // }
    });

    m_thread->start();
}
