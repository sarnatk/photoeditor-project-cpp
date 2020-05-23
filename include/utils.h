#ifndef PHOTOEDITOR_UTILS_H
#define PHOTOEDITOR_UTILS_H

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QPropertyAnimation>
#include <QStyleHints>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

enum class MessageBoxResponse {
    Yes, No, Cancel
};

class NotificationWidget : public QWidget {
Q_OBJECT
public:
    explicit NotificationWidget(QWidget *parent = nullptr);

    void showMessage(const QString &msg);

private:

    void animatedShow();

    void animatedHide();

private:
    QLabel *label;
    QPropertyAnimation *showAnimation;
    QPropertyAnimation *hideAnimation;
    QVBoxLayout *layout;
    QFrame *content;
    QTimer *timer;
};

class ImageLabel : public QLabel {
Q_OBJECT

public:
    explicit ImageLabel(QWidget *parent = nullptr);

    void setImage(const QPixmap &pixmap);

signals:

    void dragInitiated();

private:
    void setScaledPixmap();

    QGraphicsDropShadowEffect *DSEffect;
    QPixmap pixmap;
};

class MessageBoxHelper
{
public:
    static bool yesNo(const QString &title, const QString &question);
    static MessageBoxResponse yesNoCancel(const QString &title, const QString &question);
    static void ok(const QString &title, const QString &info);

private:
    static MessageBoxResponse mapReplyToMessageBoxResponse(int reply);
};

#endif //PHOTOEDITOR_UTILS_H
