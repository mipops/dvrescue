#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include <QObject>
#include <QThread>
#include <QPair>
#include <xmlparser.h>

class QwtQuick2PlotCurve;
class GraphModel : public QObject
{
    Q_OBJECT
public:
    explicit GraphModel(QObject *parent = nullptr);
    ~GraphModel();

public Q_SLOTS:
    void update(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve* videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve* audioCurve2);
    void populate(const QString& fileName);

Q_SIGNALS:
    void populated();

private:
    XmlParser* m_parser { nullptr };
    std::unique_ptr<QThread> m_thread;

    QList<QPair<float, float>> m_videoValues;
    QList<QPair<float, float>> m_audioValues;
};

#endif // GRAPHMODEL_H
