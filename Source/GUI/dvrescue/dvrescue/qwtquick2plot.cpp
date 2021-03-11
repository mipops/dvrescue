#include "qwtquick2plot.h"
#include "plotdata.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_plot_legenditem.h>
#include <qwt_text.h>
#include "qwt_painter.h"

#include <QObject>
#include <QDebug>
#include <qwt_picker_machine.h>

QwtQuick2Plot::QwtQuick2Plot(QQuickItem* parent) : QQuickPaintedItem(parent)
    , m_qwtPlot(nullptr)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_canvasItem = new QQuickItem(this);
    m_canvasItem->setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &QQuickPaintedItem::widthChanged, this, &QwtQuick2Plot::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &QwtQuick2Plot::updatePlotSize);

    m_qwtPlot = new QwtPlot();
    m_qwtPlot->setAutoReplot(false);

    QTimer::singleShot(0, [&]() {
        updatePlotSize();
        replotAndUpdate();
    });
}

QwtQuick2Plot::~QwtQuick2Plot()
{
    delete m_qwtPlot;
    m_qwtPlot = nullptr;
}

void QwtQuick2Plot::replotAndUpdate()
{
    m_qwtPlot->replot();
    updateCanvaSize();
    update();
}

QwtPlot *QwtQuick2Plot::plot() const
{
    return m_qwtPlot;
}

QQuickItem *QwtQuick2Plot::canvasItem() const
{
    return m_canvasItem;
}

void QwtQuick2Plot::paint(QPainter* painter)
{
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);
    renderer.render(m_qwtPlot, painter, QRectF(0, 0, width(), height()));
}

