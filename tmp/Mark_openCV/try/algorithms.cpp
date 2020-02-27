//
// Created by mark on 27.02.2020.
//

#include "algorithms.h"

cv::Mat crop(const cv::Mat &img, int w, int h, int x, int y) {
    return img(cv::Rect(x, y, w, h));
}


cv::Mat rotate_in_frame(const cv::Mat &img, int angle) {
    // get rotation matrix for rotating the image around its center in pixel coordinates
    cv::Point2f center((img.cols - 1) / 2.0, (img.rows - 1) / 2.0);
    cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);

    // determine bounding rectangle, center not relevant
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), img.size(), angle).boundingRect2f();

    // adjust transformation matrix
    rot.at<double>(0, 2) += bbox.width / 2.0 - img.cols / 2.0;
    rot.at<double>(1, 2) += bbox.height / 2.0 - img.rows / 2.0;

    cv::Mat dst;
    cv::warpAffine(img, dst, rot, bbox.size());
    return dst;
}

cv::Mat saturate(const cv::Mat &img, int saturation, double scale) {
    // what it does here is dst = (uchar) ((double)src*scale+saturation);
    cv::Mat saturated;

    img.convertTo(saturated, CV_8UC1, scale, saturation);

    return saturated;
}