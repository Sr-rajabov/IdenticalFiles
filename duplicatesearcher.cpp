#include "duplicatesearcher.h"
#include <QFile>
#include <QDir>
#include <QFileInfoList>
#include <QFileIconProvider>
#include <QCryptographicHash>

duplicateSearcher::duplicateSearcher(){}

void duplicateSearcher::searchDuplicates(const QString &dirPath)
{
    QMap<QString, QStringList> fileMap;
    QMap<QString, qint64> fileSizes;

    QDir dir(dirPath);
    addFilesWithHash(dir, fileMap, fileSizes);

    QStringList duplicates;
    for (const auto &filePair : fileMap) {
        if (filePair.size() > 1) {
            duplicates.append(filePair.join(", "));
        }
    }
    emit updateProgress(64);
    emit searchFinished(duplicates, fileMap, fileSizes);
}

void duplicateSearcher::addFilesWithHash(const QDir &dir, QMap<QString, QStringList> &fileMap, QMap<QString, qint64> &fileSizes)
{
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

    for (const QFileInfo &fileInfo : fileInfoList) {
        QString filePath = fileInfo.absoluteFilePath();
        QString fileHash = calculateFileHash(filePath);
        if (!fileHash.isEmpty()) {
            fileMap[fileHash].append(filePath);
            fileSizes[fileHash] = fileInfo.size();
        }
    }

    QFileInfoList dirInfoList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const QFileInfo &dirInfo : dirInfoList) {
        QDir subDir = dir.absoluteFilePath(dirInfo.fileName());
        addFilesWithHash(subDir, fileMap, fileSizes);
    }
}

QString duplicateSearcher::calculateFileHash(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        return QString();
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return QString();
    }
    file.close();
    return hash.result().toHex();
}
