#ifndef FILEHASHMODEL_H
#define FILEHASHMODEL_H

#include <QStandardItemModel>

class FileHashModel : public QStandardItemModel
{
public:
    explicit FileHashModel(QObject *parent = nullptr);

public:
    void addFile(QStandardItem* name, QString size);
    void addTree(QString num, QStandardItem* name, QString size);
};

#endif // FILEHASHMODEL_H
