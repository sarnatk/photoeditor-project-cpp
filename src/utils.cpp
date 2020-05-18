#include "utils.h"

NotificationWidget::NotificationWidget(QWidget *parent) : QWidget(parent) {
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(7000);
    connect(timer, &QTimer::timeout, this, &NotificationWidget::animatedHide);

    content = new QFrame();
    layout = new QVBoxLayout();
    label = new QLabel(content);
    label->hide();

    showAnimation = new QPropertyAnimation(content, "geometry", this);
    showAnimation->setDuration(300);

    hideAnimation = new QPropertyAnimation(content, "geometry", this);
    hideAnimation->setDuration(300);
    connect(hideAnimation, &QPropertyAnimation::finished, label, &QLabel::hide);

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    mainLayout->addWidget(content);
    layout->addWidget(label, 0, Qt::AlignHCenter);
    content->setLayout(layout);

    setFixedHeight(40);
}

void NotificationWidget::showMessage(const QString &msg) {
    label->setText(msg);
    label->show();
    animatedShow();
}

void NotificationWidget::animatedShow() {
    showAnimation->setStartValue(QRect(0, 0, width(), 0));
    showAnimation->setEndValue(QRect(0, 0, width(), height()));
    showAnimation->start();
    timer->start();
}

void NotificationWidget::animatedHide() {
    hideAnimation->setStartValue(QRect(0, 0, width(), height()));
    hideAnimation->setEndValue(QRect(0, 0, width(), 0));
    hideAnimation->start();
}

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent), pixmap(QPixmap()) {
    DSEffect = new QGraphicsDropShadowEffect(this);

    DSEffect->setBlurRadius(5);
    DSEffect->setOffset(0);
    DSEffect->setColor(QColor(Qt::black));

    setGraphicsEffect(DSEffect);
    setCursor(Qt::OpenHandCursor);
    setAlignment(Qt::AlignCenter);
    setMinimumSize(size());
}

void ImageLabel::setImage(const QPixmap &pm) {
    pixmap = pm;
    const QString tooltip = QStringLiteral("%1x%2 px").arg(pixmap.width()).arg(pixmap.height());
    setToolTip(tooltip);
    setScaledPixmap();
}

void ImageLabel::setScaledPixmap() {
    const qreal scale = qApp->devicePixelRatio();
    QPixmap scaledPixmap = pixmap.scaled(size() * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledPixmap.setDevicePixelRatio(scale);
    setPixmap(scaledPixmap);
}
