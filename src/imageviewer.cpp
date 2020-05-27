#include "../include/imageviewer.h"
#include "algorithms.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardPaths>
#include <QStatusBar>
#include <QToolBar>
#include <QWheelEvent>

#if defined(QT_PRINTSUPPORT_LIB)

#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printdialog)

#    include <QPrintDialog>
#    include <QtWidgets/QStyleFactory>
#    include <QtWidgets/QGraphicsView>
#    include <QtWidgets/QInputDialog>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QSlider>
#include <QtCore/QTimeLine>

#  endif
#endif

ImageViewer::ImageViewer(QWidget *parent)
        : QMainWindow(parent), imageLabel(new QLabel), scrollArea(new QScrollArea) {
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    /// dark theme
    // set style
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // increase font size for better reading
    QFont defaultFont = QApplication::font();
    defaultFont.setPointSize(defaultFont.pointSize() + 2);
    qApp->setFont(defaultFont);

    // modify palette to dark
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

    qApp->setPalette(darkPalette);


    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
}

bool ImageViewer::loadFile(const QString &fileName) {
    QImageReader reader(fileName);
    reader.setAutoTransform(true);

    cv::Mat new_image = cv::imread(fileName.toStdString());
    if (new_image.empty()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                         .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    setWindowFilePath(fileName);

    controller.open_image(new_image);

    // remember and show image
    setImage(new_image);

    scaleFactor = 1.0;
    imageLabel->adjustSize();
    fitToWindow();

    return true;
}

void ImageViewer::setImage(const cv::Mat &new_image) {
    image = new_image;

    imageLabel->setPixmap(cvMatToQPixmap(image));
    scaleImage(1);

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    zoomInAct->setEnabled(true);
    zoomOutAct->setEnabled(true);
    normalSizeAct->setEnabled(true);

    updateActions();
}


bool ImageViewer::saveFile(const QString &fileName) {
    QImageWriter writer(fileName);

    if (!imwrite(fileName.toStdString(), image)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                         .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}


void ImageViewer::open() {
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QStringList list;
    for (auto &fmt : formats)
        list.append("*." + QString(fmt));
    auto filter = "Images (" + list.join(" ") + ")";


    QString path = QFileDialog::getOpenFileName(nullptr, "Pick an image file",
                                                nullptr, filter);
    if (path.isEmpty())
        return;
    loadFile(path);
}

void ImageViewer::saveAs() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File As"),
                                                    QString(),
                                                    tr("Images (*.png *jpg *jpeg *bmp)"));

    while (!saveFile(fileName)) {}
}

