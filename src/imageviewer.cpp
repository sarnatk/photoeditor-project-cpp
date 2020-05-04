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

#  endif
#endif

ImageViewer::ImageViewer(QWidget* parent)
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


bool ImageViewer::loadFile(const QString& fileName) {
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

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
            .arg(QDir::toNativeSeparators(fileName)).arg(image.cols).arg(image.rows).arg(image.depth());
    statusBar()->showMessage(message);

    controller.open_image(new_image);
    // remember and show image
    setImage(new_image);

    return true;
}

void ImageViewer::setImage(const cv::Mat& new_image) {
    image = new_image;

    imageLabel->setPixmap(cvMatToQPixmap(image));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    zoomInAct->setEnabled(true);
    zoomOutAct->setEnabled(true);
    normalSizeAct->setEnabled(true);

    updateActions();

    imageLabel->adjustSize();
    fitToWindow();
}


bool ImageViewer::saveFile(const QString& fileName) {
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
    for (auto& fmt : formats)
        list.append("*." + QString(fmt));
    auto filter = "Images (" + list.join(" ") + ")";


    QString path = QFileDialog::getOpenFileName(nullptr, "Pick an image file",
                                                nullptr, filter);
    if (path.isEmpty())
        return;
    loadFile(path);

}

void ImageViewer::save_as() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File As"),
                                                    QString(),
                                                    tr("Images (*.png *jpg *jpeg *bmp)"));

    while (!saveFile(fileName)) {}
}

void ImageViewer::upload() {
    QImage q_image = cvMatToQImage(image); //mKImageAnnotator->image(); ??????????WHYYYYYY WHY WHYYYY D:
    UploadOperation operation(q_image, mCaptureUploader);
    operation.execute();
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
    if (const QMimeData* mimeData = QGuiApplication::clipboard()->mimeData()) {
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
    double scale_factor = std::min((double) size.width() / label_size.width(),
                                   (double) (size.height() - 35) / label_size.height());

    scaleImage(scale_factor);
}

void ImageViewer::about() {
    QMessageBox::about(this, tr("About Photo Editor"),
                       tr("<p>The <b>Photo Editor</b> is a brilliant qt "
                          "and opencv applicatoin to edit an image."
                          "</p><p>Enjoy!</p>"));
}

void ImageViewer::createActions() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    undoAct = fileMenu->addAction(tr("&Undo"), this, &ImageViewer::undo);
    redoAct = fileMenu->addAction(tr("&Redo"), this, &ImageViewer::redo);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::save_as);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    uploadToImgurAct = fileMenu->addAction(tr("Upload to &Imgur..."), this, &ImageViewer::upload);
    uploadToImgurAct->setShortcut(tr("Ctrl+I"));
    uploadToImgurAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction* exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction* pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    rotateAct = editMenu->addAction(tr("&Rotate"), this, &ImageViewer::rotate);
    rotateAct->setShortcut(tr("Ctrl+R"));
    rotateAct->setEnabled(false);

    colorAct = editMenu->addAction(tr("Color"), this, &ImageViewer::color);
    colorAct->setEnabled(false);


    tintAct = editMenu->addAction(tr("Tint"), this, &ImageViewer::applyTint);
    tintAct->setEnabled(false);

    brightenAct = editMenu->addAction(tr("Brightness"), this, &ImageViewer::applyBright);
    brightenAct->setEnabled(false);

    lightenAct = editMenu->addAction(tr("Lightness"), this, &ImageViewer::applyLight);
    lightenAct->setEnabled(false);

    contrastAct = editMenu->addAction(tr("Contrast"), this, &ImageViewer::applyContrast);
    contrastAct->setEnabled(false);

    saturationAct = editMenu->addAction(tr("Saturation"), this, &ImageViewer::applySaturation);
    saturationAct->setEnabled(false);

    hueAct = editMenu->addAction(tr("Hue"), this, &ImageViewer::applyHue);
    hueAct->setEnabled(false);


    temperatureAct = editMenu->addAction(tr("Temperature"), this, &ImageViewer::applyTemperature);
    temperatureAct->setEnabled(false);

    sharpenAct = editMenu->addAction(tr("Sharpen"), this, &ImageViewer::applySharp);
    sharpenAct->setEnabled(false);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

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

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::updateActions() {
    saveAsAct->setEnabled(!image.empty());
    copyAct->setEnabled(!image.empty());
    uploadToImgurAct->setEnabled(!image.empty());
    rotateAct->setEnabled(!image.empty());
    colorAct->setEnabled(!image.empty());
    tintAct->setEnabled(!image.empty());
    temperatureAct->setEnabled(!image.empty());
    sharpenAct->setEnabled(!image.empty());
    brightenAct->setEnabled(!image.empty());
    lightenAct->setEnabled(!image.empty());
    hueAct->setEnabled(!image.empty());
    saturationAct->setEnabled(!image.empty());
    contrastAct->setEnabled(!image.empty());
    undoAct->setEnabled(controller.can_undo());
    redoAct->setEnabled(controller.can_redo());
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

void ImageViewer::adjustScrollBar(QScrollBar* scrollBar, double factor) {
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep() / 2)));
}

void ImageViewer::wheelEvent(QWheelEvent* event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        if (event->delta() > 0) {
            scaleImage(1.05);
        } else if (event->delta() < 0) {
            scaleImage(0.952381);
        }
        event->accept();
    } else QWidget::wheelEvent(event);
}

void ImageViewer::undo() {
    setImage(controller.undo());
}

void ImageViewer::redo() {
    setImage(controller.redo());
}

void ImageViewer::applySaturation() {
    int ratio = QInputDialog::getInt(this, tr("Saturation"), tr("Rate:"),
                                     0, -256, 100);

    setImage(controller.saturate(image, ratio));
}

void ImageViewer::applyBright() {
    int ratio = QInputDialog::getInt(this, tr("Brightness"), tr("Rate:"), 0, -256, 256);

    setImage(controller.brighten(image, ratio));
}

void ImageViewer::applyLight() {
    int ratio = QInputDialog::getInt(this, tr("Lightness"), tr("Rate:"), 0, -256, 256);

    setImage(controller.lighten(image, ratio));
}

void ImageViewer::applyHue() {
    int ratio = QInputDialog::getInt(this, tr("Hue"), tr("Rate:"), 0, -256, 256);

    setImage(controller.hue(image, ratio));
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

void ImageViewer::applyTemperature() {
    int degree = QInputDialog::getInt(this, tr("Temperature"), tr("Degree:"), 0, -256, 256);

    setImage(controller.temperature(image, degree));
}


void ImageViewer::applySharp() {
    double degree = QInputDialog::getDouble(this, tr("Sharpening"), tr("Degree:"), 0, -2, 2, 5);

    setImage(controller.sharpen(image, degree));
}