#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QtGlobal>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"

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

    void color();

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
        cv::Mat matNoAlpha;
        cv::cvtColor(mat, matNoAlpha, cv::COLOR_BGRA2BGR);   // drop the all-white alpha channel
        return matNoAlpha;
    }

private:
    void createActions();

    void updateActions();

    bool saveFile(const QString &fileName);

    void setImage(const cv::Mat &newMat);

    void scaleImage(double factor);

    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void wheelEvent(QWheelEvent *event) override;

    cv::Mat mat;
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
    QAction *colorAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
};

#endif

inline QImage cvMatToQImage(const cv::Mat& inMat) {
    switch (inMat.type()) {
        // 8-bit, 4 channel
        case CV_8UC4: {
            QImage image(inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_ARGB32);

            return image;
        }

            // 8-bit, 3 channel
        case CV_8UC3: {
            QImage image(inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_RGB888);

            return image.rgbSwapped();
        }

            // 8-bit, 1 channel
        case CV_8UC1: {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            QImage image(inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_Grayscale8);
#else
            static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if ( sColorTable.isEmpty() )
            {
               sColorTable.resize( 256 );

               for ( int i = 0; i < 256; ++i )
               {
                  sColorTable[i] = qRgb( i, i, i );
               }
            }

            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_Indexed8 );

            image.setColorTable( sColorTable );
#endif

            return image;
        }

        default:
            qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
    }

    return QImage();
}

inline QPixmap cvMatToQPixmap(const cv::Mat& inMat) {
    return QPixmap::fromImage(cvMatToQImage(inMat));
}
