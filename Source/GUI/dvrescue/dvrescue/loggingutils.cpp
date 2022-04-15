#include "loggingutils.h"

QList<QLoggingCategory*> LoggingUtils::categories;
QMap<QString, QLoggingCategory*> LoggingUtils::categoryByName;

LoggingUtils::LoggingUtils(QObject *parent)
    : QObject{parent}
{

}

QLoggingCategory::CategoryFilter filter;
void LoggingUtils::categoryFilter(QLoggingCategory* category) {

    if(QString::fromUtf8(category->categoryName()).startsWith("dvrescue")) {
        category->setEnabled(QtDebugMsg, false);
        category->setEnabled(QtWarningMsg, false);
        category->setEnabled(QtCriticalMsg, false);
        category->setEnabled(QtFatalMsg, false);
        category->setEnabled(QtInfoMsg, false);
        category->setEnabled(QtSystemMsg, false);
    } else {
        filter(category);
    }

    if(categoryByName.find(category->categoryName()) == categoryByName.end()) {
        categories.append(category);
        categoryByName[category->categoryName()] = category;
    }
}

void LoggingUtils::installFilter()
{
    filter = QLoggingCategory::installFilter(categoryFilter);
}

void LoggingUtils::uninstallFilter()
{
    QLoggingCategory::installFilter({});
}

bool LoggingUtils::isDebugEnabled(const QString &category) const
{
    auto it = categoryByName.find(category);
    auto cat = it != categoryByName.end() ? it.value() : nullptr;
    auto enabled = cat ? cat->isDebugEnabled() : false;

    qDebug() << "cat: " << cat << "enabled: " << enabled;
    return enabled;
}
