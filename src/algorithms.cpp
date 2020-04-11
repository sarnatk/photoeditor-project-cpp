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

cv::Mat hsv_add_scalar(const cv::Mat& img, int h, int s = 0, int v = 0) {

    cv::Mat res;

    // convert to hsv
    cvtColor(img, res, COLOR_BGR2HSV);;

    //add scalar
    res += Scalar(h, s, v);

    // convert back to bgr
    cvtColor(res, res, COLOR_HSV2BGR);

    return res;
}

cv::Mat saturate(const cv::Mat& img, int value) {

    // add value to saturation in hsv color space
    return hsv_add_scalar(img, 0, value);
}

cv::Mat brighten(const cv::Mat& img, int value) {

    // add value to "value" in hsv color space
    return hsv_add_scalar(img, 0, 0, value);
}


cv::Mat hue(const cv::Mat& img, int value) {

    // add value to hue in hsv color space
    return hsv_add_scalar(img, value);
}


cv::Mat contrast(const cv::Mat& img, int value) {

    cv::Mat res;

    // get factor
    double factor = (double) (259 * (value + 255)) / (255 * (259 - value));

    // linear transformation
    // what it does here is basically:
    // C' = (C - 128) * (factor) + 128
    img.convertTo(res, CV_8UC1, (factor), 128 * (1 - factor));

    return res;
}


cv::Mat lighten(const cv::Mat& img, int value) {

    // simply add value to Lightness channel in
    // Lab color space
    return lab_add_scalar(img, value);
}


cv::Mat lab_add_scalar(const cv::Mat& img, int l, int a, int b) {

    cv::Mat res;

    // convert to Lab
    cvtColor(img, res, cv::COLOR_BGR2Lab);;

    // add scalar
    res += Scalar(l, a, b);

    // convert back to BGR
    cvtColor(res, res, cv::COLOR_Lab2BGR);

    return res;
}



cv::Mat gray(const cv::Mat& img) {

    cv::Mat bw;

    // black & white filter
    cvtColor(img, bw, cv::COLOR_RGB2GRAY);

    return bw;
}


cv::Mat pink(const cv::Mat& img) {

    cv::Mat pink;

    // convert to another color space: it gets pink
    cvtColor(img, pink, cv::COLOR_YCrCb2RGB);

    return pink;
}


cv::Mat blend(const cv::Mat& img1, const cv::Mat& img2, double alpha) {

    // get beta
    double beta = (1.0 - alpha);

    cv::Mat dst;

    // sum of images
    addWeighted(img1, alpha, img2, beta, 0.0, dst);

    return dst;
}


cv::Mat tint(const cv::Mat& img, int value) {

    // + green
    return img + Scalar(0, value, 0);
}


cv::Mat temperature(const cv::Mat& img, int value) {

    // - blue + red
    return img + Scalar(-value, 0, value);
}


cv::Mat blur(const cv::Mat& img, double value) {

    cv::Mat res;

    // basic gaussian blur with automatically made kernel
    GaussianBlur(img, res, Size(0, 0), value);

    return res;
}

cv::Mat sharpen(const cv::Mat& img, double value) {

    // get default blur of the image
    cv::Mat res = blur(img);

    // just subtract blur image with given factor
    cv::addWeighted(img, 1 + value, res, -value, 0, res);

    return res;
}


cv::Mat transform_perspective(const cv::Mat& input, Point2f outputQuad[4]) {

    cv::Mat res;

    // Input Quadilateral or Image plane coordinates
    Point2f inputQuad[4];

    // Lambda Matrix
    Mat lambda(2, 4, CV_32FC1);


    // Set the lambda matrix the same type and size as input
    lambda = Mat::zeros(input.rows, input.cols, input.type());

    // The 4 points that select quadilateral on the input , from top-left in clockwise order
    // These four pts are the sides of the rect box used as input
    inputQuad[0] = Point2f(0, 0);
    inputQuad[1] = Point2f(input.cols, 0);
    inputQuad[2] = Point2f(input.cols, input.rows);
    inputQuad[3] = Point2f(0, input.rows);

    // Get the Perspective Transform Matrix i.e. lambda
    lambda = getPerspectiveTransform(inputQuad, outputQuad);

    // Apply the Perspective Transform just found to the src image
    warpPerspective(input, res, lambda, res.size());

    return res;
}