void QwtQuick2Plot::mousePressEvent(QMouseEvent* event)
{
    // qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void QwtQuick2Plot::mouseReleaseEvent(QMouseEvent* event)
{
    // qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void QwtQuick2Plot::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvents(event);
}

void QwtQuick2Plot::mouseDoubleClickEvent(QMouseEvent* event)
{
    // qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void QwtQuick2Plot::wheelEvent(QWheelEvent* event)
{
    routeWheelEvents(event);
}
void QwtQuick2Plot::childEvent(QChildEvent *event)
{
    if (event->type() == QEvent::ChildAdded) {
        attach(event->child());
    }
}

void QwtQuick2Plot::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    for(QObject *child : children()) {
        attach(child);
    }
}

void QwtQuick2Plot::attach(QObject *child)
{
    if (qobject_cast<QwtQuick2PlotCurve *>(child)) {
        qobject_cast<QwtQuick2PlotCurve *>(child)->attach(this);
    } else if(qobject_cast<QwtQuick2PlotGrid *>(child)) {
        qobject_cast<QwtQuick2PlotGrid *>(child)->attach(this);
    } else if(qobject_cast<QwtQuick2PlotPicker *>(child)) {
        qobject_cast<QwtQuick2PlotPicker *>(child)->attach(this);
    }
}

void QwtQuick2Plot::routeMouseEvents(QMouseEvent* event)
{
    auto mappedLocalPos = event->localPos();
    mappedLocalPos.setX(mappedLocalPos.x() - m_qwtPlot->canvas()->x());
    mappedLocalPos.setY(mappedLocalPos.y() - m_qwtPlot->canvas()->y());

    QMouseEvent* newEvent = new QMouseEvent(event->type(), mappedLocalPos,
                                            event->button(), event->buttons(),
                                            event->modifiers());
    QCoreApplication::postEvent(m_qwtPlot->canvas(), newEvent);
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

void QwtQuick2Plot::updateCanvaSize()
{
    auto mappedTopLeft = m_qwtPlot->plotLayout()->canvasRect().topLeft().toPoint();
    auto mappedBottomRight = m_qwtPlot->plotLayout()->canvasRect().bottomRight().toPoint();

    auto leftMargin = 0; m_qwtPlot->axisWidget(QwtPlot::yLeft)->margin();
    auto leftSpacing = 0; m_qwtPlot->axisWidget(QwtPlot::yLeft)->spacing();

    mappedTopLeft.setX(mappedTopLeft.x() + leftMargin + leftSpacing);
    mappedBottomRight.setX(mappedBottomRight.x() - leftMargin - leftSpacing);

    m_canvasItem->setX(mappedTopLeft.x());
    m_canvasItem->setY(mappedTopLeft.y());
    m_canvasItem->setWidth(mappedBottomRight.x() - mappedTopLeft.x());
    m_canvasItem->setHeight(mappedBottomRight.y() - mappedTopLeft.y());
}

QString QwtQuick2Plot::yLeftAxisTitle() const
{
    return m_qwtPlot->axisTitle(m_qwtPlot->yLeft).text();
}

void QwtQuick2Plot::setYLeftAxisTitle(QString yLeftAxisTitle)
{
    if (m_qwtPlot->axisTitle(m_qwtPlot->yLeft).text() == yLeftAxisTitle)
        return;

    m_qwtPlot->setAxisTitle(m_qwtPlot->yLeft, yLeftAxisTitle);
    Q_EMIT leftYAxisTitleChanged(this->yLeftAxisTitle());
}

QString QwtQuick2Plot::xBottomAxisTitle() const
{
    return m_qwtPlot->axisTitle(m_qwtPlot->xBottom).text();
}

void QwtQuick2Plot::setXBottomAxisTitle(QString xBottomAxisTitle)
{
    if (m_qwtPlot->axisTitle(m_qwtPlot->xBottom).text() == xBottomAxisTitle)
        return;

    m_qwtPlot->setAxisTitle(m_qwtPlot->xBottom, xBottomAxisTitle);
    Q_EMIT xBottomAxisTitleChanged(this->xBottomAxisTitle());
}

QVector2D QwtQuick2Plot::xBottomAxisRange() const
{
    auto scale = m_qwtPlot->axisScaleDiv(QwtPlot::xBottom);
    return QVector2D(scale.interval().minValue(), scale.interval().maxValue());
}

QFont QwtQuick2Plot::yLeftAxisFont() const
{
    return m_qwtPlot->axisTitle(QwtPlot::yLeft).font();
}

QFont QwtQuick2Plot::xBottomAxisFont() const
{
    return m_qwtPlot->axisTitle(QwtPlot::xBottom).font();
}

bool QwtQuick2Plot::xBottomAxisEnabled() const
{
    return m_qwtPlot->axisEnabled(QwtPlot::xBottom);
}

QColor QwtQuick2Plot::yLeftAxisColor() const
{
    auto axisWidget = m_qwtPlot->axisWidget(QwtPlot::yLeft);
    return axisWidget->palette().color(QPalette::WindowText);
}

QColor QwtQuick2Plot::xBottomAxisColor() const
{
    auto axisWidget = m_qwtPlot->axisWidget(QwtPlot::xBottom);
    return axisWidget->palette().color(QPalette::WindowText);
}

void QwtQuick2Plot::setXBottomAxisRange(QVector2D xBottomAxisRange)
{
    if (this->xBottomAxisRange() == xBottomAxisRange)
        return;

    QwtLinearScaleEngine se;
    auto scale = se.divideScale( xBottomAxisRange.x(), xBottomAxisRange.y(), 5, 8 );

    m_qwtPlot->setAxisScaleDiv(QwtPlot::xBottom, scale);
    Q_EMIT xBottomAxisRangeChanged(this->xBottomAxisRange());

    replotAndUpdate();
}

void QwtQuick2Plot::setYLeftAxisFont(QFont yLeftAxisFont)
{
    auto title = m_qwtPlot->axisTitle(QwtPlot::yLeft);

    if (title.font() == yLeftAxisFont)
        return;

    title.setFont(yLeftAxisFont);
    m_qwtPlot->setAxisTitle(QwtPlot::yLeft, title);

    updatePlotSize();
    replotAndUpdate();
    Q_EMIT yLeftAxisFontChanged(this->yLeftAxisFont());
}

void QwtQuick2Plot::setXBottomAxisFont(QFont xBottomAxisFont)
{
    auto title = m_qwtPlot->axisTitle(QwtPlot::xBottom);

    if (title.font() == xBottomAxisFont)
        return;

    title.setFont(xBottomAxisFont);
    m_qwtPlot->setAxisTitle(QwtPlot::xBottom, title);

    updatePlotSize();
    replotAndUpdate();
    Q_EMIT xBottomAxisFontChanged(this->xBottomAxisFont());
}

void QwtQuick2Plot::setXBottomAxisEnabled(bool xBottomAxisEnabled)
{
    if (this->xBottomAxisEnabled() == xBottomAxisEnabled)
        return;

    m_qwtPlot->enableAxis(QwtPlot::xBottom, xBottomAxisEnabled);

    updatePlotSize();
    replotAndUpdate();

    Q_EMIT xBottomAxisEnabledChanged(this->xBottomAxisEnabled());
}

void QwtQuick2Plot::setYLeftAxisColor(QColor yLeftAxisColor)
{
    if (this->yLeftAxisColor() == yLeftAxisColor)
        return;

    auto axisWidget = m_qwtPlot->axisWidget(QwtPlot::yLeft);
    QPalette palette = axisWidget->palette();
    palette.setColor( QPalette::WindowText, yLeftAxisColor); // for ticks
    palette.setColor( QPalette::Text, yLeftAxisColor); // for ticks' labels
    axisWidget->setPalette(palette);

    Q_EMIT yLeftAxisColorChanged(yLeftAxisColor);
}

void QwtQuick2Plot::setXBottomAxisColor(QColor xBottomAxisColor)
{
    if (this->xBottomAxisColor() == xBottomAxisColor)
        return;

    auto axisWidget = m_qwtPlot->axisWidget(QwtPlot::xBottom);
    QPalette palette = axisWidget->palette();
    palette.setColor( QPalette::WindowText, xBottomAxisColor); // for ticks
    palette.setColor( QPalette::Text, xBottomAxisColor); // for ticks' labels
    axisWidget->setPalette(palette);

    Q_EMIT xBottomAxisColorChanged(xBottomAxisColor);
}

QVector2D QwtQuick2Plot::yLeftAxisRange() const
{
    auto scale = m_qwtPlot->axisScaleDiv(QwtPlot::yLeft);
    return QVector2D(scale.interval().minValue(), scale.interval().maxValue());
}

void QwtQuick2Plot::setYLeftAxisRange(QVector2D yLeftAxisRange)
{
    if (this->yLeftAxisRange() == yLeftAxisRange)
        return;

    QwtLinearScaleEngine se;
    auto scale = se.divideScale( yLeftAxisRange.x(), yLeftAxisRange.y(), 5, 8 );

    m_qwtPlot->setAxisScaleDiv(QwtPlot::yLeft, scale);
    Q_EMIT yLeftAxisRangeChanged(this->yLeftAxisRange());
}

void QwtQuick2Plot::updatePlotSize()
{
    if (m_qwtPlot) {
        m_qwtPlot->setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));
        m_qwtPlot->updateLayout();
        updateCanvaSize();
    }
}

