#ifndef PHOTOEDITOR_UTILS_H
#define PHOTOEDITOR_UTILS_H

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
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

#endif //PHOTOEDITOR_UTILS_H
