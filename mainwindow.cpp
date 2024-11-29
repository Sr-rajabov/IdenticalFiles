#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <duplicatesearcher.h>
#include <filehashmodel.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QThread>
#include <QHeaderView>
#include <QList>
#include <QFileIconProvider>
#include <QFile>
#include <algorithm>
#include <QTableView>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new FileHashModel(this);
    model->setColumnCount(2);
    model->setHorizontalHeaderLabels(QStringList() << "Name" << "Size");
    ui->tableView->setModel(model);
    treeModel = new FileHashModel(this);
    ui->treeView->setModel(treeModel);
    ui->tableView->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openFolder_clicked()
{
    if (check) {
        QMessageBox::information(this, "Delete", "Indexing files");
        return;
    }
    dir = QFileDialog::getExistingDirectory(this, "Select directory", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        QMessageBox::warning(this, "Open folder", "Folder is empty");
        return;
    }
    ui->dirPath->setText(dir);
    treeModel->clear();
}

void MainWindow::startSearch()
{
    check = true;
    QMutexLocker locker(&mtx);
    ui->progressBar->setValue(0);
    QThread *thread = QThread::create([=](){
        duplicateSearcher searcher;
        connect(&searcher, &duplicateSearcher::updateProgress, this, [=](int value) {
            ui->progressBar->setValue(value);
        });

        connect(&searcher, &duplicateSearcher::searchFinished, this, &MainWindow::onSearchFinished);
        searcher.searchDuplicates(dir);
    });

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
    check = false;
}

void MainWindow::onSearchFinished(const QStringList &duplicates, const QMap<QString, QStringList> &fileMap, const QMap<QString, qint64> &fileSizes)
{
    check = true;
    if (duplicates.isEmpty()) {
        QMessageBox::information(this, "Result", "No duplicates were found");
        ui->statusbar->clearMessage();
        ui->progressBar->setValue(0);
        return;
    } else {
        int count = 0;
        for (const auto &filePair : fileMap) {
            if (filePair.size() > 1) {  // Если есть дубликаты

                int counter = 1;
                auto it = std::min_element(filePair.begin(), filePair.end(),[](const QString& a, const QString& b) {
                    return a.size() < b.size();
                });

                    QString fileHash = duplicateSearcher::calculateFileHash(*it);
                    QFileIconProvider provider;

                    QFileInfo fileinfo(*it);

                    QStandardItem* treeItem = new QStandardItem(provider.icon(fileinfo), fileinfo.fileName());

                    treeModel->addTree(QString::number(count + 1), treeItem, fileSizes[fileHash] > 1024 ?  QString::number(fileSizes[fileHash] / 1024) + " KB" : QString::number(fileSizes[fileHash]));

                for (const QString &filePath : filePair) {
                    QString fileHash = duplicateSearcher::calculateFileHash(filePath);
                    QFileIconProvider provider;

                    QFileInfo fileinfo(filePath);

                    QStandardItem* treeItem = new QStandardItem(provider.icon(fileinfo), fileinfo.fileName());
                    if (filePath == *it) continue;
                    QStandardItem* num = new QStandardItem(QString::number(counter));
                    QStandardItem* size = new QStandardItem(fileSizes[fileHash] > 1024 ?  QString::number(fileSizes[fileHash] / 1024) + " KB" : QString::number(fileSizes[fileHash]));
                    QList<QStandardItem*>rowItems;
                    rowItems << num << treeItem << size;
                    treeModel->item(treeModel->rowCount() - 1)->appendRow(rowItems);
                    pathHash[fileinfo.fileName()].append(filePath);
                    ++counter;
                }
                ++count;
            }
        }
        ui->statusbar->showMessage("Found: " + QString::number(count));
    }
    ui->tableView->resizeColumnToContents(0);
    ui->tableView->resizeColumnToContents(1);
    ui->progressBar->setValue(100);
    treeModel->setHorizontalHeaderLabels(QStringList() << "№" << "Name" << "Size");
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Столбец "Имя" растягивается на всё доступное место
    ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->treeView->show();
    check = false;

}


void MainWindow::on_update_clicked()
{
    check = true;
    ui->statusbar->showMessage("Working...");
    treeModel->clear();
    pathHash.clear();
    startSearch();
    check = false;

}


void MainWindow::on_deleteAll_clicked()
{
    QMutexLocker locker(&mtx);
    if (check) {
        QMessageBox::information(this, "Delete", "Indexing files");
        return;
    }
    if (QMessageBox::question(this, "delete", "Sure want to delete it?") == QMessageBox::Yes) {

        for (int row = 0; row < treeModel->rowCount(); ++row) {
            QStandardItem* mainItem = treeModel->item(row);
            if (mainItem) {

                for (int childRow = 0; childRow < mainItem->rowCount(); ++childRow) {
                    QStandardItem *childItem = mainItem->child(childRow, 1);
                    QString path;

                    if (childItem) {
                        // Добавляем дочерний элемент в список
                        path = childItem->text();
                        QString filePath = pathHash[path];
                        QFileInfo fileinfo(filePath);
                        QFileIconProvider provider;
                        QStandardItem* name = new QStandardItem(provider.icon(fileinfo), fileinfo.fileName());
                        QStandardItem *size = new QStandardItem(fileinfo.size() > 1024 ? QString::number(fileinfo.size() / 1024) + "KB" : QString::number(fileinfo.size()));
                        model->appendRow(QList<QStandardItem*>() << name << size);

                        if (!QFileInfo::exists(filePath)) {
                            QMessageBox::warning(this, "delete", "File doesn't exist");
                            return;
                        }

                        if (QFile::remove(filePath)){
                        }
                        else {
                            QMessageBox::warning(this, "Error", "This file could not be deleted.");
                        }
                    }
                }
                while(mainItem->rowCount() > 0) {
                    mainItem->removeRow(0);
                }
            }
        }
    }
    ui->tableView->resizeColumnToContents(0);
    ui->tableView->resizeColumnToContents(1);
}



void MainWindow::on_trash_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::Checked) {
        ui->tableView->show();
    }
    else ui->tableView->hide();
}