QwtQuick2PlotCurve::QwtQuick2PlotCurve(QObject *parent) : QObject(parent)
{
    class QwtPlotCurveEx : public QwtPlotCurve {
        void drawSticks( QPainter *painter,
            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
            const QRectF &canvasRect, int from, int to ) const
        {
            Q_UNUSED( canvasRect )

            painter->save();
            painter->setRenderHint( QPainter::Antialiasing, false );

            const bool doAlign = QwtPainter::roundingAlignment( painter );

            auto xBottomAxisScalDiv = plot()->axisScaleDiv(QwtPlot::xBottom);
            auto first = xMap.transform(0);
            auto second = xMap.transform(1);

            double x0 = xMap.transform( baseline() );
            double y0 = yMap.transform( baseline() );
            if ( doAlign )
            {
                x0 = qRound( x0 );
                y0 = qRound( y0 );
            }

            const Qt::Orientation o = orientation();

            const QwtSeriesData<QPointF> *series = data();

            for ( int i = from; i <= to; i++ )
            {
                const QPointF sample = series->sample( i );
                double xi = xMap.transform( sample.x() );
                double yi = yMap.transform( sample.y() );

                if ( doAlign )
                {
                    xi = qRound( xi );
                    yi = qRound( yi );
                }

                if(qAbs(second - first) > 1)
                {
                    if ( o == Qt::Horizontal )
                        QwtPainter::drawRect( painter, x0, yi, xi, yi );
                    else
                    {
                        auto top = qMin(y0, yi);
                        auto left = xi;
                        auto barWidth = qAbs(second - first);
                        left -= barWidth / 2;

                        auto width = barWidth;
                        auto height = qAbs(top - qMax(y0, yi));

                        QRectF rect(left, top, width, height);
                        QwtPainter::fillRect( painter, rect, pen().color() );
                    }
                } else {
                    if ( o == Qt::Horizontal )
                        QwtPainter::drawLine( painter, x0, yi, xi, yi );
                    else
                        QwtPainter::drawLine( painter, xi, y0, xi, yi );
                }
            }

            painter->restore();
        }

    };

    m_qwtPlotCurve = new QwtPlotCurveEx();
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

QColor QwtQuick2PlotCurve::titleColor() const
{
    auto title = m_qwtPlotCurve->title();
    return title.color();
}

void QwtQuick2PlotCurve::setCurveStyle(QwtQuick2PlotCurve::CurveStyle curveStyle)
{
    m_qwtPlotCurve->setStyle((QwtPlotCurve::CurveStyle) curveStyle);
    Q_EMIT curveStyleChanged(curveStyle);
}

void QwtQuick2PlotCurve::setTitle(QString title)
{
    if (this->title() == title)
        return;

    auto qwtTitle = m_qwtPlotCurve->title();
    qwtTitle.setText(title);
    m_qwtPlotCurve->setTitle(qwtTitle);

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

void QwtQuick2PlotCurve::setTitleColor(QColor titleColor)
{
    if (this->titleColor() == titleColor)
        return;

    auto title = m_qwtPlotCurve->title();
    title.setColor(titleColor);
    m_qwtPlotCurve->setTitle(title);

    Q_EMIT titleColorChanged(this->titleColor());
}


QwtQuick2PlotGrid::QwtQuick2PlotGrid(QObject* parent) : QObject(parent)
{
    m_qwtPlotGrid = new QwtPlotGrid();
}

void QwtQuick2PlotGrid::attach(QwtQuick2Plot *plot)
{
    m_qwtPlotGrid->attach(plot->plot());
}

bool QwtQuick2PlotGrid::enableXMin() const
{
    return m_qwtPlotGrid->xMinEnabled();
}

bool QwtQuick2PlotGrid::enableYMin() const
{
    return m_qwtPlotGrid->yMinEnabled();
}

QColor QwtQuick2PlotGrid::majorPenColor() const
{
    return m_qwtPlotGrid->majorPen().color();
}

qreal QwtQuick2PlotGrid::majorPenWidth() const
{
    return m_qwtPlotGrid->majorPen().widthF();
}

Qt::PenStyle QwtQuick2PlotGrid::majorPenStyle() const
{
    return m_qwtPlotGrid->majorPen().style();
}

QColor QwtQuick2PlotGrid::minorPenColor() const
{
    return m_qwtPlotGrid->minorPen().color();
}

qreal QwtQuick2PlotGrid::minorPenWidth() const
{
    return m_qwtPlotGrid->minorPen().widthF();
}

Qt::PenStyle QwtQuick2PlotGrid::minorPenStyle() const
{
    return m_qwtPlotGrid->minorPen().style();
}

void QwtQuick2PlotGrid::setEnableXMin(bool enableXMin)
{
    if (m_qwtPlotGrid->xMinEnabled() == enableXMin)
        return;

    m_qwtPlotGrid->enableXMin(enableXMin);
    Q_EMIT enableXMinChanged(m_qwtPlotGrid->xMinEnabled());
}

void QwtQuick2PlotGrid::setEnableYMin(bool enableYMin)
{
    if (m_qwtPlotGrid->yMinEnabled() == enableYMin)
        return;

    m_qwtPlotGrid->enableYMin(enableYMin);
    Q_EMIT enableYMinChanged(m_qwtPlotGrid->yMinEnabled());
}

void QwtQuick2PlotGrid::setMajorPenColor(QColor majorPenColor)
{
    if (m_qwtPlotGrid->majorPen().color() == majorPenColor)
        return;

    auto pen = m_qwtPlotGrid->majorPen();
    pen.setColor(majorPenColor);

    m_qwtPlotGrid->setMajorPen(pen);
    Q_EMIT majorPenColorChanged(m_qwtPlotGrid->majorPen().color());
}

void QwtQuick2PlotGrid::setMajorPenWidth(qreal majorPenWidth)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_qwtPlotGrid->majorPen().widthF(), majorPenWidth))
        return;

    auto pen = m_qwtPlotGrid->majorPen();
    pen.setWidthF(majorPenWidth);

    m_qwtPlotGrid->setMajorPen(pen);
    Q_EMIT majorPenWidthChanged(m_qwtPlotGrid->majorPen().widthF());
}

