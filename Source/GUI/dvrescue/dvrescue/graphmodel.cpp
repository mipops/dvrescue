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

void GraphModel::update(QwtQuick2PlotCurve *curve, QwtQuick2PlotCurve *curve2)
{
    curve->plot()->plot()->setAxisScale(QwtPlot::yLeft, -800, 800);

    auto videoCount = curve->data().count();
    for(auto i = videoCount; i < m_values.count(); ++i) {

        // qDebug() << "appending: " << m_values.at(i);
        auto value = m_values.at(i);
        curve->data().append(QPointF(i, value.first));
        curve2->data().append(QPointF(i, value.second));
    }

    curve->plot()->replotAndUpdate();
}

void GraphModel::populate(const QString &fileName)
{
    if(m_thread)
    {
        qDebug() << "request interruption";
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();

        m_values.clear();
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

    connect(m_parser, &XmlParser::gotFrame, [&](auto frameNumber) {
        // qDebug() << QThread::currentThread() << frameNumber;
        while(m_values.size() != (frameNumber + 1))
            m_values.append(QPair<int, int>(0, 0));
    });

    connect(m_parser, &XmlParser::gotSta, [&](auto frameNumber, auto t, auto n, auto n_even) {
        // qDebug() << "got sta: " << frameNumber << t << n << n_even;
        if(t == 10) {
            QPair<int, int> value(n_even, -(n - n_even));
            m_values[frameNumber] = value;
        }
    });
    connect(m_parser, &XmlParser::gotAud, [&](auto frameNumber, auto t, auto n, auto n_even) {
        // qDebug() << "got aud: " << frameNumber << t << n << n_even;
    });

    m_thread->start();
}
