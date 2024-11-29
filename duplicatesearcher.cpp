#include "duplicatesearcher.h"
#include <QFile>
#include <QDir>
#include <QFileInfoList>
#include <QFileIconProvider>
#include <QCryptographicHash>

duplicateSearcher::duplicateSearcher(){}

void duplicateSearcher::searchDuplicates(const QString &dirPath)
{
    QMap<QString, QStringList> fileMap; // Хэш -> список файлов
    QMap<QString, qint64> fileSizes; // Хэш -> размер файла

    // Рекурсивно обходим директорию
    QDir dir(dirPath);
    addFilesWithHash(dir, fileMap, fileSizes);

    // Поиск дубликатов
    QStringList duplicates;
    for (const auto &filePair : fileMap) {
        if (filePair.size() > 1) {  // У нас есть дубликаты
            duplicates.append(filePair.join(", ")); // Формируем строку дубликатов
        }
    }
    emit updateProgress(64);
    emit searchFinished(duplicates, fileMap, fileSizes);
}

void duplicateSearcher::addFilesWithHash(const QDir &dir, QMap<QString, QStringList> &fileMap, QMap<QString, qint64> &fileSizes)
{
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

    // Обрабатываем файлы
    for (const QFileInfo &fileInfo : fileInfoList) {
        QString filePath = fileInfo.absoluteFilePath();
        QString fileHash = calculateFileHash(filePath);
        if (!fileHash.isEmpty()) {
            fileMap[fileHash].append(filePath); // Добавляем путь к файлу по хэшу
            fileSizes[fileHash] = fileInfo.size(); // Запоминаем размер файла
        }
    }

    // Обрабатываем подкаталоги
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
        return QString(); // Не удалось открыть файл
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return QString(); // Не удалось прочитать файл
    }
    file.close();
    return hash.result().toHex(); // Возвращаем хэш в шестнадцатеричном формате
}