void QwtQuick2PlotGrid::setMajorPenStyle(Qt::PenStyle majorPenStyle)
{
    if (m_qwtPlotGrid->majorPen().style() == majorPenStyle)
        return;

    auto pen = m_qwtPlotGrid->majorPen();
    pen.setStyle(majorPenStyle);

    m_qwtPlotGrid->setMajorPen(pen);
    Q_EMIT majorPenStyleChanged(m_qwtPlotGrid->majorPen().style());
}

void QwtQuick2PlotGrid::setMinorPenColor(QColor minorPenColor)
{
    if (m_qwtPlotGrid->minorPen().color() == minorPenColor)
        return;

    auto pen = m_qwtPlotGrid->minorPen();
    pen.setColor(minorPenColor);

    m_qwtPlotGrid->setMinorPen(pen);
    Q_EMIT minorPenColorChanged(m_qwtPlotGrid->minorPen().color());
}

void QwtQuick2PlotGrid::setMinorPenWidth(qreal minorPenWidth)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_qwtPlotGrid->minorPen().widthF(), minorPenWidth))
        return;

    auto pen = m_qwtPlotGrid->minorPen();
    pen.setWidthF(minorPenWidth);

    m_qwtPlotGrid->setMinorPen(pen);
    Q_EMIT minorPenWidthChanged(m_qwtPlotGrid->minorPen().widthF());
}

