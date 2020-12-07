#ifndef QMLPLOT_H
#define QMLPLOT_H

#include <QtQuick>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class QwtQuick2Plot : public QQuickPaintedItem
{
    Q_OBJECT

public:
    QwtQuick2Plot(QQuickItem* parent = nullptr);
    virtual ~QwtQuick2Plot();

    void paint(QPainter* painter);
    Q_INVOKABLE void replotAndUpdate();
    QwtPlot* plot() const;

protected:
    void routeMouseEvents(QMouseEvent* event);
    void routeWheelEvents(QWheelEvent* event);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent *event);

    virtual void childEvent(QChildEvent *event);
    virtual void componentComplete();
private:
    QwtPlot*         m_qwtPlot;

private Q_SLOTS:
    void updatePlotSize();
};

class QwtQuick2PlotCurve : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CurveStyle curveStyle READ curveStyle WRITE setCurveStyle NOTIFY curveStyleChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    enum CurveStyle
    {
        NoCurve = -1,
        Lines,
        Sticks,
        Steps,
        Dots,
        UserCurve = 100
    };
    Q_ENUM(CurveStyle)

    QwtQuick2PlotCurve(QObject* quickPlotObject = nullptr);
    virtual ~QwtQuick2PlotCurve();

    QVector<QPointF>& data() { return m_curveData; }
    QwtPlotCurve* curve() const;
    QwtQuick2Plot* plot() const;

    void attach(QwtQuick2Plot* plot);
    CurveStyle curveStyle() const;

    QString title() const;
    qreal width() const;
    QColor color() const;

public Q_SLOTS:
    void setCurveStyle(CurveStyle curveStyle);
    void setTitle(QString title);
    void setWidth(qreal width);
    void setColor(QColor color);

Q_SIGNALS:
    void curveStyleChanged(CurveStyle curveStyle);
    void titleChanged(QString title);
    void widthChanged(qreal width);
    void colorChanged(QColor color);

private:
    QwtQuick2Plot* m_qwtQuickPlot;
    QwtPlotCurve* m_qwtPlotCurve;
    QVector<QPointF> m_curveData;
};


#endif // QMLPLOT_H
