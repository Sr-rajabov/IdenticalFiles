#include "filehashmodel.h"
#include <QFileIconProvider>

FileHashModel::FileHashModel(QObject *parent)
    : QStandardItemModel{parent}
{
    setHorizontalHeaderLabels(QStringList() << "№" << "Name" << "Size");

}

void FileHashModel::addFile(QStandardItem* name, QString size)
{
    appendRow(QList<QStandardItem *>() << name << new QStandardItem(size));
}

void FileHashModel::addTree(QString num, QStandardItem *name, QString size)
{
    appendRow(QList<QStandardItem *>() << new QStandardItem(num) << name << new QStandardItem(size));
}