void ImageViewer::print() {
    Q_ASSERT(imageLabel->pixmap());
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

void ImageViewer::copy() {
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(cvMatToQImage(image));
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD

static QImage clipboardImage() {
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const auto image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}

#endif // !QT_NO_CLIPBOARD

void ImageViewer::paste() {
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
        setImage(image);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
                .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}


void ImageViewer::zoomIn() {
    scaleImage(1.25);
}

void ImageViewer::zoomOut() {
    scaleImage(0.8);
}

void ImageViewer::normalSize() {
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow() {
    QSize size = scrollArea->size();
    QSize label_size = imageLabel->size();

    // get factor to scale for
    double factor = std::min((double) (size.width() - 3) / label_size.width(),
                             (double) (size.height() - 3) / label_size.height());

    scaleImage(factor);
}

void ImageViewer::about() {
    QMessageBox::about(this, tr("About Photo Editor"),
                       tr("<p>The <b>Photo Editor</b> is a brilliant qt "
                          "and opencv applicatoin to edit an image."
                          "</p><p>Enjoy!</p>"));
}

QToolBar *ImageViewer::createToolBar() {
    auto *tools = new QToolBar("Linker ToolBar");
    tools->setIconSize(QSize(40, 40));

    toolUndoAct = tools->addAction(QPixmap("icons/undo.png"), tr("Undo"), this, &ImageViewer::undo);
    toolUndoAct->setEnabled(false);

    toolRedoAct = tools->addAction(QPixmap("icons/redo.png"), tr("Redo"), this, &ImageViewer::redo);
    toolRedoAct->setEnabled(false);

    tools->addSeparator();
    tools->addSeparator();
    tools->addSeparator();


    toolRotateAct = tools->addAction(QPixmap("icons/rotate.png"), tr("Rotate"), this, &ImageViewer::rotate);
    toolRotateAct->setEnabled(false);

    toolLightenAct = tools->addAction(QPixmap("icons/light.png"), tr("Light"), this, &ImageViewer::applyLight);
    toolLightenAct->setEnabled(false);

    toolTemperatureAct = tools->addAction(QPixmap("icons/temperature.png"), tr("Temperature"), this, &ImageViewer::applyTemperature);
    toolTemperatureAct->setEnabled(false);

    tools->addSeparator();
    tools->addSeparator();
    tools->addSeparator();

    toolColorAct = tools->addAction(QPixmap("icons/palette.png"), tr("Color"), this, &ImageViewer::color);
    toolColorAct->setEnabled(false);

    toolHueAct = tools->addAction(QPixmap("icons/rainbow.png"), tr("Hue"), this, &ImageViewer::applyHue);
    toolHueAct->setEnabled(false);

    tools->addSeparator();
    tools->addSeparator();
    tools->addSeparator();

    toolSharpenAct = tools->addAction(QPixmap("icons/clear.png"), tr("Sharp"), this, &ImageViewer::applySharp);
    toolSharpenAct->setEnabled(false);

    toolBlurAct = tools->addAction(QPixmap("icons/blur.png"), tr("Blur"), this, &ImageViewer::applyBlur);
    toolBlurAct->setEnabled(false);

    toolSaturateAct = tools->addAction(QPixmap("icons/stars.png"), tr("Saturation"), this, &ImageViewer::applySaturation);
    toolSaturateAct->setEnabled(false);

    return tools;
}

void ImageViewer::createActions() {
    addToolBar(Qt::LeftToolBarArea, createToolBar());

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    undoAct = fileMenu->addAction(tr("&Undo"), this, &ImageViewer::undo);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setEnabled(false);

    redoAct = fileMenu->addAction(tr("&Redo"), this, &ImageViewer::redo);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setEnabled(false);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::saveAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    uploadToImgurAct = fileMenu->addAction(tr("Upload to &Imgur..."), this, &ImageViewer::uploadToImgur);
    uploadToImgurAct->setShortcut(tr("Ctrl+I"));
    uploadToImgurAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    rotateAct = editMenu->addAction(tr("&Rotate"), this, &ImageViewer::rotate);
    rotateAct->setShortcut(tr("Ctrl+R"));
    rotateAct->setEnabled(false);

    tintAct = editMenu->addAction(tr("Tint"), this, &ImageViewer::applyTint);
    tintAct->setEnabled(false);

    brightenAct = editMenu->addAction(tr("Brightness"), this, &ImageViewer::applyBright);
    brightenAct->setEnabled(false);

    contrastAct = editMenu->addAction(tr("Contrast"), this, &ImageViewer::applyContrast);
    contrastAct->setEnabled(false);

    saturationAct = editMenu->addAction(tr("Saturation"), this, &ImageViewer::applySaturation);
    saturationAct->setEnabled(false);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(tr("Ctrl+="));
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+N"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::updateActions() {
    saveAsAct->setEnabled(!image.empty());
    copyAct->setEnabled(!image.empty());
    uploadToImgurAct->setEnabled(!image.empty());
    rotateAct->setEnabled(!image.empty());
    tintAct->setEnabled(!image.empty());
    brightenAct->setEnabled(!image.empty());
    saturationAct->setEnabled(!image.empty());
    contrastAct->setEnabled(!image.empty());
    undoAct->setEnabled(controller.can_undo());
    redoAct->setEnabled(controller.can_redo());
    toolUndoAct->setEnabled(controller.can_undo());
    toolRedoAct->setEnabled(controller.can_redo());
    toolRotateAct->setEnabled(!image.empty());
    toolColorAct->setEnabled(!image.empty());
    toolLightenAct->setEnabled(!image.empty());
    toolTemperatureAct->setEnabled(!image.empty());
    toolHueAct->setEnabled(!image.empty());
    toolSharpenAct->setEnabled(!image.empty());
    toolBlurAct->setEnabled(!image.empty());
    toolSaturateAct->setEnabled(!image.empty());
}

void ImageViewer::scaleImage(double factor) {
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor) {
    scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep() / 2)));
}

