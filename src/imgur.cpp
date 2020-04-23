#include "imgur.h"

MessageBoxResponse MessageBoxHelper::mapReplyToMessageBoxResponse(int reply) {
    switch (reply) {
        case QMessageBox::Yes:
            return MessageBoxResponse::Yes;
        case QMessageBox::No:
            return MessageBoxResponse::No;
        case QMessageBox::Cancel:
            return MessageBoxResponse::Cancel;
        default:
            return MessageBoxResponse::No;
    }
}

bool UploadOperation::execute() {
    if (!image_.isNull() && proceedWithUpload()) {
        uploader_->upload(image_);
        return true;
    }
    return false;
}

ImgurUploader::ImgurUploader(QObject *parent) : QObject(parent),
                                                access_manager_(new QNetworkAccessManager(this)) {
    connect(access_manager_, &QNetworkAccessManager::finished,
            this, &ImgurUploader::handleReply);

    client_id_ = "16d41e28a3ba71e";
}

void ImgurUploader::startUpload(const QImage &image, const QByteArray &accessToken) const {

    QByteArray imageByteArray;
    QBuffer buffer(&imageByteArray);
    image.save(&buffer, "PNG");

    QUrl url(QStringLiteral("https://api.imgur.com/3/upload.xml"));
    QUrlQuery urlQuery;

    urlQuery.addQueryItem(QStringLiteral("title"), QStringLiteral("Ksnip Screenshot"));
    urlQuery.addQueryItem(QStringLiteral("description"), QStringLiteral("Screenshot uploaded via Ksnip"));

    url.setQuery(urlQuery);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    if (accessToken.isEmpty()) {
        request.setRawHeader("Authorization", "Client-ID " + client_id_);
    } else {
        request.setRawHeader("Authorization", "Bearer " + accessToken);
    }

    access_manager_->post(request, imageByteArray);
}


