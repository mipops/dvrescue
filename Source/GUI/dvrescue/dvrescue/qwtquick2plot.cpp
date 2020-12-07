#include "qwtquick2plot.h"
#include "plotdata.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_renderer.h>
#include <qwt_text.h>

#include <QObject>
#include <QDebug>

QwtQuick2Plot::QwtQuick2Plot(QQuickItem* parent) : QQuickPaintedItem(parent)
    , m_qwtPlot(nullptr)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &QwtQuick2Plot::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &QwtQuick2Plot::updatePlotSize);

    m_qwtPlot = new QwtPlot();
    m_qwtPlot->setAutoReplot(false);

    updatePlotSize();

    m_qwtPlot->setAxisTitle(m_qwtPlot->xBottom, tr("t"));
    m_qwtPlot->setAxisTitle(m_qwtPlot->yLeft, tr("S"));

    replotAndUpdate();
}

QwtQuick2Plot::~QwtQuick2Plot()
{
    delete m_qwtPlot;
    m_qwtPlot = nullptr;
}

void QwtQuick2Plot::replotAndUpdate()
{
    m_qwtPlot->replot();
    update();
}

QwtPlot *QwtQuick2Plot::plot() const
{
    return m_qwtPlot;
}

void QwtQuick2Plot::paint(QPainter* painter)
{
    if (m_qwtPlot) {
        QPixmap picture(boundingRect().size().toSize());

        QwtPlotRenderer renderer;
        renderer.renderTo(m_qwtPlot, picture);

        painter->drawPixmap(QPoint(), picture);
    }
}

void QwtQuick2Plot::mousePressEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void QwtQuick2Plot::mouseReleaseEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void QwtQuick2Plot::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvents(event);
}

void QwtQuick2Plot::mouseDoubleClickEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void QwtQuick2Plot::wheelEvent(QWheelEvent* event)
{
    routeWheelEvents(event);
}
void QwtQuick2Plot::childEvent(QChildEvent *event)
{
    if (event->type() == QEvent::ChildAdded) {
        auto child = event->child();
        auto plotCurve = qobject_cast<QwtQuick2PlotCurve *>(child);
        if (plotCurve) {
            plotCurve->attach(this);
        }
    }
}

void QwtQuick2Plot::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    for(QObject *child : children()) {
        auto plotCurve = qobject_cast<QwtQuick2PlotCurve *>(child);
        if (plotCurve) {
            plotCurve->attach(this);
        }
    }
}

void QwtQuick2Plot::routeMouseEvents(QMouseEvent* event)
{
    if (m_qwtPlot) {
        QMouseEvent* newEvent = new QMouseEvent(event->type(), event->localPos(),
                                                event->button(), event->buttons(),
                                                event->modifiers());
        QCoreApplication::postEvent(m_qwtPlot, newEvent);
    }
}

void QwtQuick2Plot::routeWheelEvents(QWheelEvent* event)
{
    if (m_qwtPlot) {
        QWheelEvent* newEvent = new QWheelEvent(event->pos(), event->delta(),
                                                event->buttons(), event->modifiers(),
                                                event->orientation());
        QCoreApplication::postEvent(m_qwtPlot, newEvent);
    }
}

void QwtQuick2Plot::updatePlotSize()
{
    if (m_qwtPlot) {
        m_qwtPlot->setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));
    }
}

QwtQuick2PlotCurve::QwtQuick2PlotCurve(QObject *quickPlotObject)
{
    m_qwtPlotCurve = new QwtPlotCurve("Curve 1");
    m_qwtPlotCurve->setTitle("Curve 1");
    m_qwtPlotCurve->setPen(QPen(Qt::red));
    m_qwtPlotCurve->setStyle(QwtPlotCurve::Lines);
    m_qwtPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);

    m_qwtPlotCurve->setData(new PlotData(&m_curveData));
}

QwtQuick2PlotCurve::~QwtQuick2PlotCurve()
{

}

QwtPlotCurve *QwtQuick2PlotCurve::curve() const
{
    return m_qwtPlotCurve;
}

QwtQuick2Plot *QwtQuick2PlotCurve::plot() const
{
    return m_qwtQuickPlot;
}

void QwtQuick2PlotCurve::attach(QwtQuick2Plot *plot)
{
    m_qwtQuickPlot = plot;
    curve()->attach(plot->plot());
}

QwtQuick2PlotCurve::CurveStyle QwtQuick2PlotCurve::curveStyle() const
{
    return (QwtQuick2PlotCurve::CurveStyle) m_qwtPlotCurve->style();
}

QString QwtQuick2PlotCurve::title() const
{
    return m_qwtPlotCurve->title().text();
}

qreal QwtQuick2PlotCurve::width() const
{
    return m_qwtPlotCurve->pen().widthF();
}

QColor QwtQuick2PlotCurve::color() const
{
    return m_qwtPlotCurve->pen().color();
}

void QwtQuick2PlotCurve::setCurveStyle(QwtQuick2PlotCurve::CurveStyle curveStyle)
{
    m_qwtPlotCurve->setStyle((QwtPlotCurve::CurveStyle) curveStyle);
    Q_EMIT curveStyleChanged(curveStyle);
}

void QwtQuick2PlotCurve::setTitle(QString title)
{
    if (m_qwtPlotCurve->title().text() == title)
        return;

    m_qwtPlotCurve->setTitle(title);
    Q_EMIT titleChanged(title);
}

void QwtQuick2PlotCurve::setWidth(qreal width)
{
    if (qFuzzyCompare(m_qwtPlotCurve->pen().widthF(), width))
        return;

    auto pen = m_qwtPlotCurve->pen();
    pen.setWidthF(width);
    m_qwtPlotCurve->setPen(pen);
    Q_EMIT widthChanged(width);
}

void QwtQuick2PlotCurve::setColor(QColor color)
{
    if (m_qwtPlotCurve->pen().color() == color)
        return;

    auto pen = m_qwtPlotCurve->pen();
    pen.setColor(color);
    m_qwtPlotCurve->setPen(pen);
    Q_EMIT colorChanged(color);
}

