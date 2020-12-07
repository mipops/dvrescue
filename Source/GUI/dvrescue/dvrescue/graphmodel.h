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
    void update(QwtQuick2PlotCurve *curve, QwtQuick2PlotCurve* curve2);
    void populate(const QString& fileName);

private:
    XmlParser* m_parser { nullptr };
    std::unique_ptr<QThread> m_thread;

    QList<QPair<int, int>> m_values;
};

#endif // GRAPHMODEL_H