void ImgurUploader::getAccessToken(const QByteArray &pin,
                                   const QByteArray &clientId,
                                   const QByteArray &clientSecret) const {
    QNetworkRequest request;

    request.setUrl(QUrl(QStringLiteral("https://api.imgur.com/oauth2/token.xml")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QByteArray params;
    params.append("client_id=" + clientId);
    params.append("&client_secret=" + clientSecret);
    params.append("&grant_type=pin");
    params.append("&pin=" + pin);

    access_manager_->post(request, params);
}

void ImgurUploader::refreshToken(const QByteArray &refreshToken, const QByteArray &clientId,
                                 const QByteArray &clientSecret) const {
    QNetworkRequest request;

    request.setUrl(QUrl(QStringLiteral("https://api.imgur.com/oauth2/token.xml")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QByteArray params;
    params.append("refresh_token=" + refreshToken);
    params.append("&client_id=" + clientId);
    params.append("&client_secret=" + clientSecret);
    params.append("&grant_type=refresh_token");

    access_manager_->post(request, params);
}

QUrl ImgurUploader::pinRequestUrl(const QString &clientId) const {
    QUrl url(QStringLiteral("https://api.imgur.com/oauth2/authorize"));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("client_id"), clientId);
    urlQuery.addQueryItem(QStringLiteral("response_type"), QStringLiteral("pin"));

    url.setQuery(urlQuery);
    return url;
}

void ImgurUploader::handleDataResponse(const QDomElement &element) const {
    if (element.attribute(QStringLiteral("status")) == QStringLiteral("200") &&
        !element.elementsByTagName(QStringLiteral("link")).isEmpty()) {
        auto link = element.elementsByTagName(QStringLiteral("link")).at(0).toElement().text();
        auto deleteHash = element.elementsByTagName(QStringLiteral("deletehash")).at(0).toElement().text();

        emit uploadFinished(UploadResponse(link, deleteHash));
    } else if (element.attribute(QStringLiteral("status")) == QStringLiteral("403")) {
        emit tokenRefreshRequired();
    } else {
        if (element.elementsByTagName(QStringLiteral("error")).isEmpty()) {
            emit error(QStringLiteral("Server responded with ") + element.attribute(QStringLiteral("status")));
        } else {
            emit error(QStringLiteral("Server responded with ") + element.attribute(QStringLiteral("status")) + ": " +
                       element.elementsByTagName(QStringLiteral("error")).at(0).toElement().text());
        }
    }
}

void ImgurUploader::handleTokenResponse(const QDomElement &element) const {
    if (!element.elementsByTagName(QStringLiteral("access_token")).isEmpty() &&
        !element.elementsByTagName(QStringLiteral("refresh_token")).isEmpty() &&
        !element.elementsByTagName(QStringLiteral("account_username")).isEmpty()
            ) {
        emit tokenUpdated(element.elementsByTagName(QStringLiteral("access_token")).at(0).toElement().text(),
                          element.elementsByTagName(QStringLiteral("refresh_token")).at(0).toElement().text(),
                          element.elementsByTagName(QStringLiteral("account_username")).at(0).toElement().text()
        );
    } else {
        emit error(QStringLiteral("Expected token response was received, something went wrong."));
    }
}

void ImgurUploader::handleReply(QNetworkReply *reply) {

    if (reply->error() != QNetworkReply::NoError &&
        reply->error() != QNetworkReply::ContentOperationNotPermittedError) {
        emit error(QStringLiteral("Network Error(") + QString::number(reply->error()) + "): " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QDomDocument doc;
    QString errorMessage;
    int errorLine;
    int errorColumn;

    if (!doc.setContent(reply->readAll(), false, &errorMessage, &errorLine, &errorColumn)) {
        emit error(QStringLiteral("Parse error: ") + errorMessage + QStringLiteral(", line:") + errorLine +
                   QStringLiteral(", column:") + errorColumn);
        reply->deleteLater();
        return;
    }

    auto rootElement = doc.documentElement();

    if (rootElement.tagName() == QStringLiteral("data")) {
        handleDataResponse(rootElement);
    } else if (rootElement.tagName() == QStringLiteral("response")) {
        handleTokenResponse(rootElement);
    } else {
        emit error(QStringLiteral("Received unexpected reply from imgur server."));
    }

    reply->deleteLater();
}

CaptureUploader::CaptureUploader() {
    imgur_uploader_ = new ImgurUploader();
    imgur_response_logger = new ImgurResponseLogger();

    connect(imgur_uploader_, &ImgurUploader::uploadFinished, this, &CaptureUploader::imgurUploadFinished);
    connect(imgur_uploader_, &ImgurUploader::error, this, &CaptureUploader::imgurError);
    connect(imgur_uploader_, &ImgurUploader::tokenUpdated, this, &CaptureUploader::imgurTokenUpdated);
    connect(imgur_uploader_, &ImgurUploader::tokenRefreshRequired, this, &CaptureUploader::imgurTokenRefresh);
}

CaptureUploader::~CaptureUploader() {
    delete imgur_uploader_;
    delete imgur_response_logger;
}

void CaptureUploader::upload(const QImage &image) {
    image_ = image;
    imgur_uploader_->startUpload(image_);
}

void CaptureUploader::imgurUploadFinished(const UploadResponse &response) {
    qInfo("%s", qPrintable(tr("Upload to imgur.com finished!")));
    emit finished(response.link());

    imgur_response_logger->log(response);

    image_ = QImage();
}

void CaptureUploader::imgurError(const QString &message) {
    qCritical("ImageViewer: Imgur uploader returned error: '%s'", qPrintable(message));
    emit error(message);
}

void CaptureUploader::imgurTokenUpdated() {
        qInfo("%s", qPrintable(tr("Received new token")));
    upload(image_);
}

void CaptureUploader::imgurTokenRefresh() {
    qInfo("%s", qPrintable(tr("Imgur token has expired")));
}

ImgurResponseLogger::ImgurResponseLogger() {
    log_filename_ = QStringLiteral("imgur_history.txt");
    log_path_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    log_file_path_ = log_path_ + QStringLiteral("/") + log_filename_;
}

void ImgurResponseLogger::log(const UploadResponse &response) {
    createPathIfRequired();
    auto logEntry = getLogEntry(response);

    writeLogEntry(logEntry);
}

void ImgurResponseLogger::writeLogEntry(const QString &logEntry) const {
    QFile file(log_file_path_);
    auto fileOpened = file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
    if (fileOpened) {
        QTextStream stream(&file);
        stream << logEntry << endl;
    }
}

void ImgurResponseLogger::createPathIfRequired() const {
    QDir qdir;
    qdir.mkpath(log_path_);
}

QString ImgurResponseLogger::getLogEntry(const UploadResponse &response) const {
    auto separator = QStringLiteral(",");
    auto deleteLink = QStringLiteral("https://imgur.com/delete/") + response.deleteHash();
    auto timestamp = response.timeStamp().toString(QStringLiteral("dd.MM.yyyy hh:mm:ss"));
    return timestamp + separator + response.link() + separator + deleteLink;
}

QStringList ImgurResponseLogger::getLogs() const {
    auto logEntries = QStringList();
    QFile file(log_file_path_);
    auto fileOpened = file.open(QIODevice::ReadOnly);
    if (fileOpened) {

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            auto entry = stream.readLine();
            logEntries.append(entry);
        }
    }
    return logEntries;
}
