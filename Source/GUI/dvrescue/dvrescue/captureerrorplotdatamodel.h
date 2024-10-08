#ifndef CAPTUREERRORPLOTDATAMODEL_H
#define CAPTUREERRORPLOTDATAMODEL_H


#include <QObject>
#include <QThread>
#include <QPair>
#include <QSet>
#include <QMap>
#include <QVariant>
#include <qwtquick2plot.h>
#include <xmlparser.h>

class QAbstractTableModel;
class QJSEngine;
class QXmlStreamAttributes;
class QXmlStreamAttributes;

class CaptureErrorPlotDataModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int total READ total NOTIFY totalChanged)
    Q_PROPERTY(int totalWithErrors READ totalWithErrors NOTIFY totalWithErrorsChanged)
    Q_PROPERTY(int last READ last NOTIFY lastChanged)
    Q_PROPERTY(int lastWithErrors READ lastWithErrors NOTIFY lastWithErrorsChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* evenCurve READ evenCurve WRITE setEvenCurve NOTIFY evenCurveChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* oddCurve READ oddCurve WRITE setOddCurve NOTIFY oddCurveChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* notInPlayOrRecordCurveEven READ notInPlayOrRecordCurveEven WRITE setNotInPlayOrRecordCurveEven NOTIFY notInPlayOrRecordCurveEvenChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* notInPlayOrRecordCurveOdd READ notInPlayOrRecordCurveOdd WRITE setNotInPlayOrRecordCurveOdd NOTIFY notInPlayOrRecordCurveOddChanged)

public:
    explicit CaptureErrorPlotDataModel(QObject *parent = nullptr);
    ~CaptureErrorPlotDataModel();

    int total() const;
    int totalWithErrors() const;
    int last() const;
    int lastWithErrors() const;

    struct GraphStats {
        int frameNumber;
        float evenValue;
        float oddValue;
        bool playing;
    };

    QwtQuick2PlotCurve *evenCurve() const;
    void setEvenCurve(QwtQuick2PlotCurve *newEvenCurve);

    QwtQuick2PlotCurve *oddCurve() const;
    void setOddCurve(QwtQuick2PlotCurve *newOddCurve);

    QwtQuick2PlotCurve *notInPlayOrRecordCurveEven() const;
    void setNotInPlayOrRecordCurveEven(QwtQuick2PlotCurve *newNotInPlayOrRecordCurveEven);

    QwtQuick2PlotCurve *notInPlayOrRecordCurveOdd() const;
    void setNotInPlayOrRecordCurveOdd(QwtQuick2PlotCurve *newNotInPlayOrRecordCurveOdd);

public Q_SLOTS:
    void update();
    void reset();

    void append(int frameNumber, float even, float odd, bool playing);

Q_SIGNALS:
    void totalChanged();
    void totalWithErrorsChanged();
    void lastChanged();
    void lastWithErrorsChanged();

    void updated();
    void clearModel();
    void evenCurveChanged();
    void oddCurveChanged();

    void notInPlayOrRecordCurveEvenChanged();
    void notInPlayOrRecordCurveOddChanged();

private:

    QList<std::tuple<int, GraphStats>> m_values;
    int m_total { 0 };
    int m_totalWithErrors { 0 };
    int m_last { 0 };
    int m_lastWithErrors { 0 };

    QwtQuick2PlotCurve *m_evenCurve = nullptr;
    QwtQuick2PlotCurve *m_oddCurve = nullptr;
    QwtQuick2PlotCurve *m_notInPlayOrRecordCurveEven = nullptr;
    QwtQuick2PlotCurve *m_notInPlayOrRecordCurveOdd = nullptr;
};
#endif // CAPTUREERRORPLOTDATAMODEL_H
