#ifndef SORTFILTERTABLEMODEL_H
#define SORTFILTERTABLEMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QQmlListProperty>
#include <QJSValue>

class SortFilterTableModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QAbstractTableModel* tableModel READ tableModel WRITE setTableModel NOTIFY tableModelChanged)
    Q_PROPERTY(int columnCount READ columnCount NOTIFY columnCountChanged FINAL)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged FINAL)
    Q_PROPERTY(QJSValue rowFilter READ rowFilter WRITE setRowFilter NOTIFY rowFilterChanged)

public:
    SortFilterTableModel(QObject *parent = nullptr);
    Q_INVOKABLE QVariant getRow(int rowIndex);
    Q_INVOKABLE void setRow(int rowIndex, const QVariant &row);

    /*
    Q_INVOKABLE void appendRow(const QVariant &row);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void insertRow(int rowIndex, const QVariant &row);
    Q_INVOKABLE void moveRow(int fromRowIndex, int toRowIndex, int rows = 1);
    Q_INVOKABLE void removeRow(int rowIndex, int rows = 1);
    */

    Q_INVOKABLE void resetInitialSort();
    Q_INVOKABLE int fromSourceRowIndex(int rowIndex);
    Q_INVOKABLE int toSourceRowIndex(int rowIndex);
    Q_INVOKABLE void setFilterText(int column, QString filterText);
    Q_INVOKABLE void invalidateFilter();

    Q_INVOKABLE void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE Qt::SortOrder initialSortOrder(int column) const;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    Q_INVOKABLE virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QAbstractTableModel* tableModel() const;

    const QJSValue &rowFilter() const;
    void setRowFilter(const QJSValue &newRowFilter);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

public Q_SLOTS:
    void setTableModel(QAbstractTableModel* tableModel);
    void initRoles();

private Q_SLOTS:
    void resetInternalData();

Q_SIGNALS:
    void filterTextChanged(QString filterText);
    void tableModelChanged(QAbstractTableModel* tableModel);

    void columnCountChanged(int columnCount);
    void rowCountChanged(int rowCount);

    void rowFilterChanged();

private:
    QVector<QString> m_filters;
    QAbstractTableModel* m_tableModel;
    int m_columnCount;
    int m_rowCount;
    QVariant m_rows;
    mutable QJSValue m_rowFilter;
};

#endif // SORTFILTERTABLEMODEL_
