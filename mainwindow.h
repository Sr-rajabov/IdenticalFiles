#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <QStandardItemModel>
#include <QMutex>
#include <filehashmodel.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openFolder_clicked();
    void startSearch();
    void onSearchFinished(const QStringList &duplicates, const QMap<QString, QStringList> &fileMap, const QMap<QString, qint64> &fileSizes);
    void on_update_clicked();

    void on_deleteAll_clicked();


    void on_trash_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::MainWindow *ui;
    FileHashModel* model;
    FileHashModel* treeModel;
    QString dir;
    QMutex mtx;
    QMap<QString, QString> pathHash;
    bool check = false;
};
#endif // MAINWINDOW_H
