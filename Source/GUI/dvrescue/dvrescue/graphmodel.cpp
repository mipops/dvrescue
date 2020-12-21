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
    videoCurve->plot()->plot()->setAxisScale(QwtPlot::yLeft, -100, 100);

    auto videoCount = videoCurve->data().count();
    for(auto i = videoCount; i < m_videoValues.count(); ++i) {
        auto value = m_videoValues.at(i);
        videoCurve->data().append(QPointF(i, value.first));
        videoCurve2->data().append(QPointF(i, value.second));
    }

    videoCurve->plot()->replotAndUpdate();

    audioCurve->plot()->plot()->setAxisScale(QwtPlot::yLeft, -100, 100);

    auto audioCount = audioCurve->data().count();
    for(auto i = audioCount; i < m_audioValues.count(); ++i) {
        auto value = m_audioValues.at(i);
        audioCurve->data().append(QPointF(i, value.first));
        audioCurve2->data().append(QPointF(i, value.second));
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

    connect(m_parser, &XmlParser::gotFrame, [&](auto frameNumber) {
        // qDebug() << QThread::currentThread() << frameNumber;
        while(m_videoValues.size() != (frameNumber + 1))
            m_videoValues.append(QPair<int, int>(0, 0));

        while(m_audioValues.size() != (frameNumber + 1))
            m_audioValues.append(QPair<int, int>(0, 0));
    });

    connect(m_parser, &XmlParser::gotSta, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        // qDebug() << "got sta: " << frameNumber << t << n << n_even;
        if(t == 10) {
            QPair<float, float> value(float(n_even) / den * 100, -float(n - n_even) / den * 100);
            m_videoValues[frameNumber] = value;
        }
    });
    connect(m_parser, &XmlParser::gotAud, [&](auto frameNumber, auto t, auto n, auto n_even, auto den) {
        Q_UNUSED(t);
        // qDebug() << "got aud: " << frameNumber << t << n << n_even;
        // if(t == 10) {
            QPair<float, float> value(float(n_even) / den * 100, -float(n - n_even) / den * 100);
            m_audioValues[frameNumber] = value;
        // }
    });

    m_thread->start();
}