void QwtQuick2PlotGrid::setMinorPenStyle(Qt::PenStyle minorPenStyle)
{
    if (m_qwtPlotGrid->minorPen().style() == minorPenStyle)
        return;

    auto pen = m_qwtPlotGrid->minorPen();
    pen.setStyle(minorPenStyle);

    m_qwtPlotGrid->setMinorPen(pen);
    Q_EMIT minorPenStyleChanged(m_qwtPlotGrid->minorPen().style());
}

class PlotPicker: public QwtPlotPicker
{
public:
    explicit PlotPicker( QWidget *canvas, const std::function<void(bool)>& pickerActiveCallback) : QwtPlotPicker(canvas), canvas(canvas),
        pickerActiveCallback(pickerActiveCallback) {}

    virtual QwtText trackerText( const QPoint & p ) const
    {
        auto t = QwtPlotPicker::trackerText(p);
        return t;
    }

    virtual QwtText trackerTextF( const QPointF & p ) const
    {
        auto t = QwtPlotPicker::trackerTextF(p);
        return t;
    }

    virtual void begin() {
        pos = trackerPosition();

        QwtPlotPicker::begin();
    }

    virtual void move( const QPoint & p ) {
        pos = p;

        QwtPlotPicker::move(p);
    }

    virtual void append( const QPoint & p ) {
        pos = p;

        QwtPlotPicker::append(p);
    }

    virtual bool end( bool ok = true ) {
        auto b = QwtPlotPicker::end(ok);

        return b;
    }

    virtual bool eventFilter( QObject *o, QEvent *e) {
        auto b = QwtPlotPicker::eventFilter(o, e);

        return b;
    }

    virtual void widgetMousePressEvent (QMouseEvent *e)
    {
        QwtPlotPicker::widgetMousePressEvent(e);

        if(pickerActiveCallback)
            pickerActiveCallback(true);
    }

    virtual void widgetMouseReleaseEvent (QMouseEvent *e)
    {
        QwtPlotPicker::widgetMouseReleaseEvent(e);

        if(pickerActiveCallback)
            pickerActiveCallback(false);
    }

    virtual void widgetMouseDoubleClickEvent (QMouseEvent *e)
    {
        QwtPlotPicker::widgetMouseDoubleClickEvent(e);
    }

    virtual void widgetMouseMoveEvent (QMouseEvent *e)
    {
        QwtPlotPicker::widgetMouseMoveEvent(e);
    }

    QPointF invTransform( const QPoint & p ) const
    {
        return QwtPlotPicker::invTransform(p);
    }

    QPoint transform( const QPointF & p ) const
    {
        return QwtPlotPicker::transform(p);
    }

    QWidget* canvas;
    std::function<void(bool)> pickerActiveCallback;
    QPoint pos;
};

class PickerDragPointMachine : public QwtPickerDragPointMachine
{
    virtual QList<Command> transition(const QwtEventPattern &ep, const QEvent * e) {
        auto cmdList = QwtPickerDragPointMachine::transition(ep, e);
        return cmdList;
    }
};

QwtQuick2PlotPicker::QwtQuick2PlotPicker(QQuickItem *parent) : QQuickItem(parent)
{
}

