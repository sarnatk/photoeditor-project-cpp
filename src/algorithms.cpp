//
// Created by mark on 27.02.2020.
//

#include "../include/algorithms.h"

namespace image_algorithms {
    using namespace cv;

    cv::Mat takePicture(cv::VideoCapture camera) {

        cv::Mat image;

        while (!camera.isOpened()) {
            std::cout << "Failed to make connection to camera" << std::endl;
            camera.open(0);
        }

        camera >> image;
        flip(image, image, 1);

        return image;
    }

    cv::Mat crop(const cv::Mat& image, int w, int h, int x, int y) {
        return image(cv::Rect(x, y, w, h));
    }

    cv::Mat gray(const cv::Mat& image) {
        cv::Mat bw;

        // black & white filter
        cvtColor(image, bw, cv::COLOR_BGR2GRAY);
        cvtColor(bw, bw, cv::COLOR_GRAY2BGR);

        return bw;
    }

    cv::Mat rotate_in_frame(const cv::Mat& image, double angle) {

        // get rotation matrix for rotating the image around its center in pixel coordinates
        cv::Point2f center((image.cols - 1) / 2.0, (image.rows - 1) / 2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);

        // determine bounding rectangle, center not relevant
        cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), image.size(), angle).boundingRect2f();

        // adjust transformation matrix
        rot.at<double>(0, 2) += bbox.width / 2.0 - image.cols / 2.0;
        rot.at<double>(1, 2) += bbox.height / 2.0 - image.rows / 2.0;

        cv::Mat dst;
        cv::warpAffine(image, dst, rot, bbox.size());

        return dst;
    }

    cv::Mat hsv_add_scalar(const cv::Mat& image, int h, int s, int v) {

        cv::Mat res;

        // convert to hsv
        cvtColor(image, res, COLOR_BGR2HSV);;

        //add scalar
        res += Scalar(h, s, v);

        // convert back to bgr
        cvtColor(res, res, COLOR_HSV2BGR);

        return res;
    }

    // value in [-100, 100]
    cv::Mat saturate(const cv::Mat& image, int value) {
        value >= 0 ? value += 50 : value -= 50;
        cv::Mat tmp, res;
        addWeighted(image, value / 50.0, gray(image), 1 - value / 50.0, 0.0, res);
        return res;
    }

    cv::Mat brighten(const cv::Mat& image, int value) {

        // add value to "value" in hsv color space
        return hsv_add_scalar(image, 0, 0, value);
    }


    cv::Mat hue(const cv::Mat& image, int value) {

        // add value to hue in hsv color space
        return hsv_add_scalar(image, value);
    }


    cv::Mat contrast(const cv::Mat& image, int value) {

        cv::Mat res;

        // get factor
        double factor = (double) (259 * (value + 255)) / (255 * (259 - value));

        // linear transformation
        // what it does here is basically:
        // C' = (C - 128) * (factor) + 128
        image.convertTo(res, CV_8UC1, (factor), 128 * (1 - factor));

        return res;
    }


    cv::Mat lighten(const cv::Mat& image, int value) {

        // simply add value to Lightness channel in
        // Lab color space
        return lab_add_scalar(image, value);
    }


    cv::Mat lab_add_scalar(const cv::Mat& image, int l, int a, int b) {

        cv::Mat res;

        // convert to Lab
        cvtColor(image, res, cv::COLOR_BGR2Lab);;

        // add scalar
        res += Scalar(l, a, b);

        // convert back to BGR
        cvtColor(res, res, cv::COLOR_Lab2BGR);

        return res;
    }


    cv::Mat blend(const cv::Mat& img1, const cv::Mat& img2, double alpha) {

        // get beta
        double beta = (1.0 - alpha);

        cv::Mat dst;

        // sum of images
        addWeighted(img1, alpha, img2, beta, 0.0, dst);

        return dst;
    }


    cv::Mat tint(const cv::Mat& image, int value) {

        // + green
        return image + Scalar(0, value, 0);
    }


    cv::Mat temperature(const cv::Mat& image, int value) {

        // - blue + red
        return image + Scalar(-value, 0, value);
    }


    cv::Mat blur(const cv::Mat& image, double value) {

        cv::Mat res;

        // basic gaussian blur with automatically made kernel
        GaussianBlur(image, res, Size(0, 0), value);

        return res;
    }

    cv::Mat sharpen(const cv::Mat& image, double value) {

        // get default blur of the image
        cv::Mat res = blur(image, 3);

        // just subtract blur image with given factor
        cv::addWeighted(image, 1 + value, res, -value, 0, res);

        return res;
    }


    cv::Mat transform_perspective(const cv::Mat& input, cv::Point2f outputQuad[4]) {

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

    cv::Mat apply_color(const cv::Mat& mat, int r, int g, int b, double alpha) {
        cv::Mat m(mat.rows, mat.cols, CV_8UC3, cv::Scalar(b, g, r));
        return blend(mat, m, 1 - alpha);
    }


    Crop::Crop(int width, int height, int x, int y) : w{width}, h{height}, x{x},
                                                      y{y} {}

    cv::Mat Crop::execute(const cv::Mat& image) const {
        return crop(image, w, h, x, y);
    }


    RotateInFrame::RotateInFrame(double angle) : angle{angle} {}

    cv::Mat RotateInFrame::execute(const cv::Mat& base_image) const {
        return rotate_in_frame(base_image, angle);
    }

    Saturate::Saturate(int value) : value{value} {
    }

    cv::Mat Saturate::execute(const cv::Mat& image) const {
        return saturate(image, value);
    }


    Brighten::Brighten(int value) : value{value} {
    }

    cv::Mat Brighten::execute(const Mat& image) const {
        return brighten(image, value);
    }

    Lighten::Lighten(int value) : value{value} {
    }

    cv::Mat Lighten::execute(const Mat& image) const {
        return lighten(image, value);
    }

    Hue::Hue(int value) : value{value} {
    }

    cv::Mat Hue::execute(const Mat& image) const {
        return hue(image, value);
    }


    Contrast::Contrast(int value) : value{value} {
    }

    cv::Mat Contrast::execute(const Mat& image) const {
        return contrast(image, value);
    }

    cv::Mat Gray::execute(const Mat& image) const {
        return gray(image);
    }

    Blend::Blend(const Mat& image_2, double alpha) : image_2{image_2}, value{alpha} {
    }

    cv::Mat Blend::execute(const Mat& image_1) const {
        return blend(image_1, image_2, value);
    }


    Tint::Tint(int value) : value{value} {
    }

    cv::Mat Tint::execute(const Mat& image) const {
        return tint(image, value);
    }


    Temperature::Temperature(int value) : value{value} {
    }

    cv::Mat Temperature::execute(const Mat& image) const {
        return temperature(image, value);
    }


    Blur::Blur(double value) : value{value} {
    }

    cv::Mat Blur::execute(const Mat& image) const {
        return blur(image, value);
    }

    Sharpen::Sharpen(double value) : value{value} {
    }

    cv::Mat Sharpen::execute(const Mat& image) const {
        return sharpen(image, value);
    }

    ApplyColor::ApplyColor(int r, int g, int b, double alpha) : r{r}, g{g}, b{b}, alpha{alpha} {
    }

    cv::Mat ApplyColor::execute(const Mat& image) const {
        return apply_color(image, r, g, b, alpha);
    }

    TransformPerspective::TransformPerspective(cv::Point2f* outputQuad) : outputQuad{outputQuad} {
    }

    cv::Mat TransformPerspective::execute(const Mat& image) const {
        return transform_perspective(image, outputQuad);
    }

    cv::Mat Nothing::execute(const Mat& image) const {
        return image;
    }
}