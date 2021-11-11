#include "util.hpp"

void drawBalls(cv::Mat& image, std::vector<billiard::detection::Ball>& balls) {
    for(auto c : balls) {
        cv::Point center = cv::Point(c._position.x, c._position.y);
        uint8_t radius = 15;
        cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

            // circle center
            cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            // circle outline
            cv::circle(image, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
        }
    }
}

cv::Mat drawDetectedBallsGrid(const cv::Mat& input, const billiard::detection::State& pixelState, int tileSize, int tilesPerLine) {

    cv::Scalar backgroundColor {255, 255, 255};
    int padding = 10;
    int totalTiles = pixelState._balls.size();
    int numberOfTileLines = totalTiles / tilesPerLine + 1;
    int imageWidth = tileSize * tilesPerLine + padding * (tilesPerLine + 1);
    int imageHeight = numberOfTileLines * tileSize + padding * (numberOfTileLines + 1);

    std::cout
            << "Ball tiles: " << " "
            << "totalTiles=" << totalTiles << " "
            << "tileSize=" << tileSize << " "
            << "tilesPerLine=" << tilesPerLine << " "
            << "numberOfTileLines=" << numberOfTileLines << " "
            << "imageWidth=" << imageWidth << " "
            << "imageHeight=" << imageHeight << " "
            << std::endl;

    float ballRadiusInPixels = 30;

    cv::Mat result {imageHeight, imageWidth, CV_8UC3, backgroundColor};

    int tileIndex = 0;
    for (auto& ball : pixelState._balls) {

        auto& position = ball._position;
        cv::Rect roi {
                (int) (position.x - ballRadiusInPixels),
                (int) (position.y - ballRadiusInPixels),
                (int) ballRadiusInPixels * 2,
                (int) ballRadiusInPixels * 2
        };
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= input.cols && roi.height <= input.rows) {

            cv::Mat ballImage = input(roi);
            cv::Mat ballImageScaled;
            cv::resize(ballImage, ballImageScaled, cv::Size {tileSize, tileSize});

            int colIndex = tileIndex % tilesPerLine;
            int rowIndex = tileIndex / tilesPerLine;
            int x = colIndex * tileSize + padding * (colIndex + 1);
            int y = rowIndex * tileSize + padding * (rowIndex + 1);
            cv::Rect resultRoi {cv::Point(x, y), cv::Size {ballImageScaled.cols, ballImageScaled.rows}};
            cv::Mat dst = result(resultRoi);
            ballImageScaled.copyTo(dst);

            std::cout
                    << "    Ball tile: " << " "
                    << "colIndex=" << colIndex << " "
                    << "rowIndex=" << rowIndex << " "
                    << "x=" << x << " "
                    << "y=" << y << " "
                    << std::endl;

        } else {
            std::cout << "unable to cut out ball image since roi is not inside image" << std::endl; // TODO: handle this?
        }
        tileIndex++;
    }
    return result;
}

