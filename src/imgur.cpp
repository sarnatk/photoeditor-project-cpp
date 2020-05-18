#include "imgur.h"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QShortcut>
#include <QDrag>
#include <QMimeData>
#include <QBuffer>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>

ImgurUploader::ImgurUploader(const QPixmap &capture, QWidget *parent) : QWidget(parent), pixmap(capture) {
    setWindowTitle(tr("Upload to Imgur"));

    infoLabel = new QLabel(tr("Uploading Image"));

    vLayout = new QVBoxLayout();
    setLayout(vLayout);
    vLayout->addWidget(infoLabel);

    NetworkAM = new QNetworkAccessManager(this);
    connect(NetworkAM, &QNetworkAccessManager::finished, this, &ImgurUploader::handleReply);

    setAttribute(Qt::WA_DeleteOnClose);
    upload();
}

void ImgurUploader::handleReply(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        QJsonObject data = json[QStringLiteral("data")].toObject();
        imageURL.setUrl(data[QStringLiteral("link")].toString());
        deleteImageURL.setUrl(QStringLiteral("https://imgur.com/delete/%1").arg(
                data[QStringLiteral("deletehash")].toString()));
        onUploadOk();
    } else {
        infoLabel->setText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ImgurUploader::startDrag() {
    auto *mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl>{imageURL});
    mimeData->setImageData(pixmap);

    auto *dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    dragHandler->exec();
}

void ImgurUploader::upload() {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap.save(&buffer, "PNG");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("title"), QStringLiteral("image"));

    QUrl url(QStringLiteral("https://api.imgur.com/3/image"));
    url.setQuery(urlQuery);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/application/x-www-form-urlencoded");
    request.setRawHeader("Authorization", QStringLiteral("Client-ID %1").arg("313baf0c7b4d3ff").toUtf8());

    NetworkAM->post(request, byteArray);
}

void ImgurUploader::onUploadOk() {
    infoLabel->deleteLater();

    notification = new NotificationWidget();
    vLayout->addWidget(notification);

    auto *imageLabel = new ImageLabel();
    imageLabel->setImage(pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(imageLabel, &ImageLabel::dragInitiated, this, &ImgurUploader::startDrag);
    vLayout->addWidget(imageLabel);

    hLayout = new QHBoxLayout();
    vLayout->addLayout(hLayout);

    copyUrlButton = new QPushButton(tr("Copy URL"));
    openButton = new QPushButton(tr("Open URL"));
    deleteButton = new QPushButton(tr("Delete image"));
    toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    hLayout->addWidget(copyUrlButton);
    hLayout->addWidget(openButton);
    hLayout->addWidget(deleteButton);
    hLayout->addWidget(toClipboardButton);

    connect(copyUrlButton, &QPushButton::clicked, this, &ImgurUploader::copyURL);
    connect(openButton, &QPushButton::clicked, this, &ImgurUploader::openURL);
    connect(deleteButton, &QPushButton::clicked, this, &ImgurUploader::openDeleteURL);
    connect(toClipboardButton, &QPushButton::clicked, this, &ImgurUploader::copyImage);
}

void ImgurUploader::openURL() {
    bool successful = QDesktopServices::openUrl(imageURL);
    if (!successful) {
        notification->showMessage(tr("Unable to open the URL."));
    }
}

void ImgurUploader::copyURL() {
    QApplication::clipboard()->setText(imageURL.toString());
    notification->showMessage(tr("URL copied to clipboard."));
}

void ImgurUploader::openDeleteURL() {
    bool successful = QDesktopServices::openUrl(deleteImageURL);
    if (!successful) {
        notification->showMessage(tr("Unable to open the URL."));
    }
}

void ImgurUploader::copyImage() {
    QApplication::clipboard()->setPixmap(pixmap);
    notification->showMessage(tr("Screenshot copied to clipboard."));
}
