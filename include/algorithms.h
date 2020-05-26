//
// Created by mark on 27.02.2020.
//

#ifndef OPENCVTEST_ALGORITHMS_H
#define OPENCVTEST_ALGORITHMS_H

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace image_algorithms {
    /**
     * First argument -- src image
     *
     * Result -- return value
     */


    class Command {
    public:
        virtual cv::Mat execute(const cv::Mat& image) const = 0;
    };

    /**
     *
     * Take picture using camera
     *
     * You have to create
     * cv::VideoCapture variable
     *
     * Probably like this:
     * cv::VideoCapture camera(0);
     */
    cv::Mat takePicture(cv::VideoCapture camera);

    /**
     * Does nothing -- use to open image
     */
    class Nothing : public Command {
    public:
        cv::Mat execute(const cv::Mat& image) const override;
    };


    /**
     * Crops image
     * Result with width = w, height = h
     */
    class Crop : public Command {
    private:
        int w;
        int h;
        int x = 0;
        int y = 0;

    public:
        // width + x <= cols,
        // height + y <= rows
        Crop(int width, int height, int x = 0, int y = 0);

        cv::Mat execute(const cv::Mat& image) const override;

    };

    /**
     * Rotates image by given angle without crop
     */
    class RotateInFrame : public Command {
    private:
        double angle;

    public:

        RotateInFrame(double angle);

        cv::Mat execute(const cv::Mat& image) const override;

    };

    /**
     * Saturates image
     */
    class Saturate : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Saturate(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };

    /**
     * Brightens image
     */
    class Brighten : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Brighten(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


    /**
     * Lightens image (it actually is different from brighten)
     */
    class Lighten : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Lighten(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


    /**
     * Hue
     */
    class Hue : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Hue(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


    /**
     * Change contrast
     */
    class Contrast : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Contrast(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


    /**
     *  Add scalar in L*a*b color space
     */
    cv::Mat lab_add_scalar(const cv::Mat& image, int l, int a = 0, int b = 0);

    /**
     *  Add scalar in hsv color space
     */
    cv::Mat hsv_add_scalar(const cv::Mat& image, int l, int a = 0, int b = 0);


    /**
     * Black and white filter
     */
    class Gray : public Command {
    public:
        cv::Mat execute(const cv::Mat& image) const override;

    };


    /**
     * Blends two images with given ratio
     */
    class Blend : public Command {
    private:
        const cv::Mat& image_2;
        double value;

    public:
        Blend(const cv::Mat& image_2, double alpha = 0.5);

        cv::Mat execute(const cv::Mat& image_1) const override;

    };


    /**
     * Tint
     */
    class Tint : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Tint(int value);

        cv::Mat execute(const cv::Mat& image) const override;

    };


    /**
     *  Changes color temperature
     */
    class Temperature : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Temperature(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


    /**
     * Blurs image
     *
     * Use double value in (0, 5]
     */
    class Blur : public Command {
    private:
        double value;

    public:
        Blur(double value = 3);

        cv::Mat execute(const cv::Mat& image) const override;
    };


    /**
     * Sharpens image
     *
     * Use |value| <= 1
     */
    class Sharpen : public Command {
    private:
        double value;

    public:
        Sharpen(double value = 0.5);

        cv::Mat execute(const cv::Mat& image) const override;
    };

    /**
     * Blend image with given
     * RGB color
     *
     * Use value <= 0.4
     */
    class ApplyColor : public Command {
    private:
        int r, g, b;
        double alpha;

    public:
        ApplyColor(int r, int g, int b, double alpha = 0.1);

        cv::Mat execute(const cv::Mat& image) const override;
    };

    class TransformPerspective : public Command {
    private:
        cv::Point2f* outputQuad;

    public:
        // use value <= 0.4
        TransformPerspective(cv::Point2f* outputQuad);

        cv::Mat execute(const cv::Mat& image) const override;
    };
}

#endif //OPENCVTEST_ALGORITHMS_H
