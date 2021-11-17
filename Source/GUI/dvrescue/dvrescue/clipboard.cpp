#include "clipboard.h"
#include <QGuiApplication>
#include <QClipboard>

Clipboard::Clipboard(QObject *parent) : QObject(parent)
{

}

void Clipboard::setText(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
}