void QwtQuick2PlotPicker::attach(QwtQuick2Plot *plot)
{
    m_qwtPlotPicker = new PlotPicker(plot->plot()->canvas(), [this](bool pickerActive) {
        this->setActive(pickerActive);
    });

    m_qwtPlotPicker->setAxis( QwtPlot::xBottom, QwtPlot::yLeft );
    m_qwtPlotPicker->setRubberBand( QwtPlotPicker::CrossRubberBand );
    m_qwtPlotPicker->setRubberBandPen( QColor( Qt::green ) );

    m_qwtPlotPicker->setTrackerMode( QwtPicker::AlwaysOn );
    m_qwtPlotPicker->setTrackerPen( QColor( Qt::black ) );

    m_qwtPlotPicker->setStateMachine( new PickerDragPointMachine () );

    connect(m_qwtPlotPicker, &QwtPlotPicker::moved, this, [&](auto p) {
        auto picker = static_cast<PlotPicker*>(m_qwtPlotPicker);
        setX(picker->pos.x());
        setY(picker->pos.y());

        setPoint(p);
    });

    connect(m_qwtPlotPicker, static_cast<void(QwtPlotPicker::*)(const QPointF&)>(&QwtPlotPicker::selected), this, [&](auto p) {
        auto picker = static_cast<PlotPicker*>(m_qwtPlotPicker);
        setX(picker->pos.x());
        setY(picker->pos.y());

        setPoint(p);
    });

    connect(m_qwtPlotPicker, &QwtPlotPicker::appended, this, [&](auto p) {
        auto picker = static_cast<PlotPicker*>(m_qwtPlotPicker);
        setX(picker->pos.x());
        setY(picker->pos.y());

        setPoint(p);
    });

    if(m_qwtQuickPlot != plot)
    {
        m_qwtQuickPlot = plot;
        Q_EMIT plotItemChanged(m_qwtQuickPlot);
    }
}

bool QwtQuick2PlotPicker::active() const
{
    return m_active;
}

QPointF QwtQuick2PlotPicker::point() const
{
    return m_point;
}

QPoint QwtQuick2PlotPicker::transform(const QPointF &p)
{
    if(m_qwtPlotPicker)
        return static_cast<PlotPicker*>(m_qwtPlotPicker)->transform(p);

    return QPoint(p.x(), p.y());
}

QPointF QwtQuick2PlotPicker::invTransform(const QPoint &p)
{
    if(m_qwtPlotPicker)
        return static_cast<PlotPicker*>(m_qwtPlotPicker)->invTransform(p);

    return QPointF(p.x(), p.y());
}

QwtQuick2Plot *QwtQuick2PlotPicker::plotItem() const
{
    return m_qwtQuickPlot;
}

void QwtQuick2PlotPicker::setActive(bool active)
{
    if (m_active == active)
        return;

    m_active = active;
    Q_EMIT activeChanged(m_active);
}

void QwtQuick2PlotPicker::setPoint(QPointF point)
{
    if (m_point == point)
        return;

    m_point = point;
    Q_EMIT pointChanged(m_point);
}

QwtQuick2PlotLegend::QwtQuick2PlotLegend(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setFlag(QQuickItem::ItemHasContents);
    m_legend = new QwtLegend();
}

QwtQuick2Plot* QwtQuick2PlotLegend::plotItem() const
{
    return m_qwtQuickPlot;
}

void QwtQuick2PlotLegend::setPlotItem(QwtQuick2Plot *plot)
{
    if (m_qwtQuickPlot == plot)
        return;

    if(m_qwtQuickPlot && m_qwtQuickPlot->plot())
    {
        disconnect(m_qwtQuickPlot->plot(), &QwtPlot::legendDataChanged, m_legend, &QwtLegend::updateLegend);
    }

    m_qwtQuickPlot = plot;
    connect(m_qwtQuickPlot->plot(), &QwtPlot::legendDataChanged, m_legend, &QwtLegend::updateLegend);
    m_qwtQuickPlot->plot()->updateLayout();

    Q_EMIT plotItemChanged(m_qwtQuickPlot);
}

void QwtQuick2PlotLegend::paint(QPainter *painter)
{
    m_legend->renderLegend(painter, this->boundingRect(), true);
}

void QwtQuick2PlotLegend::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
    m_legend->setGeometry(newGeometry.toRect());
    update();
}