void ImageViewer::animFinished() {
    if (_numScheduledScalings > 0) _numScheduledScalings--;
    else sender()->~QObject();
}

void ImageViewer::scalingTime(double x) {
    scaleImage(tmpFactor);
}

void ImageViewer::wheelEvent(QWheelEvent *event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        _numScheduledScalings = 300;
        if (event->delta() > 0) tmpFactor = pow(2.5, 1 / 300.0);
        else tmpFactor = pow(2.5, -1 / 300.0);

        auto *anim = new QTimeLine(350, this);
        anim->setUpdateInterval(20);

        connect(anim, SIGNAL(valueChanged(qreal)), SLOT(scalingTime(qreal)));
        connect(anim, SIGNAL(finished()), SLOT(animFinished()));
        anim->start();
    }
}

void ImageViewer::undo() {
    setImage(controller.undo());
}

void ImageViewer::redo() {
    setImage(controller.redo());
}

void ImageViewer::applyBright() {
    int ratio = QInputDialog::getInt(this, tr("Brightness"), tr("Rate:"), 0, -256, 256);
    setImage(controller.brighten(image, ratio));
}

void ImageViewer::cancel() {
    setImage(oldImage);
    window->close();
}

void ImageViewer::saturate(int ratio) {
    auto im = image;
    setImage(controller.saturate(im, ratio));
}

