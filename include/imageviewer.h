#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>

#if defined(QT_PRINTSUPPORT_LIB)

#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printer)

#    include <QPrinter>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#  endif
#endif

QT_BEGIN_NAMESPACE
class QAction;

class QLabel;

class QMenu;

class QScrollArea;

class QScrollBar;

QT_END_NAMESPACE


class ImageViewer : public QMainWindow {
Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr);

    bool loadFile(const QString &);

private slots:

    void open();

    void saveAs();

    void print();

    void copy();

    void paste();

    void rotate();

    void zoomIn();

    void zoomOut();

    void normalSize();

    void fitToWindow();

    void about();

private:
    QImage to_QImage(cv::Mat const &src) {
        QImage dest((const uchar *) src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
        dest.bits();
        return dest;
    }

    // ПРОБЛЕМЫ ЗДЕСЬ (возможно дело в CV_8UC4, но может и нет)
    cv::Mat to_mat(QImage const &inImage) {
        cv::Mat mat(inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar *>(inImage.bits()),
                    static_cast<size_t>(inImage.bytesPerLine()));
        cv::Mat  matNoAlpha;
        cv::cvtColor( mat, matNoAlpha, cv::COLOR_BGRA2BGR );   // drop the all-white alpha channel
        return matNoAlpha;
    }

private:
    void createActions();

    void updateActions();

    bool saveFile(const QString &fileName);

    void setImage(const QImage &newImage);

    void scaleImage(double factor);

    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void wheelEvent(QWheelEvent *event) override;

    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1;

#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
#endif

    QAction *saveAsAct;
    QAction *printAct;
    QAction *copyAct;
    QAction *rotateAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
};

#endif