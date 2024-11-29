#ifndef DUPLICATESEARCHER_H
#define DUPLICATESEARCHER_H

#include <QObject>
#include <QDir>

class duplicateSearcher : public QObject
{
    Q_OBJECT
public:
    duplicateSearcher();

public slots:
    void searchDuplicates(const QString& dirPath);

signals:
    void searchFinished(const QStringList& duplicates, const QMap<QString, QStringList>& fileMap, const QMap<QString, qint64>& fileSizes);
    void updateProgress(int);
private:

    void addFilesWithHash(const QDir& dir, QMap<QString, QStringList>& fileMap, QMap<QString, qint64>& fileSizes);
public:
    static QString calculateFileHash(const QString &filePath);

};

#endif // DUPLICATESEARCHER_H
