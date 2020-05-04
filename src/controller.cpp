//
// Created by mark on 5/1/20.
//

#include "controller.h"


namespace controller {

    Controller::Version::Version(const image_algorithms::Command& command, const cv::Mat& img) : command{command},
                                                                                                 mat{img} {
    }

    cv::Mat Controller::Version::execute(const cv::Mat& image) const {
        return command.execute(image);
    }


    cv::Mat Controller::undo() {
        assert(current_version > 1);
        return versions[--current_version].mat;
    }

    cv::Mat Controller::redo() {
        assert(current_version < 9 && versions.size() > current_version + 1);
        return versions[++current_version].mat;
    }

    cv::Mat Controller::execute(const image_algorithms::Command& command, const cv::Mat& image) {
        auto this_version = Version(command, image);

        ++current_version;
        // too much images
        if (current_version > 9) {
            versions.pop_front();
            current_version = 9;
        }

        // insert in center
        while (current_version + 1 < versions.size()) {
            versions.pop_back();
        }

        versions.push_back(this_version);

        return this_version.execute(image);
    }

    cv::Mat Controller::saturate(const cv::Mat& img, int value) {
        return execute(image_algorithms::Saturate(value), img);
    }

    cv::Mat Controller::crop(const cv::Mat& img, int w, int h, int x, int y) {
        return execute(image_algorithms::Crop(w, h, x, y), img);
    }

    cv::Mat Controller::rotate_in_frame(const cv::Mat& img, double angle) {
        return execute(image_algorithms::RotateInFrame(angle), img);
    }

    cv::Mat Controller::brighten(const cv::Mat& img, int value) {
        return execute(image_algorithms::Brighten(value), img);
    }

    cv::Mat Controller::hue(const cv::Mat& img, int value) {
        return execute(image_algorithms::Hue(value), img);
    }

    cv::Mat Controller::contrast(const cv::Mat& img, int value) {
        return execute(image_algorithms::Contrast(value), img);
    }

    cv::Mat Controller::lighten(const cv::Mat& img, int value) {
        return execute(image_algorithms::Lighten(value), img);
    }

    cv::Mat Controller::gray(const cv::Mat& img) {
        return execute(image_algorithms::Gray(), img);
    }

    cv::Mat Controller::blend(const cv::Mat& img1, const cv::Mat& img2, double alpha) {
        return execute(image_algorithms::Blend(img2, alpha), img1);
    }

    cv::Mat Controller::tint(const cv::Mat& img, int value) {
        return execute(image_algorithms::Tint(value), img);
    }

    cv::Mat Controller::temperature(const cv::Mat& img, int value) {
        return execute(image_algorithms::Temperature(value), img);
    }

    cv::Mat Controller::blur(const cv::Mat& img, double value) {
        return execute(image_algorithms::Blur(value), img);
    }

    cv::Mat Controller::sharpen(const cv::Mat& img, double value) {
        return execute(image_algorithms::Sharpen(value), img);
    }

    cv::Mat Controller::transform_perspective(const cv::Mat& img, cv::Point2f* outputQuad) {
        return execute(image_algorithms::TransformPerspective(outputQuad), img);
    }

    cv::Mat Controller::apply_color(const cv::Mat& img, int r, int g, int b, double alpha) {
        return execute(image_algorithms::ApplyColor(r, g, b, alpha), img);
    }

    void Controller::open_image(const cv::Mat& image) {
        execute(image_algorithms::Nothing(), image);
    }

    bool Controller::can_undo() const {
        return current_version > 1;
    }

    bool Controller::can_redo() const {
        return current_version < 10;
    }


}