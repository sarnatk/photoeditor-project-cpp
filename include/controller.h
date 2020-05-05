//
// Created by mark on 5/1/20.
//

#ifndef PHOTOEDITOR_CONTROLLER_H
#define PHOTOEDITOR_CONTROLLER_H

#include "algorithms.h"
#include <memory>

namespace controller {
    class Controller {
    public:

        void open_image(const cv::Mat& image);

        [[nodiscard]] bool can_undo() const;

        [[nodiscard]] bool can_redo() const;

        cv::Mat undo();

        cv::Mat redo();

        cv::Mat saturate(const cv::Mat& image, int value);

        cv::Mat crop(const cv::Mat& image, int w, int h, int x, int y);

        cv::Mat rotate_in_frame(const cv::Mat& image, double angle);

        cv::Mat brighten(const cv::Mat& image, int value);

        cv::Mat hue(const cv::Mat& image, int value);

        cv::Mat contrast(const cv::Mat& image, int value);

        cv::Mat lighten(const cv::Mat& image, int value);

        cv::Mat gray(const cv::Mat& image);

        cv::Mat blend(const cv::Mat& image_1, const cv::Mat& image_2, double alpha);

        cv::Mat tint(const cv::Mat& image, int value);

        cv::Mat temperature(const cv::Mat& image, int value);

        cv::Mat blur(const cv::Mat& image, double value);

        cv::Mat sharpen(const cv::Mat& image, double value);

        cv::Mat transform_perspective(const cv::Mat& image, cv::Point2f outputQuad[4]);

        cv::Mat apply_color(const cv::Mat& image, int r, int g, int b, double alpha);


    private:
        struct Version : public image_algorithms::Command {
            explicit Version(const image_algorithms::Command& command);

            [[nodiscard]] cv::Mat execute(const cv::Mat& image) const override;

            const image_algorithms::Command& command;
            cv::Mat mat;
        };

        int current_version = 0;
        std::deque<Version> versions;

        cv::Mat execute(const image_algorithms::Command& command, const cv::Mat& image);
    };

}
#endif //PHOTOEDITOR_CONTROLLER_H
