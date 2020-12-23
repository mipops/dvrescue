#ifndef QMLPLOT_H
#define QMLPLOT_H

#include <QtQuick>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>

namespace QwtQuick2
{
    Q_NAMESPACE         // required for meta object creation
    enum PenStyle {
        NoPen,
        SolidLine,
        DashLine,
        DotLine,
        DashDotLine,
        DashDotDotLine,
        CustomDashLine
    };
    Q_ENUM_NS(PenStyle)  // register the enum in meta object data
}

class QwtQuick2Plot : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem* canvasItem READ canvasItem NOTIFY canvasItemChanged);
public:
    QwtQuick2Plot(QQuickItem* parent = nullptr);
    virtual ~QwtQuick2Plot();

    void paint(QPainter* painter);
    Q_INVOKABLE void replotAndUpdate();
    QwtPlot* plot() const;
    QQuickItem* canvasItem() const;
    void updateCanvaSize();

Q_SIGNALS:
    void canvasItemChanged();

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
    void attach(QObject* child);
    QwtPlot* m_qwtPlot;
    QQuickItem* m_canvasItem;

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

    QwtQuick2PlotCurve(QObject* parent = nullptr);
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
    QwtQuick2Plot* m_qwtQuickPlot { nullptr };
    QwtPlotCurve* m_qwtPlotCurve;
    QVector<QPointF> m_curveData;
};

class QwtQuick2PlotGrid : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enableXMin READ enableXMin WRITE setEnableXMin NOTIFY enableXMinChanged)
    Q_PROPERTY(bool enableYMin READ enableYMin WRITE setEnableYMin NOTIFY enableYMinChanged)

    Q_PROPERTY(QColor majorPenColor READ majorPenColor WRITE setMajorPenColor NOTIFY majorPenColorChanged)
    Q_PROPERTY(qreal majorPenWidth READ majorPenWidth WRITE setMajorPenWidth NOTIFY majorPenWidthChanged)
    Q_PROPERTY(Qt::PenStyle majorPenStyle READ majorPenStyle WRITE setMajorPenStyle NOTIFY majorPenStyleChanged)

    Q_PROPERTY(QColor minorPenColor READ minorPenColor WRITE setMinorPenColor NOTIFY minorPenColorChanged)
    Q_PROPERTY(qreal minorPenWidth READ minorPenWidth WRITE setMinorPenWidth NOTIFY minorPenWidthChanged)
    Q_PROPERTY(Qt::PenStyle minorPenStyle READ minorPenStyle WRITE setMinorPenStyle NOTIFY minorPenStyleChanged)
public:
    QwtQuick2PlotGrid(QObject* parent = nullptr);

    void attach(QwtQuick2Plot* plot);

    bool enableXMin() const;
    bool enableYMin() const;

    QColor majorPenColor() const;
    qreal majorPenWidth() const;
    Qt::PenStyle majorPenStyle() const;

    QColor minorPenColor() const;
    qreal minorPenWidth() const;
    Qt::PenStyle minorPenStyle() const;

public Q_SLOTS:
    void setEnableXMin(bool enableXMin);
    void setEnableYMin(bool enableYMin);

    void setMajorPenColor(QColor majorPenColor);
    void setMajorPenWidth(qreal majorPenWidth);
    void setMajorPenStyle(Qt::PenStyle majorPenStyle);

    void setMinorPenColor(QColor minorPenColor);
    void setMinorPenWidth(qreal minorPenWidth);
    void setMinorPenStyle(Qt::PenStyle minorPenStyle);

Q_SIGNALS:
    void enableXMinChanged(bool enableXMin);
    void enableYMinChanged(bool enableYMin);

    void majorPenColorChanged(QColor majorPenColor);
    void majorPenWidthChanged(qreal majorPenWidth);
    void majorPenStyleChanged(Qt::PenStyle majorPenStyle);

    void minorPenColorChanged(QColor minorPenColor);
    void minorPenWidthChanged(qreal minorPenWidth);
    void minorPenStyleChanged(Qt::PenStyle minorPenStyle);

private:
    QwtPlotGrid* m_qwtPlotGrid;
};

class QwtQuick2PlotPicker : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QPointF point READ point WRITE setPoint NOTIFY pointChanged)
public:
    QwtQuick2PlotPicker(QQuickItem* parent = nullptr);

    void attach(QwtQuick2Plot* plot);
    bool active() const;    
    QPointF point() const;

public Q_SLOTS:
    void setActive(bool active);
    void setPoint(QPointF point);

Q_SIGNALS:
    void activeChanged(bool active);    
    void pointChanged(QPointF point);

private:
    QwtPlotPicker* m_qwtPlotPicker;
    bool m_active;
    QPointF m_point;
};

#endif // QMLPLOT_H