void ImageViewer::applySaturation() {
    oldImage = image;
    auto *slider = new QSlider(Qt::Horizontal);
    auto *applyButton= new QPushButton("Apply");
    auto *cancelButton= new QPushButton("Cancel");
    auto *layout = new QVBoxLayout;
    window = new QDialog;
    slider->setMinimum(-3);
    slider->setMaximum(30);
    layout->addWidget(slider);
    layout->addWidget(applyButton);
    layout->addWidget(cancelButton);
    window->setWindowTitle(tr("Saturation"));
    window->setLayout(layout);
    window->show();
    connect(applyButton, SIGNAL(clicked()), window, SLOT(close()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(slider, &QSlider::valueChanged, this, &ImageViewer::saturate);
}

void ImageViewer::lighten(int ratio) {
    auto im = image;
    ratio = ratio > 0 ? 3 : -5;
    setImage(controller.lighten(im, ratio));
}

void ImageViewer::applyLight() {
    oldImage = image;
    auto *slider = new QSlider(Qt::Horizontal);
    auto *applyButton= new QPushButton("Apply");
    auto *cancelButton= new QPushButton("Cancel");
    auto *layout = new QVBoxLayout;
    window = new QDialog;
    slider->setMinimum(-20);
    slider->setMaximum(20);
    layout->addWidget(slider);
    layout->addWidget(applyButton);
    layout->addWidget(cancelButton);
    window->setWindowTitle(tr("Light"));
    window->setLayout(layout);
    window->show();
    connect(applyButton, SIGNAL(clicked()), window, SLOT(close()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(slider, &QSlider::valueChanged, this, &ImageViewer::lighten);
}

void ImageViewer::hue(int ratio) {
    auto im = image;
    setImage(controller.hue(im, ratio));
}

void ImageViewer::applyHue() {
    oldImage = image;
    auto *slider = new QSlider(Qt::Horizontal);
    auto *applyButton= new QPushButton("Apply");
    auto *cancelButton= new QPushButton("Cancel");
    auto *layout = new QVBoxLayout;
    window = new QDialog;
    slider->setMinimum(0);
    slider->setMaximum(50);
    layout->addWidget(slider);
    layout->addWidget(applyButton);
    layout->addWidget(cancelButton);
    window->setWindowTitle(tr("Hue"));
    window->setLayout(layout);
    window->show();
    connect(applyButton, SIGNAL(clicked()), window, SLOT(close()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(slider, &QSlider::valueChanged, this, &ImageViewer::hue);
}

void ImageViewer::applyContrast() {
    int ratio = QInputDialog::getInt(this, tr("Contrast"), tr("Rate:"), 0, -256, 256);
    setImage(controller.contrast(image, ratio));
}


void ImageViewer::rotate() {
    double angle = QInputDialog::getDouble(this, tr("Rotate"), tr("Angle:"), 0, 0, 360, 0);
    setImage(controller.rotate_in_frame(image, angle));
}

void ImageViewer::color() {
    QStringList items;
    items << tr("Black and White") << tr("Colors");
    QString item = QInputDialog::getItem(this, tr("Filter"), tr("Color:"), items, 0, false);


    if (item == "Black and White") setImage(controller.gray(image));
    else if (item == "Colors") {
        QColor color = QColorDialog::getColor(Qt::magenta, this);
        double alpha = QInputDialog::getDouble(this, tr("Colors"), tr("Intensity"), 0, 0, 1, 3);
        int r, g, b;
        color.getRgb(&r, &g, &b);

        setImage(controller.apply_color(image, r, g, b, alpha));
    }

}

void ImageViewer::applyTint() {
    int ratio = QInputDialog::getInt(this, tr("Tint"), tr("Rate:"), 0, -256, 256);
    setImage(controller.tint(image, ratio));
}


void ImageViewer::temperature(int ratio) {
    auto im = image;
    ratio = ratio > 0 ? 2 : -2;
    setImage(controller.temperature(im, ratio));
}

void ImageViewer::applyTemperature() {
    oldImage = image;
    auto *slider = new QSlider(Qt::Horizontal);
    auto *applyButton= new QPushButton("Apply");
    auto *cancelButton= new QPushButton("Cancel");
    auto *layout = new QVBoxLayout;
    window = new QDialog;
    slider->setMinimum(-25);
    slider->setMaximum(25);
    layout->addWidget(slider);
    layout->addWidget(applyButton);
    layout->addWidget(cancelButton);
    window->setWindowTitle(tr("Temperature"));
    window->setLayout(layout);
    window->show();
    connect(applyButton, SIGNAL(clicked()), window, SLOT(close()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(slider, &QSlider::valueChanged, this, &ImageViewer::temperature);
}

void ImageViewer::blur(int ratio) {
    auto im = image;
    double degree = (double) ratio / -10;
    setImage(controller.sharpen(im, degree));
}

void ImageViewer::applyBlur() {
    oldImage = image;
    auto *sliderBlur = new QSlider(Qt::Horizontal);
    auto *applyButton= new QPushButton("Apply");
    auto *cancelButton= new QPushButton("Cancel");
    auto *layout = new QVBoxLayout;
    window = new QDialog;
    sliderBlur->setMinimum(0);
    sliderBlur->setMaximum(20);
    layout->addWidget(sliderBlur);
    layout->addWidget(applyButton);
    layout->addWidget(cancelButton);
    window->setWindowTitle(tr("Blur"));
    window->setLayout(layout);
    window->show();
    connect(applyButton, SIGNAL(clicked()), window, SLOT(close()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(sliderBlur, &QSlider::valueChanged, this, &ImageViewer::blur);
}

void ImageViewer::sharp(int ratio) {
    auto im = image;
    double degree = (double) ratio / 10;
    setImage(controller.sharpen(im, degree));
}

void ImageViewer::applySharp() {
    oldImage = image;
    auto *sliderSharpness = new QSlider(Qt::Horizontal);
    auto *applyButton= new QPushButton("Apply");
    auto *cancelButton= new QPushButton("Cancel");
    auto *layout = new QVBoxLayout;
    window = new QDialog;
    sliderSharpness->setMinimum(0);
    sliderSharpness->setMaximum(20);
    layout->addWidget(sliderSharpness);
    layout->addWidget(applyButton);
    layout->addWidget(cancelButton);
    window->setWindowTitle(tr("Sharpness"));
    window->setLayout(layout);
    window->show();
    connect(applyButton, SIGNAL(clicked()), window, SLOT(close()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(sliderSharpness, &QSlider::valueChanged, this, &ImageViewer::sharp);
}

void ImageViewer::uploadToImgur() {
    QPixmap pixmap = cvMatToQPixmap(image);
    ImgurUploader imgurUploader(pixmap);
}