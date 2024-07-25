#ifndef QMLPLOT_H
#define QMLPLOT_H

#include <QtQuick>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_marker.h>
#include <qwt_legend.h>

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
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged);
    Q_PROPERTY(QColor yLeftAxisColor READ yLeftAxisColor WRITE setYLeftAxisColor NOTIFY yLeftAxisColorChanged);
    Q_PROPERTY(QString yLeftAxisTitle READ yLeftAxisTitle WRITE setYLeftAxisTitle NOTIFY leftYAxisTitleChanged);
    Q_PROPERTY(QFont yLeftAxisFont READ yLeftAxisFont WRITE setYLeftAxisFont NOTIFY yLeftAxisFontChanged);
    Q_PROPERTY(bool yLeftAxisVisible READ yLeftAxisVisible WRITE setYLeftAxisVisible NOTIFY yLeftAxisVisibleChanged);
    Q_PROPERTY(QColor xBottomAxisColor READ xBottomAxisColor WRITE setXBottomAxisColor NOTIFY xBottomAxisColorChanged);
    Q_PROPERTY(QString xBottomAxisTitle READ xBottomAxisTitle WRITE setXBottomAxisTitle NOTIFY xBottomAxisTitleChanged)
    Q_PROPERTY(QFont xBottomAxisFont READ xBottomAxisFont WRITE setXBottomAxisFont NOTIFY xBottomAxisFontChanged);
    Q_PROPERTY(bool xBottomAxisVisible READ xBottomAxisVisible WRITE setXBottomAxisVisible NOTIFY xBottomAxisVisibleChanged);
    Q_PROPERTY(QVector2D yLeftAxisRange READ yLeftAxisRange WRITE setYLeftAxisRange NOTIFY yLeftAxisRangeChanged);
    Q_PROPERTY(QVector2D xBottomAxisRange READ xBottomAxisRange WRITE setXBottomAxisRange NOTIFY xBottomAxisRangeChanged);
    Q_PROPERTY(bool xBottomAxisEnabled READ xBottomAxisEnabled WRITE setXBottomAxisEnabled NOTIFY xBottomAxisEnabledChanged);
public:
    QwtQuick2Plot(QQuickItem* parent = nullptr);
    virtual ~QwtQuick2Plot();

    void paint(QPainter* painter);
    Q_INVOKABLE void replotAndUpdate();
    QwtPlot* plot() const;
    QQuickItem* canvasItem() const;
    void updateCanvaSize();

    QString yLeftAxisTitle() const;
    QString xBottomAxisTitle() const;
    QVector2D yLeftAxisRange() const;
    QVector2D xBottomAxisRange() const;
    QFont yLeftAxisFont() const;
    QFont xBottomAxisFont() const;
    bool xBottomAxisEnabled() const;    
    QColor yLeftAxisColor() const;
    QColor xBottomAxisColor() const;
    const QColor &backgroundColor() const;

    bool yLeftAxisVisible() const;
    void setYLeftAxisVisible(bool newYLeftAxisVisible);
    bool xBottomAxisVisible() const;
    void setXBottomAxisVisible(bool newXBottomAxisVisible);

public Q_SLOTS:
    void setYLeftAxisTitle(QString yLeftAxisTitle);
    void setXBottomAxisTitle(QString xBottomAxisTitle);
    void setYLeftAxisRange(QVector2D yLeftAxisRange);
    void setXBottomAxisRange(QVector2D xBottomAxisRange);
    void setYLeftAxisFont(QFont yLeftAxisFont);
    void setXBottomAxisFont(QFont xBottomAxisFont);
    void setXBottomAxisEnabled(bool xBottomAxisEnabled);
    void setYLeftAxisColor(QColor yLeftAxisColor);
    void setXBottomAxisColor(QColor xBottomAxisColor);
    void setBackgroundColor(const QColor &newBackgroundColor);

Q_SIGNALS:
    void canvasItemChanged();
    void leftYAxisTitleChanged(QString yLeftAxisTitle);
    void xBottomAxisTitleChanged(QString xBottomAxisTitle);
    void yLeftAxisRangeChanged(QVector2D yLeftAxisRange);
    void xBottomAxisRangeChanged(QVector2D xBottomAxisRange);
    void yLeftAxisFontChanged(QFont yLeftAxisFont);
    void xBottomAxisFontChanged(QFont xBottomAxisFont);
    void xBottomAxisEnabledChanged(bool xBottomAxisEnabled);
    void yLeftAxisColorChanged(QColor yLeftAxisColor);
    void xBottomAxisColorChanged(QColor xBottomAxisColor);
    void backgroundColorChanged();
    void yLeftAxisVisibleChanged();
    void xBottomAxisVisibleChanged();

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
    Q_PROPERTY(QColor titleColor READ titleColor WRITE setTitleColor NOTIFY titleColorChanged)
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

    QwtPlotCurve* curve() const;
    QwtQuick2Plot* plot() const;

    void attach(QwtQuick2Plot* plot);
    CurveStyle curveStyle() const;

    QString title() const;
    qreal width() const;
    QColor color() const;    
    QColor titleColor() const;

