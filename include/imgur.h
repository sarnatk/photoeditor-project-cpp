#ifndef PHOTOEDITOR_IMGUR_H
#define PHOTOEDITOR_IMGUR_H

#include <QCoreApplication>
#include <QString>
#include <QDateTime>
#include <QObject>
#include <QUrl>
#include <QUrlQuery>
#include <QtNetwork/QNetworkReply>
#include <QtXml/QDomDocument>
#include <QImage>
#include <QBuffer>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QtGui>
#include <QDir>
#include <QMessageBox>

enum class MessageBoxResponse {
    Yes, No, Cancel
};

class MessageBoxHelper {
public:
    static bool yesNo(const QString &title, const QString &question) {
        auto reply = QMessageBox::question(nullptr, title, question, QMessageBox::Yes | QMessageBox::No);
        return reply == QMessageBox::Yes;
    }

    static MessageBoxResponse yesNoCancel(const QString &title, const QString &question) {
        auto reply = QMessageBox::question(nullptr, title, question,
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        return mapReplyToMessageBoxResponse(reply);
    }

    static void ok(const QString &title, const QString &info) {
        QMessageBox::question(nullptr, title, info, QMessageBox::Ok);
    }

private:
    static MessageBoxResponse mapReplyToMessageBoxResponse(int reply);
};

class UploadResponse {
public:
    explicit UploadResponse(const QString &link, const QString &deleteHash) {
        link_ = link;
        delete_hash_ = deleteHash;
        time_stamp_ = QDateTime::currentDateTime();
    }

    ~UploadResponse() = default;

    [[nodiscard]] QString link() const { return link_; }

    [[nodiscard]] QString deleteHash() const { return delete_hash_; }

    [[nodiscard]] QDateTime timeStamp() const { return time_stamp_; }

private:
    QString link_;
    QString delete_hash_;
    QDateTime time_stamp_;
};


class ImgurResponseLogger {
public:
    explicit ImgurResponseLogger();

    ~ImgurResponseLogger() = default;

    void log(const UploadResponse &response);

    QStringList getLogs() const;

private:
    QString log_filename_;
    QString log_path_;
    QString log_file_path_;

    void createPathIfRequired() const;

    QString getLogEntry(const UploadResponse &response) const;

    void writeLogEntry(const QString &logEntry) const;
};


class ImgurUploader : public QObject {
Q_OBJECT

public:
    explicit ImgurUploader(QObject *parent = nullptr);

    void startUpload(const QImage &image, const QByteArray &accessToken = nullptr) const;

    void getAccessToken(const QByteArray &pin, const QByteArray &clientId, const QByteArray &clientSecret) const;

    void refreshToken(const QByteArray &refreshToken, const QByteArray &clientId, const QByteArray &clientSecret) const;

    [[nodiscard]] QUrl pinRequestUrl(const QString &clientId) const;

signals:

    void uploadFinished(const UploadResponse &response) const;

    void error(const QString &message) const;

    void tokenUpdated(const QString &accessToken, const QString &refreshToken, const QString &username) const;

    void tokenRefreshRequired() const;

private:
    QNetworkAccessManager *access_manager_;
    QByteArray client_id_;

    void handleDataResponse(const QDomElement &element) const;

    void handleTokenResponse(const QDomElement &element) const;

private slots:

    void handleReply(QNetworkReply *reply);
};

class CaptureUploader : public QObject {
Q_OBJECT

public:
    explicit CaptureUploader();

    ~CaptureUploader() override;

    void upload(const QImage &image);

signals:

    void finished(const QString &url) const;

    void error(const QString &error) const;

private:
    ImgurUploader *imgur_uploader_;
    ImgurResponseLogger *imgur_response_logger;
    QImage image_;

private slots:

    void imgurUploadFinished(const UploadResponse &response);

    void imgurError(const QString &message);

    void imgurTokenUpdated();

    void imgurTokenRefresh();
};

class UploadOperation : public QObject {
Q_OBJECT

public:
    UploadOperation(const QImage &image, CaptureUploader *uploader) {
        image_ = image;
        uploader_ = uploader;
    }

    ~UploadOperation() override = default;

    bool execute();

private:
    CaptureUploader *uploader_;
    QImage image_;

    [[nodiscard]] bool proceedWithUpload() const {
        return getProceedWithUpload();
    }

    [[nodiscard]] bool getProceedWithUpload() const {
        return MessageBoxHelper::yesNo(tr("Imgur Upload"), tr("Upload to imgur.com?"));
    }
};

#endif //PHOTOEDITOR_IMGUR_H
