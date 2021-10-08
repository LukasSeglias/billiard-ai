#pragma once
#include <billiard_search/billiard_search.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <opencv2/opencv.hpp>

std::vector<std::string> labels = {
        "BLACK",
        "BROWN",
        "RED",
        "PINK",
        "YELLOW",
        "WHITE",
        "BLUE",
        "GREEN",
        "UNKNOWN"
};

const std::vector<cv::Scalar>& labelColors = {
        // WARNING: these colors are in BGR format, NOT RGB!
        cv::Scalar {0, 0, 0}, // BLACK
        cv::Scalar {0, 102, 204}, // BROWN
        cv::Scalar {0, 51, 255}, // RED
        cv::Scalar {204, 102, 255}, // PINK
        cv::Scalar {0, 255, 255}, // YELLOW
        cv::Scalar {150, 150, 150}, // WHITE
        cv::Scalar {255, 0, 0}, // BLUE
        cv::Scalar {51, 153, 0}, // GREEN
        cv::Scalar {203, 76, 140}, // UNKNOWN
};

int findLabelIndex(const std::vector<std::string>& labels, const std::string& label) {
    for (int i = 0; i < labels.size(); i++) {
        if (labels[i] == label) {
            return i;
        }
    }
    std::cerr << "Unable to find label " << label << std::endl;
    return -1;
};

cv::Mat visualize(const billiard::search::State& state, std::shared_ptr<billiard::search::SearchNode> searchNode) {

    // TODO: config
    std::pair<float, float> tableDimensions {1881.0, 943.0};
    float ballRadius = 26.15; // in millimeters

    int lineThickness = 2;
    cv::Scalar backgroundColor {255, 255, 255};
    cv::Scalar lineColor {128, 128, 128};
    int imageWidth = 1024;
    int imageHeight = (int) (tableDimensions.second / tableDimensions.first * (float) imageWidth);

    int ballRadiusInPixels = (int) (((float) imageWidth) / tableDimensions.first * ballRadius);

    auto modelPointToImagePoint = [tableDimensions, imageWidth, imageHeight](const glm::vec2& position) {

        // Model y-axis to image y-axis
        glm::vec2 yFlipped { position.x, -position.y };

        // Model origin in table center
        glm::vec2 shifted = yFlipped + glm::vec2 {tableDimensions.first / 2, tableDimensions.second / 2};

        // Rescale model dimensions to image dimensions
        glm::vec2 scaled = glm::vec2 {(float) imageWidth / tableDimensions.first, (float) imageHeight / tableDimensions.second} * shifted;

        return cv::Point2i {(int) scaled.x, (int) scaled.y};
    };

    cv::Mat image {imageHeight, imageWidth, CV_8UC3, backgroundColor};

    for (auto& ball : state._balls) {

        std::string label = ball._type;
        int labelIndex = findLabelIndex(labels, label);
        cv::Scalar ballColor = labelColors[labelIndex];
        cv::Point2i imagePoint = modelPointToImagePoint(ball._position);
        cv::circle(image, imagePoint, ballRadiusInPixels, ballColor, cv::FILLED);

        std::cout << "Ball at " << std::to_string(imagePoint.x) << ", " << std::to_string(imagePoint.y) << " ball radius (pixels): " << std::to_string(ballRadiusInPixels) << std::endl;
    }

    if (searchNode) {
        auto path = searchNode->path();

        std::optional<glm::vec2> lastPosition = std::nullopt;

        for(auto& node : path) {

            if (node->_type == billiard::search::SearchNodeType::SEARCH) {
                auto search = node->asSearch();

                std::cout << "Node on path: " << search->_ballId << std::endl;

                std::vector<billiard::search::PhysicalEvent>& events = search->_events;
                for (int i = events.size()-1; i >= 0; i--) {
                    auto& physicalEvent = events[i];

                    glm::vec2 currentPosition = physicalEvent._targetPosition;

                    if (lastPosition.has_value()) {
                        cv::Point2i lineStart = modelPointToImagePoint(lastPosition.value());
                        cv::Point2i lineEnd = modelPointToImagePoint(currentPosition);
                        cv::line(image, lineStart, lineEnd, lineColor, lineThickness);

                        std::cout << "(Model) Line between " << lastPosition.value().x << ", " << lastPosition.value().y << " and " << currentPosition.x << ", " << currentPosition.y << std::endl;
                        std::cout << "(Pixel) Line between " << lineStart.x << ", " << lineStart.y << " and " << lineEnd.x << ", " << lineEnd.y << std::endl;
                    }
                    lastPosition = std::make_optional(currentPosition);
                }
            }
        }
    }

    return image;
}
