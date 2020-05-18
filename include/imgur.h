#ifndef PHOTOEDITOR_IMGUR_H
#define PHOTOEDITOR_IMGUR_H

#include "utils.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QUrl>

class NotificationWidget;

class ImgurUploader : public QWidget {
Q_OBJECT
public:
    explicit ImgurUploader(const QPixmap &capture, QWidget *parent = nullptr);

private slots:

    void handleReply(QNetworkReply *reply);

    void startDrag();

    void openURL();

    void copyURL();

    void openDeleteURL();

    void copyImage();

private:
    void upload();

    void onUploadOk();

private:
    QPixmap pixmap;
    QNetworkAccessManager *NetworkAM;

    QVBoxLayout *vLayout;
    QHBoxLayout *hLayout;
    QLabel *infoLabel;
    QPushButton *openButton;
    QPushButton *deleteButton;
    QPushButton *copyUrlButton;
    QPushButton *toClipboardButton;
    QUrl imageURL;
    QUrl deleteImageURL;
    NotificationWidget *notification;

};

#endif //PHOTOEDITOR_IMGUR_H