public Q_SLOTS:
    void setCurveStyle(CurveStyle curveStyle);
    void setTitle(QString title);
    void setWidth(qreal width);
    void setColor(QColor color);
    void setTitleColor(QColor titleColor);

Q_SIGNALS:
    void curveStyleChanged(CurveStyle curveStyle);
    void titleChanged(QString title);
    void widthChanged(qreal width);
    void colorChanged(QColor color);
    void titleColorChanged(QColor titleColor);

private:
    QwtQuick2Plot* m_qwtQuickPlot { nullptr };
    QwtPlotCurve* m_qwtPlotCurve;
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
    Q_PROPERTY(QwtQuick2Plot* plotItem READ plotItem NOTIFY plotItemChanged);
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QPointF point READ point WRITE setPoint NOTIFY pointChanged)
public:
    QwtQuick2PlotPicker(QQuickItem* parent = nullptr);

    void attach(QwtQuick2Plot* plot);
    bool active() const;    
    QPointF point() const;

    Q_INVOKABLE QPoint transformPoint(const QPointF& p);
    Q_INVOKABLE QPointF invTransformPoint(const QPoint& p);

    QwtQuick2Plot* plotItem() const;

public Q_SLOTS:
    void setActive(bool active);
    void setPoint(QPointF point);

Q_SIGNALS:
    void activeChanged(bool active);    
    void pointChanged(QPointF point);    
    void plotItemChanged(QwtQuick2Plot* plotItem);

private:
    QwtPlotPicker* m_qwtPlotPicker { nullptr };
    bool m_active { false };
    QPointF m_point;
    QwtQuick2Plot* m_qwtQuickPlot { nullptr };
};

class QwtQuick2PlotLegend : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QwtQuick2Plot* plotItem READ plotItem WRITE setPlotItem NOTIFY plotItemChanged)
public:
    QwtQuick2PlotLegend(QQuickItem* parent = nullptr);
    QwtQuick2Plot* plotItem() const;

public Q_SLOTS:
    void setPlotItem(QwtQuick2Plot* plot);

Q_SIGNALS:
    void plotItemChanged(QwtQuick2Plot* plot);

protected:
    void paint(QPainter* painter);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    virtual void geometryChanged(const QRectF &newGeometry,
                                 const QRectF &oldGeometry);
#else
    virtual void geometryChange(const QRectF &newGeometry,
                                 const QRectF &oldGeometry);
#endif //


private:
    QwtLegend* m_legend;
    QwtQuick2Plot* m_qwtQuickPlot { nullptr };
};

class QwtQuick2PlotMarker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged)
    Q_PROPERTY(qreal penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged)
    Q_PROPERTY(Qt::PenStyle penStyle READ penStyle WRITE setPenStyle NOTIFY penStyleChanged)
    Q_PROPERTY(LineStyle lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)

    Q_PROPERTY(QPointF value READ value WRITE setValue NOTIFY valueChanged)
public:
    enum LineStyle {
        //! No line
        NoLine,

        //! A horizontal line
        HLine,

        //! A vertical line
        VLine,

        //! A crosshair
        Cross
    };
    Q_ENUM(LineStyle)

    QwtQuick2PlotMarker(QObject* parent = nullptr);

    void attach(QwtQuick2Plot* plot);

    QColor penColor() const;
    qreal penWidth() const;
    Qt::PenStyle penStyle() const;

    void setPenColor(QColor penColor);
    void setPenWidth(qreal penWidth);
    void setPenStyle(Qt::PenStyle penStyle);

    QPointF value() const;
    void setValue(QPointF newValue);

    LineStyle lineStyle() const;
    void setLineStyle(const LineStyle &newLineStyle);

Q_SIGNALS:
    void penColorChanged(QColor penColor);
    void penWidthChanged(qreal penWidth);
    void penStyleChanged(Qt::PenStyle penStyle);

    void valueChanged();
    void lineStyleChanged();

private:
    QwtPlotMarker* m_qwtPlotMarker;
};

#endif // QMLPLOT_H
