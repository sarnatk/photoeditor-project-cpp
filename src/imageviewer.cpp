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


bool ImageViewer::loadFile(const QString& fileName) {
    QImageReader reader(fileName);
    reader.setAutoTransform(true);

    cv::Mat newMat = cv::imread(fileName.toStdString());
    if (newMat.empty()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                         .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    setImage(newMat);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
            .arg(QDir::toNativeSeparators(fileName)).arg(mat.cols).arg(mat.rows).arg(mat.depth());
    statusBar()->showMessage(message);

    return true;
}

void ImageViewer::setImage(const cv::Mat& newMat) {
    mat = newMat;

    imageLabel->setPixmap(cvMatToQPixmap(mat));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);

    fitToWindow();

    updateActions();
}


bool ImageViewer::saveFile(const QString& fileName) {
    std::cerr << "\n\nin SaveFile\n\n";

    QImageWriter writer(fileName);


    if (!imwrite(fileName.toStdString(), mat)) {
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
    QGuiApplication::clipboard()->setImage(cvMatToQImage(mat));
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
        setImage(mat);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
                .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::rotate() {
    double angle = QInputDialog::getDouble(this, tr("Rotate"), tr("Angle:"), 0, 0, 360, 0);

    setImage(rotate_in_frame(mat, angle));
}

void ImageViewer::color() {
    QStringList items;
    items << tr("Pink") << tr("Black and White");
    QString item = QInputDialog::getItem(this, tr("Filter"), tr("Color:"), items, 0, false);

/*
 * qcolor dialog : TODO ?
 *
 *  QColor color = QColorDialog::getColor(Qt::yellow, this);
 *  int r, g, b;
 *  color.getRgb(&r, &g, &b);
*/

    cv::Mat colored_mat;
    if (item == "Pink") colored_mat = pink(mat);
    if (item == "Black and White") colored_mat = gray(mat);

    setImage(colored_mat);
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
    QSize size = this->size();
    QSize label_size = imageLabel->size();
    std::cerr << "\n\n" << size.width() << " "  << label_size.width() << " " << size.height() << " " << label_size.height() << "\n\n";
    double scale_factor = std::min((double) size.width() / label_size.width(), (double) size.height() / label_size.height());
    scaleImage(scale_factor);
}

void ImageViewer::about() {
    QMessageBox::about(this, tr("About Photo Editor"),
                       tr("<p>The <b>Photo Editor</b> is a brilliant qt "
                          "and opencv applicatoin to edit an image."
                          "</p><p>Enjoy!</p>"));
}

void ImageViewer::createActions() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::saveAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

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

    colorAct = editMenu->addAction(tr("Color"), this, &ImageViewer::color);
    colorAct->setEnabled(false);

    tintAct = editMenu->addAction(tr("Tint"), this, &ImageViewer::tint);
    tintAct->setEnabled(false);

//    temperatureAct = editMenu->addAction(tr("Tint"), this, &ImageViewer::temperature);
//    temperatureAct->setEnabled(false);

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
    saveAsAct->setEnabled(!mat.empty());
    copyAct->setEnabled(!mat.empty());
    rotateAct->setEnabled(!mat.empty());
    colorAct->setEnabled(!mat.empty());
    tintAct->setEnabled(!mat.empty());
    temperatureAct->setEnabled(!mat.empty());
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
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep() / 2)));
}

void ImageViewer::wheelEvent(QWheelEvent *event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        if (event->delta() > 0) {
            scaleImage(1.05);
        } else if (event->delta() < 0) {
            scaleImage(0.952381);
        }
        event->accept();
    } else QWidget::wheelEvent(event);


}

void ImageViewer::tint() {
    auto slider = new QSlider();
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(1);
    slider->setMinimum(-256);
    slider->setMaximum(256);
    slider->setValue(0);


}
