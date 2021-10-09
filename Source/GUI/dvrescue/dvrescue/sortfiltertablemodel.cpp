#include "sortfiltertablemodel.h"
#include <QDebug>
#include <QMetaObject>
#include <QJSEngine>

SortFilterTableModel::SortFilterTableModel(QObject *parent)
  : QSortFilterProxyModel (parent)
{
}

QVariant SortFilterTableModel::getRow(int rowIndex)
{
    auto proxyIndex = index(rowIndex, 0);
    auto modelIndex = mapToSource(proxyIndex);

    QVariant row;
    QMetaObject::invokeMethod(sourceModel(), "getRow",  Q_RETURN_ARG(QVariant, row), Q_ARG(int, modelIndex.row()));

    return row;
}

void SortFilterTableModel::setRow(int rowIndex, const QVariant &row)
{
    auto proxyIndex = index(rowIndex, 0);
    auto sourceIndex = mapToSource(proxyIndex);

    QMetaObject::invokeMethod(m_tableModel, "setRow", Q_ARG(int, sourceIndex.row()), Q_ARG(QVariant, row));
}

void SortFilterTableModel::resetInitialSort()
{
    QSortFilterProxyModel::sort(-1);
}

int SortFilterTableModel::fromSourceRowIndex(int rowIndex)
{
    auto sourceIndex = sourceModel()->index(rowIndex, 0);
    auto modelIndex = mapFromSource(sourceIndex);

    return modelIndex.row();
}

int SortFilterTableModel::toSourceRowIndex(int rowIndex)
{
    auto proxyIndex = index(rowIndex, 0);
    auto modelIndex = mapToSource(proxyIndex);

    return modelIndex.row();
}

void SortFilterTableModel::setFilterText(int column, QString filterText)
{
    if(m_filters.size() != columnCount())
        m_filters.resize(columnCount());

    m_filters[column] = filterText;
    invalidateFilter();
}

void SortFilterTableModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}

void SortFilterTableModel::sort(int column, Qt::SortOrder order)
{
    QSortFilterProxyModel::sort(column, order);
}

int SortFilterTableModel::columnCount(const QModelIndex &parent) const
{
    auto count = QSortFilterProxyModel::columnCount(parent);
    return count;
}

int SortFilterTableModel::rowCount(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::rowCount(parent);
}

Qt::SortOrder SortFilterTableModel::initialSortOrder(int column) const
{
    bool ok = false;
    if (column < 0 || column >= m_tableModel->columnCount())
        return Qt::AscendingOrder;
    int ret = m_tableModel->data(m_tableModel->index(0, column), Qt::InitialSortOrderRole).toInt(&ok);
    if (ok)
        return Qt::SortOrder(ret);
    else
        return Qt::AscendingOrder;
}

QModelIndex SortFilterTableModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    int row = QSortFilterProxyModel::mapFromSource(sourceIndex).row();
    return m_tableModel->index(row, sourceIndex.column());
}

QModelIndex SortFilterTableModel::mapToSource(const QModelIndex &proxyIndex) const
{
    QModelIndex rowIndex = QSortFilterProxyModel::mapToSource(proxyIndex);
    int col = -1;
    if (proxyIndex.column() >= 0 && proxyIndex.column() < m_tableModel->columnCount())
        col = proxyIndex.column();
    return m_tableModel->index(rowIndex.row(), col);
}

QVariant SortFilterTableModel::data(const QModelIndex &index, int role) const
{
    return QSortFilterProxyModel::data(index, role);
}

QHash<int, QByteArray> SortFilterTableModel::roleNames() const
{
    return m_tableModel->roleNames();
}

QAbstractTableModel *SortFilterTableModel::tableModel() const
{
    return m_tableModel;
}

bool SortFilterTableModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if(m_filters.empty() && !m_rowFilter.isCallable())
        return true;

    for(auto i = 0; i < m_filters.size(); ++i) {
        if(m_filters[i].isEmpty())
            continue;

        QModelIndex index = sourceModel()->index(sourceRow, i, sourceParent);
        auto data = sourceModel()->data(index).toString();
        if(!data.contains(m_filters[i]))
            return false;
    }

    if(m_rowFilter.isCallable()) {
        auto result = m_rowFilter.call(QJSValueList { QJSValue(sourceRow) });
        if(result.isBool() && result.toBool() == false)
            return false;
    }

    return true;
}

bool SortFilterTableModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    return QSortFilterProxyModel::lessThan(sourceLeft, sourceRight);
}

void SortFilterTableModel::setTableModel(QAbstractTableModel *tableModel)
{
    if (m_tableModel == tableModel)
        return;

    m_tableModel = tableModel;
    if (m_tableModel && m_tableModel->roleNames().isEmpty()) { // workaround for when a model has no roles and roles are added when the model is populated (ListModel)
            // QTBUG-57971
        connect(m_tableModel, &QAbstractItemModel::rowsInserted, this, &SortFilterTableModel::initRoles);
    }

    setSourceModel(m_tableModel);

    Q_EMIT tableModelChanged(m_tableModel);
}

void SortFilterTableModel::initRoles()
{
    disconnect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &SortFilterTableModel::initRoles);
    resetInternalData();
}

void SortFilterTableModel::resetInternalData()
{
    QSortFilterProxyModel::resetInternalData();
}

const QJSValue &SortFilterTableModel::rowFilter() const
{
    return m_rowFilter;
}

void SortFilterTableModel::setRowFilter(const QJSValue &newRowFilter)
{
    if (m_rowFilter.equals(newRowFilter))
        return;
    m_rowFilter = newRowFilter;
    Q_EMIT rowFilterChanged();
}
