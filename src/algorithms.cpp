//
// Created by mark on 27.02.2020.
//

#include "../include/algorithms.h"


using namespace cv;

cv::Mat takePicture(cv::VideoCapture cam) {
    cv::Mat pic;

    while (!cam.isOpened()) {
        std::cout << "Failed to make connection to cam" << std::endl;
        cam.open(0);
    }
    cam >> pic;
    flip(pic, pic, 1);
    return pic;
}

cv::Mat crop(const cv::Mat& img, int w, int h, int x, int y) {
    return img(cv::Rect(x, y, w, h));
}


cv::Mat rotate_in_frame(const cv::Mat& img, double angle) {
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

cv::Mat saturate(const cv::Mat& img, int saturation, double scale) {
    // what it does here is dst = (uchar) ((double)src*scale+saturation);
    cv::Mat saturated;

//    img.convertTo(saturated, CV_8UC1, scale, saturation);

    img.convertTo(saturated, CV_8UC1, scale, saturation);


    return saturated;
}

cv::Mat brighten(const cv::Mat& img, int brightness, int contrast) {
    cv::Mat saturated;

    // *(contrast/127 + 1) - contrast + brightness
    img.convertTo(saturated, CV_8UC1, ((double) contrast / 127 + 1), brightness);


    return saturated;
}


cv::Mat gray(const cv::Mat &img) {
    cv::Mat bw;
    cvtColor(img, bw, cv::COLOR_RGB2GRAY);
    return bw;
}


cv::Mat pink(const cv::Mat &img) {
    cv::Mat pink;
    cvtColor(img, pink,  cv::COLOR_YCrCb2RGB);
    return pink;
}


Mat Sharpen(const Mat& myImage) {
    CV_Assert(myImage.depth() == CV_8U);  // accept only uchar images
    Mat Result;

    Result.create(myImage.size(), myImage.type());
    const int nChannels = myImage.channels();

    for (int j = 1; j < myImage.rows - 1; ++j) {
        const auto* previous = myImage.ptr<uchar>(j - 1);
        const auto* current = myImage.ptr<uchar>(j);
        const auto* next = myImage.ptr<uchar>(j + 1);

        auto* output = Result.ptr<uchar>(j);

        for (int i = nChannels; i < nChannels * (myImage.cols - 1); ++i) {
            *output++ = saturate_cast<uchar>(5 * current[i]
                                             - current[i - nChannels] - current[i + nChannels] - previous[i] - next[i]);
        }
    }

    Result.row(0).setTo(Scalar(0));
    Result.row(Result.rows - 1).setTo(Scalar(0));
    Result.col(0).setTo(Scalar(0));
    Result.col(Result.cols - 1).setTo(Scalar(0));

    return Result;
}


cv::Mat blend(const cv::Mat &img1, const cv::Mat &img2, double alpha) {
    double beta = (1.0 - alpha);
    cv::Mat dst;
    addWeighted(img1, alpha, img2, beta, 0.0, dst);
    return dst;
}
