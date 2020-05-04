//
// Created by mark on 27.02.2020.
//

#ifndef OPENCVTEST_ALGORITHMS_H
#define OPENCVTEST_ALGORITHMS_H

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace image_algorithms {
    /*
     *
     * first argument -- src image
     *
     * result -- return value
     *
     */


    class Command {
    public:
        virtual cv::Mat execute(const cv::Mat& image) const = 0;
    };

    /*
     *
     * take picture using camera
     *
     * you have to create
     * cv::VideoCapture variable
     *
     * probably like this:
     * cv::VideoCapture camera(0);
     *
     */

    cv::Mat takePicture(cv::VideoCapture camera);

    /*
     *
     * does nothing
     *
     */

    class Nothing : public Command {
    public:
        cv::Mat execute(const cv::Mat& image) const override;
    };

    /*
     * crops image
     *
     * result with width = w, height = h
     *
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

/*
 *
 * rotates image by given angle without crop
 *
 */

    class RotateInFrame : public Command {
    private:
        double angle;

    public:

        RotateInFrame(double angle);

        cv::Mat execute(const cv::Mat& image) const override;

    };

/*
 *
 * saturates image
 *
 */

    class Saturate : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Saturate(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };

/*
 *
 * brightens image
 *
 */

    class Brighten : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Brighten(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


/*
 *
 * lightens image (it actually is different from brighten)
 *
*/

    class Lighten : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Lighten(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


/*
 *
 * brightens image
 *
 */

    class Hue : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Hue(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


/*
 *
 * brightens image
 *
 */

    class Contrast : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Contrast(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


/*
 *
 *  add scalar in L*a*b color space
 *
 */

    cv::Mat lab_add_scalar(const cv::Mat& image, int l, int a = 0, int b = 0);

/*
 *
 *  add scalar in hsv color space
 *
 */

    cv::Mat hsv_add_scalar(const cv::Mat& image, int l, int a = 0, int b = 0);


/*
 *
 * black and white filter
 *
 */

    class Gray : public Command {
    public:
        cv::Mat execute(const cv::Mat& image) const override;

    };


/*
 *
 * blends two images with given ratio
 *
 */

    class Blend : public Command {
    private:
        const cv::Mat& image_2;
        double value;

    public:
        Blend(const cv::Mat& image_2, double alpha = 0.5);

        cv::Mat execute(const cv::Mat& image_1) const override;

    };


/*
 *
 * tint
 *
 */

    class Tint : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Tint(int value);

        cv::Mat execute(const cv::Mat& image) const override;

    };


/*
 *
 *  changes color temperature
 *
 */

    class Temperature : public Command {
    private:
        int value;

    public:
        // use value <= 60
        Temperature(int value);

        cv::Mat execute(const cv::Mat& image) const override;
    };


/*
 *
 * blurs image
 *
 * double value in (0, 5]
 *
 */

    class Blur : public Command {
    private:
        double value;

    public:
        // use value <= 10
        Blur(double value = 3);

        cv::Mat execute(const cv::Mat& image) const override;
    };


/*
 *
 * sharpens image
 *
 */

    class Sharpen : public Command {
    private:
        double value;

    public:
        // use value <= 1
        Sharpen(double value = 0.5);

        cv::Mat execute(const cv::Mat& image) const override;
    };

/*
 *
 * blend image with given
 * rgb color
 *
 */

    class ApplyColor : public Command {
    private:
        int r, g, b;
        double alpha;

    public:
        // use value <= 0.4
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
