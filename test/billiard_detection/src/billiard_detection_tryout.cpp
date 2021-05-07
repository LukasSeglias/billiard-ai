#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>

void imageGallery(const std::string& name, const std::vector<cv::Mat>& images) {
    int imageIndex = 0;
    while(true) {

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) break;
        if (key == 97 /* A */) imageIndex = imageIndex == 0 ? images.size() - 1 : imageIndex - 1;
        if (key == 100 /* D */) imageIndex = (imageIndex + 1) % (images.size());

        cv::imshow(name, images[imageIndex]);
    }
}

void drawHoughResult(cv::Mat& image, std::vector<cv::Vec3f>& circles) {
    for(auto c : circles) {
        cv::Point center = cv::Point(c[0], c[1]);
        uint8_t radius = c[2];
        cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

            // circle center
            cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            // circle outline
            cv::circle(image, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
        }
    }
}

TEST(BallDetectionTests, anotherTest2) {
    const auto& imagePath = "./resources/test_detection/1.png";

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double error = 30;

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusRange = ceil(radiusInPixel * (error / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusRange;
    uint16_t maxRadius = radiusInPixel + errorRadiusRange;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 15;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusRange) << std::endl;

    auto original = imread(imagePath, cv::IMREAD_COLOR);
    cv::Mat input;
    cv::resize(original, input, cv::Size(), scale, scale);

    std::vector<cv::Mat> images;

    images.push_back(input);

    // TODO: try out different levels of blur
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

//    for(int i = 0 ; i < 4; i++) {
//        cv::Mat blurred2;
//        cv::GaussianBlur(input, blurred2, cv::Size(2*i+1, 2*i+1), 0, 0);
//        cv::imshow("blurred " + std::to_string(2*i+1) + "x" + std::to_string(2*i+1), blurred2);
//    }

//    cv::imshow("blurred", blurred);
    images.push_back(blurred);

    cv::Mat houghOnGrayscale;
    {
        cv::Mat grayscale;
        cv::cvtColor(blurred, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );

        input.copyTo(houghOnGrayscale);
        drawHoughResult(houghOnGrayscale, circles);
    }
    images.push_back(houghOnGrayscale);

    cv::Mat hsv;
    cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::Mat hue = channels[0];
    cv::Mat saturation = channels[1];
    cv::Mat value = channels[2];

//    cv::imwrite("hsv.jpeg", hsv);
//    cv::imwrite("hue.jpeg", channels[0]);
//    cv::imwrite("saturation.jpeg", channels[1]);
//    cv::imwrite("value.jpeg", channels[2]);

    images.push_back(channels[0]);
    images.push_back(channels[1]);
    images.push_back(channels[2]);

//    cv::imshow("hue", channels[0]);
//    cv::imshow("saturation", channels[1]);
//    cv::imshow("value", channels[2]);

    // Table mask
    cv::Mat tableMask;
    {
        cv::Mat hueTableMask;
        cv::inRange(hue, cv::Scalar(45), cv::Scalar(90), hueTableMask);

        cv::Mat saturationTableMask;
        cv::inRange(saturation, cv::Scalar(75), cv::Scalar(175), saturationTableMask);

        cv::Mat valueTableMask;
        cv::inRange(value, cv::Scalar(50), cv::Scalar(240), valueTableMask);

        cv::Mat totalTableMask;
        cv::bitwise_and(hueTableMask, saturationTableMask, totalTableMask);
        cv::bitwise_and(totalTableMask, valueTableMask, totalTableMask);
        cv::imshow("totalTableMask", totalTableMask);

        tableMask = totalTableMask;
    }
    cv::imshow("tableMask", tableMask);

    // Red ball mask
    cv::Mat redBallMask;
    {
        // RED/BROWN/PINK/YELLOW
        // TODO:
        // Red: Hue 0 - 5, Saturation 193, Value 203
        // Pink: Hue 0 - 5, Saturation 85, Value 251
        // Red: Hue 170 - 180, Saturation 193, Value 203
        // Pink: Hue 170 - 180, Saturation 85, Value 251
        cv::Mat hueRedMask1;
        cv::Mat hueRedMask2;
        cv::inRange(hue, cv::Scalar(150), cv::Scalar(180), hueRedMask1);
        cv::inRange(hue, cv::Scalar(0), cv::Scalar(30), hueRedMask2);
        cv::bitwise_or(hueRedMask1, hueRedMask2, redBallMask);
//        images.push_back(hueRedMask1);
//        images.push_back(hueRedMask2);
    }
//    cv::imshow("redBallMask", redBallMask);

    // Brown ball mask
//    cv::Mat brownBallMask;
//    {
//        // TODO:
//        // BROWN: Hue 6 - 12, Saturation 210, Value 150
//        cv::Mat hueBrownMask;
//        cv::inRange(hue, cv::Scalar(0), cv::Scalar(20), hueBrownMask);
////        images.push_back(hueBrownMask);
//        cv::imshow("hueBrownMask", hueBrownMask);
//
////        cv::Mat saturationBrownMask;
////        cv::inRange(saturation, cv::Scalar(135), cv::Scalar(165), saturationBrownMask);
//////        images.push_back(saturationBrownMask);
////        cv::imshow("saturationBrownMask", saturationBrownMask);
////
////        cv::Mat valueBrownMask;
////        cv::inRange(value, cv::Scalar(140), cv::Scalar(165), valueBrownMask);
//////        images.push_back(valueBrownMask);
////        cv::imshow("valueBrownMask", valueBrownMask);
////
////        cv::Mat totalBrownMask;
////        cv::bitwise_and(hueBrownMask, saturationBrownMask, totalBrownMask);
////        cv::bitwise_and(totalBrownMask, valueBrownMask, totalBrownMask);
////        cv::imshow("totalBrownMask", totalBrownMask);
//
////        int closing_size = 1;
////        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
////                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
////                                                cv::Point(closing_size, closing_size));
////        cv::Mat valueBrownMaskClosed;
////        cv::morphologyEx(valueBrownMask, valueBrownMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 5);
////        images.push_back(valueBrownMaskClosed);
//
//        brownBallMask = hueBrownMask;
//    }

    // Black ball mask
    cv::Mat blackBallMask;
    {
        // TODO:
        // BLACK: Hue Any, Saturation 50, Value 35
        cv::Mat valueBlackMask;
        cv::inRange(value, cv::Scalar(0), cv::Scalar(40), valueBlackMask);
//        images.push_back(valueBlackMask);

        int closing_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
                                                cv::Point(closing_size, closing_size));
        cv::Mat valueBlackMaskClosed;
        cv::morphologyEx(valueBlackMask, valueBlackMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 5);
//        images.push_back(valueBlackMaskClosed);

        blackBallMask = valueBlackMaskClosed;
    }
//    cv::imshow("blackBallMask", blackBallMask);

    // White ball mask
    cv::Mat whiteBallMask;
    {
        // TODO:
        // WHITE: Hue 20 - 30, Saturation 40, Value 253
        cv::Mat valueWhiteMask;
        cv::inRange(value, cv::Scalar(240), cv::Scalar(255), valueWhiteMask);
//        images.push_back(valueWhiteMask);

        int closing_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
                                                cv::Point(closing_size, closing_size));
        cv::Mat valueWhiteMaskClosed;
        cv::morphologyEx(valueWhiteMask, valueWhiteMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 5);
//        images.push_back(valueWhiteMaskClosed);

        whiteBallMask = valueWhiteMaskClosed;
    }
//    cv::imshow("whiteBallMask", whiteBallMask);

    // Yellow ball mask
//    cv::Mat yellowBallMask;
//    {
//        // TODO:
//        // YELLOW: Hue 20 - 30, Saturation 230, Value 230
//        cv::Mat hueYellowMask;
//        cv::inRange(hue, cv::Scalar(20), cv::Scalar(30), hueYellowMask);
////        images.push_back(hueYellowMask);
//
////        int closing_size = 1;
////        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
////                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
////                                                cv::Point(closing_size, closing_size));
////        cv::Mat valueWhiteMaskClosed;
////        cv::morphologyEx(hueYellowMask, valueWhiteMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 5);
////        images.push_back(valueWhiteMaskClosed);
//
//        yellowBallMask = hueYellowMask;
//        cv::imshow("yellowBallMask", yellowBallMask);
//    }

    // Blue ball mask
    cv::Mat blueBallMask;
    {
        // TODO:
        // Hue 100 - 110, Saturation 248, Value 230
        cv::Mat hueBlueMask;
        cv::inRange(hue, cv::Scalar(90), cv::Scalar(120), hueBlueMask);
//        images.push_back(hueBlueMask);
//        cv::imshow("hueBlueMask", hueBlueMask);

//        cv::Mat saturationBlueMask;
//        cv::inRange(value, cv::Scalar(190), cv::Scalar(255), saturationBlueMask);
////        images.push_back(saturationBlueMask);
//        cv::imshow("saturationBlueMask", saturationBlueMask);
//
//        cv::Mat valueBlueMask;
//        cv::inRange(value, cv::Scalar(190), cv::Scalar(255), valueBlueMask);
////        images.push_back(valueBlueMask);
//        cv::imshow("valueBlueMask", valueBlueMask);
//
//        cv::Mat blueMask;
//        cv::bitwise_and(hueBlueMask, saturationBlueMask, blueMask);
//        cv::bitwise_and(blueMask, valueBlueMask, blueMask);
//        cv::imshow("blueMask", blueMask);

        int closing_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
                                                cv::Point(closing_size, closing_size));
        cv::Mat blueMaskClosed;
        cv::morphologyEx(hueBlueMask, blueMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 5);
//        images.push_back(blueMaskClosed);
//        cv::imshow("blueMaskClosed", blueMaskClosed);

        blueBallMask = hueBlueMask;
    }
//    cv::imshow("blueBallMask", blueBallMask);

    // Green ball mask
    cv::Mat greenBallMask;
    {
        // TODO:
        // GREEN: Hue 80 - 90, Saturation 150, Value 130
        cv::Mat hueGreenMask;
        cv::inRange(hue, cv::Scalar(45), cv::Scalar(90), hueGreenMask);
//        images.push_back(hueGreenMask);
//        cv::imshow("hueGreenMask", hueGreenMask);

        cv::Mat saturationGreenMask;
        cv::inRange(saturation, cv::Scalar(200), cv::Scalar(255), saturationGreenMask);
//        cv::imshow("saturationGreenMask", saturationGreenMask);

//        cv::Mat valueGreenMask;
//        cv::inRange(value, cv::Scalar(100), cv::Scalar(125), valueGreenMask);
//        cv::imshow("valueGreenMask", valueGreenMask);

        cv::Mat totalGreenMask;
        cv::bitwise_and(hueGreenMask, saturationGreenMask, totalGreenMask);
//        cv::imshow("totalGreenMask", totalGreenMask);

        int closing_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
                                                cv::Point(closing_size, closing_size));
        cv::Mat grenMaskClosed;
        cv::morphologyEx(totalGreenMask, grenMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 5);
//        images.push_back(valueBlueMaskClosed);

        greenBallMask = grenMaskClosed;
    }
//    cv::imshow("greenBallMask", greenBallMask);

//    int erosion_size = 1;
//    cv::Mat element = getStructuringElement(cv::MORPH_RECT,
//                                         cv::Size(2*erosion_size + 1, 2*erosion_size+1),
//                                         cv::Point(erosion_size, erosion_size));
//    cv::Mat eroded;
//    cv::erode(redBallMask, eroded, element, cv::Point(-1, -1), 13);

//    cv::imshow("eroded", eroded);
//    images.push_back(eroded);

    cv::Mat totalMask;
    cv::bitwise_or(redBallMask, blackBallMask, totalMask);
    cv::bitwise_or(totalMask, whiteBallMask, totalMask);
    cv::bitwise_or(totalMask, blueBallMask, totalMask);
    cv::bitwise_or(totalMask, greenBallMask, totalMask);
//    cv::bitwise_or(totalMask, brownBallMask, totalMask);
//    images.push_back(totalMask);

    cv::Mat masked_blurred;
    cv::bitwise_and(blurred, blurred, masked_blurred, totalMask);
    images.push_back(masked_blurred);

    cv::Mat houghOnMaskedInput;
    {
        cv::Mat grayscale;
        cv::cvtColor(masked_blurred, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );

        input.copyTo(houghOnMaskedInput);
        drawHoughResult(houghOnMaskedInput, circles);
    }
    images.push_back(houghOnMaskedInput);

    cv::Mat tableMaskInverted;
    cv::bitwise_not(tableMask, tableMaskInverted);
    cv::Mat backgroundRemoved;
    cv::bitwise_and(blurred, blurred, backgroundRemoved, tableMaskInverted);
    images.push_back(backgroundRemoved);
    cv::imshow("backgroundRemoved", backgroundRemoved);

    cv::Mat backgroundRemovedClosed;
    {
        int closing_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
                                                cv::Point(closing_size, closing_size));

        cv::morphologyEx(backgroundRemoved, backgroundRemovedClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 2);
    }
    cv::imshow("backgroundRemovedClosed", backgroundRemovedClosed);

    std::vector<cv::Mat> ballImages;
    cv::Mat houghOnBackgroundRemovedInput;
    {
        cv::Mat grayscale;
        cv::cvtColor(backgroundRemoved, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );

        input.copyTo(houghOnBackgroundRemovedInput);
        drawHoughResult(houghOnBackgroundRemovedInput, circles);
    }
    images.push_back(houghOnBackgroundRemovedInput);

    cv::Mat hueLabels(redBallMask.size(), CV_8UC3);
    {
        cv::Mat labels(redBallMask.size(), CV_32S);
//        int labelCount = cv::connectedComponents(redBallMask, labels, 8, CV_32S);
        cv::Mat stats;
        cv::Mat centroids;
        int labelCount = cv::connectedComponentsWithStats(redBallMask, labels, stats, centroids, 8, CV_32S);

        std::cout << "Number of hue labels: " << labelCount << std::endl;

        std::vector<cv::Vec3b> colors(labelCount);
        colors[0] = cv::Vec3b(0, 0, 0);//background
        for (int label = 1; label < labelCount; ++label) {
            int area = stats.at<int>(label, cv::ConnectedComponentsTypes::CC_STAT_AREA );
            cv::Point center(centroids.at<double>(label, 0), centroids.at<double>(label, 1));

            if (area > 1000 && area < 10000) {
//                std::cout << "label: " << std::to_string(label) << " center: " << center << " area: " << std::to_string(area) << std::endl;
                colors[label] = cv::Vec3b( (rand()&255), (rand()&255), (rand()&255) );
            } else {
                colors[label] = cv::Vec3b(0, 0, 0);
            }
        }
        for (int r = 0; r < hueLabels.rows; ++r) {
            for (int c = 0; c < hueLabels.cols; ++c) {
                int label = labels.at<int>(r, c);
                cv::Vec3b &pixel = hueLabels.at<cv::Vec3b>(r, c);
                pixel = colors[label];
            }
        }
        for (int label = 1; label < labelCount; ++label) {
            int area = stats.at<int>(label, cv::ConnectedComponentsTypes::CC_STAT_AREA );
            cv::Point center(centroids.at<double>(label, 0), centroids.at<double>(label, 1));

            if (area > 1000 && area < 10000) {
                cv::circle(hueLabels, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            }
        }
    }

//    cv::imshow("components", hueLabels);

    images.push_back(hueLabels);

    cv::Mat houghOnHueLabels;
    {
        cv::Mat grayscale;
        cv::cvtColor(hueLabels, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );

        input.copyTo(houghOnHueLabels);
        drawHoughResult(houghOnHueLabels, circles);
    }
    images.push_back(houghOnHueLabels);

    imageGallery("Image", images);
    imageGallery("Ball", ballImages);
}

void hsvFromBgr(cv::Mat& bgr, cv::Mat& hue, cv::Mat& saturation, cv::Mat& value) {
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    hue = channels[0];
    saturation = channels[1];
    value = channels[2];
}

TEST(BallDetectionTest, houghOnSaturation) {

    std::vector<std::string> paths {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double error = 30;

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusRange = ceil(radiusInPixel * (error / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusRange;
    uint16_t maxRadius = radiusInPixel + errorRadiusRange;

    // minimal distance between centers
//    double houghMinDistance = minRadius * 2;
    double houghMinDistance = minRadius;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 15;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusRange) << std::endl;

    for (auto& imagePath : paths) {

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat resized;
        cv::resize(original, resized, cv::Size(), scale, scale);

        cv::Mat blurred;
        cv::GaussianBlur(resized, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat input = blurred;

        cv::Mat hue, saturation, value;
        hsvFromBgr(input, hue, saturation, value);

        std::stringstream saturationTitle;
        saturationTitle << "saturation " << imagePath;
        cv::imshow(saturationTitle.str(), saturation);

        cv::Point2d tableHueFilter {45, 100};
        cv::Point2d tableSaturationFilter {0, 180};
        cv::Point2d tableValueFilter {0, 200};

        cv::Mat tableMask;
        {
            cv::Mat hueTableMask;
            cv::inRange(hue, tableHueFilter.x, tableHueFilter.y, hueTableMask);

            cv::Mat saturationTableMask;
            cv::inRange(saturation, tableSaturationFilter.x, tableSaturationFilter.y, saturationTableMask);

            cv::Mat valueTableMask;
            cv::inRange(value, tableValueFilter.x, tableValueFilter.y, valueTableMask);

            cv::Mat totalTableMask;
            cv::bitwise_and(hueTableMask, saturationTableMask, totalTableMask);
            cv::bitwise_and(totalTableMask, valueTableMask, totalTableMask);

            tableMask = totalTableMask;
        }

        cv::Mat maskedImage;
        cv::bitwise_and(input, input, maskedImage, tableMask);

        std::stringstream tableMaskTitle;
        tableMaskTitle << "tableMask " << imagePath;
        cv::imshow(tableMaskTitle.str(), maskedImage);

        cv::Mat nonTableMask;
        cv::bitwise_not(tableMask, nonTableMask);

        cv::Mat maskedSaturation;
        cv::bitwise_and(saturation, saturation, maskedSaturation, nonTableMask);

        cv::Mat houghOnSaturation;
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(maskedSaturation, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnSaturation);
            drawHoughResult(houghOnSaturation, circles);
        }
        std::stringstream ss;
        ss << "houghOnMaskedSaturation " << imagePath;
        cv::imshow(ss.str(), houghOnSaturation);
    }

    cv::waitKey();
}

cv::Point2i findClosestPoint(cv::Point2i& point, std::vector<cv::Point2i>& points) {

    cv::Point2i closest {0,0};
    double min_dist = 999999;
    for(auto& p : points) {
        auto test = point - p;
        double dist = cv::abs(cv::norm(test));
        if (dist < min_dist) {
            min_dist = dist;
            closest = p;
        }
    }
    return closest;
}

TEST(BallDetectionTests, houghOnHueLabels) {
    const auto& imagePath = "./resources/test_detection/8.png";

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    auto original = imread(imagePath, cv::IMREAD_COLOR);
    cv::Mat input;
    cv::resize(original, input, cv::Size(), scale, scale);

    std::vector<cv::Mat> images;

    images.push_back(input);

    // TODO: try out different levels of blur
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

    images.push_back(blurred);

    cv::Mat hue, saturation, value;
    hsvFromBgr(blurred, hue, saturation, value);
    cv::imshow("hue", hue);
    cv::imshow("saturation", saturation);
    cv::imshow("value", value);

    cv::Mat hueLabels(hue.size(), CV_8UC3);
    {
        cv::Mat labels(hue.size(), CV_32S);
//        int labelCount = cv::connectedComponents(redBallMask, labels, 8, CV_32S);
        cv::Mat stats;
        cv::Mat centroids;
        int labelCount = cv::connectedComponentsWithStats(hue, labels, stats, centroids, 8, CV_32S);

        std::cout << "Number of hue labels: " << labelCount << std::endl;

        std::vector<cv::Vec3b> colors(labelCount);
        colors[0] = cv::Vec3b(0, 0, 0);//background
        for (int label = 1; label < labelCount; ++label) {
            int area = stats.at<int>(label, cv::ConnectedComponentsTypes::CC_STAT_AREA );
            cv::Point center(centroids.at<double>(label, 0), centroids.at<double>(label, 1));

            if (area > 1000 && area < 10000) {
//                std::cout << "label: " << std::to_string(label) << " center: " << center << " area: " << std::to_string(area) << std::endl;
                colors[label] = cv::Vec3b( (rand()&255), (rand()&255), (rand()&255) );
            } else {
                colors[label] = cv::Vec3b(0, 0, 0);
            }
        }
        for (int r = 0; r < hueLabels.rows; ++r) {
            for (int c = 0; c < hueLabels.cols; ++c) {
                int label = labels.at<int>(r, c);
                cv::Vec3b &pixel = hueLabels.at<cv::Vec3b>(r, c);
                pixel = colors[label];
            }
        }
        for (int label = 1; label < labelCount; ++label) {
            int area = stats.at<int>(label, cv::ConnectedComponentsTypes::CC_STAT_AREA );
            cv::Point center(centroids.at<double>(label, 0), centroids.at<double>(label, 1));

            if (area > 1000 && area < 10000) {
                cv::circle(hueLabels, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            }
        }
    }

    images.push_back(hueLabels);

    cv::Mat houghOnHueLabels;
    {
        cv::Mat grayscale;
        cv::cvtColor(hueLabels, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );

        input.copyTo(houghOnHueLabels);
        drawHoughResult(houghOnHueLabels, circles);
    }
    images.push_back(houghOnHueLabels);

    imageGallery("Image", images);
}

inline std::vector<cv::Vec3f> filterCircles(const std::vector<cv::Vec3f>& circles, const cv::Rect& rect) {
    std::vector<cv::Vec3f> filteredCircles;
    for(auto& circle : circles) {
        auto center = cv::Point2i{(int)circle[0], (int)circle[1]};
        if (rect.contains(center)) {
            filteredCircles.push_back(circle);
        }
    }
    return filteredCircles;
}

inline std::vector<cv::Vec3f> filterCircles(const std::vector<cv::Vec3f>& circles, const cv::Mat& mask, bool expectedValue) {
    std::vector<cv::Vec3f> filteredCircles;
    for(auto& circle : circles) {
        auto center = cv::Point2i{(int)circle[0], (int)circle[1]};
        if ((bool)mask.at<char>(center.y, center.x) == expectedValue) {
            filteredCircles.push_back(circle);
        }
    }
    return filteredCircles;
}

void drawCircles(cv::Mat& output, std::vector<cv::Vec3f>& circles, cv::Scalar color) {
    for(auto c : circles) {
        cv::Point center = cv::Point(c[0], c[1]);
        uint8_t radius = c[2];
        cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= output.cols && roi.height <= output.rows) {
            cv::circle(output, center, 13, color, cv::FILLED);
        }
    }
}

TEST(BallDetectionTests, ballDetectionByRemovingTableGreen) {
    const auto& imagePath = "./resources/test_detection/8.png";

    cv::Point2d tableHueFilter {0, 100};
    cv::Point2d tableSaturationFilter {0, 150};
    cv::Point2d tableValueFilter {0, 240};

    // Filter for saturated balls
    cv::Point2d ballsSaturationFilter {128, 255};

    // Filter for black ball
    cv::Point2d blackBallValueFilter {0, 60};

    // Filter for white ball
    cv::Point2d whiteBallHueFilter {0, 60};
    cv::Point2d whiteBallSaturationFilter {0, 200};
    cv::Point2d whiteBallValueFilter {200, 255};

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    auto original = imread(imagePath, cv::IMREAD_COLOR);
    cv::Mat input;
    cv::resize(original, input, cv::Size(), scale, scale);
    cv::imshow("input", input);

    // TODO: try out different levels of blur
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);
    cv::imshow("blurred", blurred);

    cv::Mat hue, saturation, value;
    hsvFromBgr(blurred, hue, saturation, value);
    cv::imshow("hue", hue);
    cv::imshow("saturation", saturation);
    cv::imshow("value", value);

    // Table mask
    cv::Mat tableMask;
    {
        cv::Mat hueTableMask;
        cv::inRange(hue, tableHueFilter.x, tableHueFilter.y, hueTableMask);

        cv::Mat saturationTableMask;
        cv::inRange(saturation, tableSaturationFilter.x, tableSaturationFilter.y, saturationTableMask);

        cv::Mat valueTableMask;
        cv::inRange(value, tableValueFilter.x, tableValueFilter.y, valueTableMask);

        cv::Mat totalTableMask;
        cv::bitwise_and(hueTableMask, saturationTableMask, totalTableMask);
        cv::bitwise_and(totalTableMask, valueTableMask, totalTableMask);

        tableMask = totalTableMask;
    }

    // Apply opening in order to close holes in black areas
    cv::Mat tableMaskOpened;
    {
        int opening_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*opening_size + 1, 2*opening_size+1),
                                                cv::Point(opening_size, opening_size));

        cv::morphologyEx(tableMask, tableMaskOpened, cv::MORPH_OPEN, element, cv::Point(-1, -1), 1);
    }

    // Apply closing in order to remove excess black pixels
    cv::Mat tableMaskClosed;
    {
        int closing_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
                                                cv::Point(closing_size, closing_size));

        cv::morphologyEx(tableMaskOpened, tableMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 1);
    }

    cv::imshow("tableMask", tableMask);
    cv::imshow("tableMaskOpened", tableMaskOpened);
    cv::imshow("tableMaskClosed", tableMaskClosed);

    cv::Mat tableMaskInverted;
    cv::bitwise_not(tableMask, tableMaskInverted);

    cv::Mat backgroundRemoved;
    cv::bitwise_and(blurred, blurred, backgroundRemoved, tableMaskInverted);
    cv::imshow("backgroundRemoved", backgroundRemoved);

    cv::Mat table;
    cv::bitwise_and(blurred, blurred, table, tableMask);

    cv::Mat nonTable;
    cv::bitwise_and(blurred, blurred, nonTable, tableMaskInverted);

    cv::imshow("table", table);
    cv::imshow("nonTable", nonTable);

    cv::Mat houghOnBackgroundRemovedInput;
    {
        cv::Mat grayscale;
        cv::cvtColor(backgroundRemoved, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );
        std::vector<cv::Vec3f> filteredCircles = filterCircles(circles, table_rect);

        input.copyTo(houghOnBackgroundRemovedInput);
        drawHoughResult(houghOnBackgroundRemovedInput, filteredCircles);
    }
    cv::imshow("houghOnBackgroundRemovedInput", houghOnBackgroundRemovedInput);

    cv::waitKey();
}

// TODO: Es werden nicht alle roten Kugeln detektiert.
// TODO: Es werden fälschlicherweise pinke Kugeln detektiert.
TEST(BallDetectionTests, redMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        cv::imshow("input", input);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);
        cv::imshow("blurred", blurred);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat redMask;
        {
            cv::Point2d redHueFilter1 {0, 10};
            cv::Point2d redHueFilter2 {170, 180};

            cv::Mat redMask1;
            cv::inRange(hue, redHueFilter1.x, redHueFilter1.y, redMask1);
            cv::imshow("redMask1", redMask1);

            cv::Mat redMask2;
            cv::inRange(hue, redHueFilter2.x, redHueFilter2.y, redMask2);
            cv::imshow("redMask2", redMask2);

            cv::Mat totalMask;
            cv::bitwise_or(redMask1, redMask2, totalMask);

            cv::Mat openedMask;
            cv::morphologyEx(totalMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 3);
            cv::imshow("openedMask", openedMask);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 3);
            cv::imshow("closedMask", closedMask);

            cv::Mat redSaturation;
            cv::Point2d saturationFilter {100, 255};
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, redSaturation);
            cv::imshow("redSaturation", redSaturation);

            cv::Mat redMaskAndedWithRedSaturation;
            cv::bitwise_and(closedMask, redSaturation, redMaskAndedWithRedSaturation);
            cv::imshow("redMaskAndedWithRedSaturation", redMaskAndedWithRedSaturation);

            cv::Mat redValue;
            cv::Point2d valueFilter {215, 255};
            cv::inRange(value, valueFilter.x, valueFilter.y, redValue);
            cv::imshow("redValue", redValue);

            cv::Mat redMaskAndedWithRedValue;
            cv::bitwise_and(closedMask, redValue, redMaskAndedWithRedValue);
            cv::imshow("redMaskAndedWithRedValue", redMaskAndedWithRedValue);

            redMask = redMaskAndedWithRedValue;
        }
        cv::imshow("redMask", redMask);

        cv::Mat redMaskedInput;
        cv::bitwise_and(input, input, redMaskedInput, redMask);
        cv::imshow("redMaskedInput", redMaskedInput);

        cv::Mat invRedMask;
        cv::bitwise_not(redMask, invRedMask);
        cv::Mat redMaskedDelta;
        cv::bitwise_and(input, input, redMaskedDelta, invRedMask);
        cv::imshow("redMaskedDelta", redMaskedDelta);

        cv::Mat houghOnRedMaskedInput;
        cv::Mat houghOnMaskedInputAfterCircleFilter1;
        cv::Mat houghOnMaskedInputAfterCircleFilter2;
        {
            cv::Mat grayscale;
            cv::cvtColor(redMaskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::imshow("redMaskedInput grayscale", grayscale);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);
            cv::imshow("redMaskedInput grayscale opened", opened);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            cv::imshow("redMaskedInput grayscale edges", edges);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, redMask, true);

            input.copyTo(houghOnRedMaskedInput);
            drawHoughResult(houghOnRedMaskedInput, circles);

            input.copyTo(houghOnMaskedInputAfterCircleFilter1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);

            input.copyTo(houghOnMaskedInputAfterCircleFilter2);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
        }
        cv::imshow("hough on masked input", houghOnRedMaskedInput);
        cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
        cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }
}

// TODO: Derzeit vermutlich die beste Variante.
// TODO: Die pinke Kugel wird nur manchmal detektiert
// TODO: Die schwarze Kugel wird manchmal detektiert
// DONE: Die weisse Kugeln wird nie detektiert.
TEST(BallDetectionTests, blueGreenYellowMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat ballMask;
        {
            cv::Point2d saturationFilter {100, 255};

            cv::Mat saturationMask;
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, saturationMask);

            cv::Mat openedMask;
            cv::morphologyEx(saturationMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            ballMask = closedMask;

            cv::imshow("saturationMask", saturationMask);
            cv::imshow("openedMask", openedMask);
            cv::imshow("closedMask", closedMask);
        }
        cv::imshow("ballMask", ballMask);

        cv::Mat maskedInput;
        cv::bitwise_and(input, input, maskedInput, ballMask);
        cv::imshow("maskedInput", maskedInput);

        cv::Mat invMask;
        cv::bitwise_not(ballMask, invMask);
        cv::Mat maskedDelta;
        cv::bitwise_and(input, input, maskedDelta, invMask);
        cv::imshow("maskedDelta", maskedDelta);

        cv::Mat houghOnMaskedInput;
        cv::Mat houghOnMaskedInputAfterCircleFilter1;
        cv::Mat houghOnMaskedInputAfterCircleFilter2;
        {
            cv::Mat grayscale;
            cv::cvtColor(maskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::imshow("maskedInput grayscale", grayscale);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);
            cv::imshow("maskedInput grayscale opened", opened);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            cv::imshow("maskedInput grayscale edges", edges);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, ballMask, true);

            input.copyTo(houghOnMaskedInput);
            drawHoughResult(houghOnMaskedInput, circles);

            input.copyTo(houghOnMaskedInputAfterCircleFilter1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);

            input.copyTo(houghOnMaskedInputAfterCircleFilter2);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
        }
        cv::imshow("hough on masked input", houghOnMaskedInput);
        cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
        cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

// DONE: Es wird keine andere Kugel ausser dei Schwarze detektiert
// TODO: Es werden fälschlicherweise Kugeln in den löchern detektiert.
// TODO: Es werden fälschlicherweise Kugeln detektiert, wo keine sind.
// TODO: Es werden fälschlicherweise Schatten als Kugeln detektiert.
// TODO: ausprobieren: Segmentation anhand wenig Sättigung und bei Hue grün ausschliessen?
TEST(BallDetectionTests, blackMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat blackMask;
        {
            cv::Point2d valueFilter {0, 100};
            cv::Point2d saturationFilter {0, 160};

            cv::Point2d greenHueFilter {30, 90};
            cv::Mat greenMask;
            cv::inRange(hue, greenHueFilter.x, greenHueFilter.y, greenMask);

            cv::Mat nonGreenMask;
            cv::bitwise_not(greenMask, nonGreenMask);

            cv::Mat openedNonGreenMask;
            cv::morphologyEx(nonGreenMask, openedNonGreenMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedNonGreenMask;
            cv::morphologyEx(openedNonGreenMask, closedNonGreenMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat valueMask;
            cv::inRange(value, valueFilter.x, valueFilter.y, valueMask);

            cv::Mat openedValueMask;
            cv::morphologyEx(valueMask, openedValueMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat closedValueMask;
            cv::morphologyEx(openedValueMask, closedValueMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat saturationMask;
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, saturationMask);

            cv::Mat openedSaturationMask;
            cv::morphologyEx(saturationMask, openedSaturationMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat nonGreenLowSaturationMask;
            cv::bitwise_and(closedNonGreenMask, saturationMask, nonGreenLowSaturationMask);

            cv::Mat nonGreenAndedWithValueMask;
            cv::bitwise_and(closedNonGreenMask, closedValueMask, nonGreenAndedWithValueMask);

            cv::Mat totalMask;
            cv::bitwise_and(closedValueMask, openedSaturationMask, totalMask);

            cv::Mat openedMask;
            cv::morphologyEx(totalMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            blackMask = closedMask;

            cv::imshow("greenMask", greenMask);
            cv::imshow("nonGreenMask", nonGreenMask);
            cv::imshow("openedNonGreenMask", openedNonGreenMask);
            cv::imshow("closedNonGreenMask", closedNonGreenMask);
            cv::imshow("nonGreenLowSaturationMask", nonGreenLowSaturationMask);
            cv::imshow("nonGreenAndedWithValueMask", nonGreenAndedWithValueMask);
            cv::imshow("valueMask", valueMask);
            cv::imshow("openedValueMask", openedValueMask);
            cv::imshow("closedValueMask", closedValueMask);
            cv::imshow("saturationMask", saturationMask);
            cv::imshow("openedSaturationMask", openedSaturationMask);
            cv::imshow("totalMask", totalMask);
            cv::imshow("openedMask", openedMask);
            cv::imshow("closedMask", closedMask);
        }
        cv::imshow("blackMask", blackMask);

        cv::Mat maskedInput;
        cv::bitwise_and(input, input, maskedInput, blackMask);
        cv::imshow("maskedInput", maskedInput);

        cv::Mat invMask;
        cv::bitwise_not(blackMask, invMask);
        cv::Mat maskedDelta;
        cv::bitwise_and(input, input, maskedDelta, invMask);
        cv::imshow("maskedDelta", maskedDelta);

        cv::Mat houghOnMaskedInput;
        cv::Mat houghOnMaskedInputAfterCircleFilter1;
        cv::Mat houghOnMaskedInputAfterCircleFilter2;
        {
            cv::Mat grayscale;
            cv::cvtColor(maskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::imshow("maskedInput grayscale", grayscale);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);
            cv::imshow("maskedInput grayscale opened", opened);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            cv::imshow("maskedInput grayscale edges", edges);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, blackMask, true);

            input.copyTo(houghOnMaskedInput);
            drawHoughResult(houghOnMaskedInput, circles);

            input.copyTo(houghOnMaskedInputAfterCircleFilter1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);

            input.copyTo(houghOnMaskedInputAfterCircleFilter2);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
        }
        cv::imshow("hough on masked input", houghOnMaskedInput);
        cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
        cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }
}

// TODO: Es werden fälschlicherweise rote Kugeln detektiert.
// TODO: Es werden fälschlicherweise gelbe Kugeln detektiert.
// TODO: Es werden fälschlicherweise blaue Kugeln detektiert.
TEST(BallDetectionTests, whitePinkMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat whitePinkMask;
        {
            cv::Point2d valueFilter {200, 255};

            cv::Mat valueMask;
            cv::inRange(value, valueFilter.x, valueFilter.y, valueMask);

            cv::Mat openedMask;
            cv::morphologyEx(valueMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 1);

            whitePinkMask = closedMask;

            cv::imshow("valueMask", valueMask);
            cv::imshow("openedMask", openedMask);
            cv::imshow("closedMask", closedMask);
            cv::imshow("whitePinkMask", whitePinkMask);
        }

        cv::Mat maskedInput;
        {
            cv::bitwise_and(input, input, maskedInput, whitePinkMask);

            cv::Mat invMask;
            cv::bitwise_not(whitePinkMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            cv::imshow("maskedInput", maskedInput);
            cv::imshow("maskedDelta", maskedDelta);
        }

        std::vector<cv::Vec3f> whitePinkCircles;
        {
            cv::Mat grayscale;
            cv::cvtColor(maskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            HoughCircles(grayscale, whitePinkCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            cv::imshow("maskedInput grayscale", grayscale);
            cv::imshow("maskedInput grayscale opened", opened);
            cv::imshow("maskedInput grayscale edges", edges);
        }

        {
            std::vector<cv::Vec3f> circles = whitePinkCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, whitePinkMask, true);

            cv::Mat houghOnMaskedInput = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();

            drawHoughResult(houghOnMaskedInput, whitePinkCircles);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);

            cv::imshow("hough on masked input", houghOnMaskedInput);
            cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
            cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
        }

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

// TODO: prüfen, ob es Kreise gibt, welche (fast) an derselben Position stehen
// TODO: Es werden fälschlicherweise Kugeln in den löchern detektiert.
// TODO: Es werden fälschlicherweise Kugeln detektiert, wo keine sind.
// TODO: Es werden fälschlicherweise Schatten als Kugeln detektiert.
TEST(BallDetectionTests, combined) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    std::vector<int> expectedBallCounts = {
            22,
            22,
            22,
            22, // TODO: gibt es hier doppelte kreise?
            22,
            21,
            21,
            22
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    cv::Point2d saturationFilter {100, 255};

    bool showDebuggingOutput = true;
    bool showBlack = true;
    bool showSaturated = true;
    bool showWhitePink = true;

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        // Apply blur
        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        // Convert image into HSV and retrieve separate channels
        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

        if (showDebuggingOutput) {
            cv::imshow("input", input);
            cv::imshow("blurred", blurred);
            cv::imshow("hue", hue);
            cv::imshow("saturation", saturation);
            cv::imshow("value", value);
        }

        // Filter on saturation to retrieve a mask

        cv::Mat saturationMask;
        cv::inRange(saturation, saturationFilter.x, saturationFilter.y, saturationMask);

        cv::Mat openedSaturationMask;
        cv::morphologyEx(saturationMask, openedSaturationMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

        cv::Mat closedSaturationMask;
        cv::morphologyEx(openedSaturationMask, closedSaturationMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

        cv::Mat saturatedBallMask = closedSaturationMask;

        if (showDebuggingOutput && showSaturated) {
            cv::imshow("saturationMask", saturationMask);
            cv::imshow("openedSaturationMask", openedSaturationMask);
            cv::imshow("closedSaturationMask", closedSaturationMask);
            cv::imshow("saturatedBallMask", saturatedBallMask);
        }

        cv::Mat saturationMaskedInput;
        {
            cv::bitwise_and(input, input, saturationMaskedInput, saturatedBallMask);

            cv::Mat invMask;
            cv::bitwise_not(saturatedBallMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            if (showDebuggingOutput && showSaturated) {
                cv::imshow("saturationMask - maskedInput", saturationMaskedInput);
                cv::imshow("saturationMask - maskedDelta", maskedDelta);
            }
        }

        // Black ball mask

        cv::Mat blackMask;
        {
            cv::Point2d valueFilter {0, 100};
            cv::Point2d saturationFilter {0, 160};
            cv::Point2d greenHueFilter {30, 90};

            cv::Mat valueMask;
            cv::inRange(value, valueFilter.x, valueFilter.y, valueMask);

            cv::Mat openedValueMask;
            cv::morphologyEx(valueMask, openedValueMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat closedValueMask;
            cv::morphologyEx(openedValueMask, closedValueMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat saturationMask;
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, saturationMask);

            cv::Mat openedSaturationMask;
            cv::morphologyEx(saturationMask, openedSaturationMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat totalMask;
            cv::bitwise_and(closedValueMask, openedSaturationMask, totalMask);

            cv::Mat openedMask;
            cv::morphologyEx(totalMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            blackMask = closedMask;

            if (showDebuggingOutput && showBlack) {
                cv::imshow("black - valueMask", valueMask);
                cv::imshow("black - openedValueMask", openedValueMask);
                cv::imshow("black - closedValueMask", closedValueMask);
                cv::imshow("black - saturationMask", saturationMask);
                cv::imshow("black - openedSaturationMask", openedSaturationMask);
                cv::imshow("black - totalMask", totalMask);
                cv::imshow("black - openedMask", openedMask);
                cv::imshow("black - closedMask", closedMask);
                cv::imshow("black - mask", blackMask);
            }

            cv::Mat greenMask;
            cv::inRange(hue, greenHueFilter.x, greenHueFilter.y, greenMask);

            cv::Mat nonGreenMask;
            cv::bitwise_not(greenMask, nonGreenMask);

            cv::Mat openedNonGreenMask;
            cv::morphologyEx(nonGreenMask, openedNonGreenMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedNonGreenMask;
            cv::morphologyEx(openedNonGreenMask, closedNonGreenMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat nonGreenLowSaturationMask;
            cv::bitwise_and(closedNonGreenMask, saturationMask, nonGreenLowSaturationMask);

            cv::Mat nonGreenAndedWithValueMask;
            cv::bitwise_and(closedNonGreenMask, closedValueMask, nonGreenAndedWithValueMask);

            if (showDebuggingOutput && showBlack) {
                cv::imshow("black - greenMask", greenMask);
                cv::imshow("black - nonGreenMask", nonGreenMask);
                cv::imshow("black - openedNonGreenMask", openedNonGreenMask);
                cv::imshow("black - closedNonGreenMask", closedNonGreenMask);
                cv::imshow("black - nonGreenLowSaturationMask", nonGreenLowSaturationMask);
                cv::imshow("black - nonGreenAndedWithValueMask", nonGreenAndedWithValueMask);
            }
        }

        cv::Mat blackMaskedInput;
        {
            cv::bitwise_and(input, input, blackMaskedInput, blackMask);

            cv::Mat invMask;
            cv::bitwise_not(blackMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            if (showDebuggingOutput && showBlack) {
                cv::imshow("black - maskedInput", blackMaskedInput);
                cv::imshow("black - maskedDelta", maskedDelta);
            }
        }

        // White & Pink Mask

        cv::Mat whitePinkMask;
        {
            cv::Point2d valueFilter {200, 255};

            cv::Mat valueMask;
            cv::inRange(value, valueFilter.x, valueFilter.y, valueMask);

            cv::Mat openedMask;
            cv::morphologyEx(valueMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 1);

            whitePinkMask = closedMask;

            if (showDebuggingOutput && showWhitePink) {
                cv::imshow("white&pink - valueMask", valueMask);
                cv::imshow("white&pink - openedMask", openedMask);
                cv::imshow("white&pink - closedMask", closedMask);
                cv::imshow("white&pink - mask", whitePinkMask);
            }
        }

        cv::Mat whitePinkMaskedInput;
        {
            cv::bitwise_and(input, input, whitePinkMaskedInput, whitePinkMask);

            cv::Mat invMask;
            cv::bitwise_not(whitePinkMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            if (showDebuggingOutput && showWhitePink) {
                cv::imshow("white&pink - maskedInput", whitePinkMaskedInput);
                cv::imshow("white&pink - maskedDelta", maskedDelta);
            }
        }

        // Hough on saturation masked input
        std::vector<cv::Vec3f> saturationCircles;
        {
            cv::Mat grayscale;
            cv::cvtColor(saturationMaskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::Mat edges;
            cv::Canny(grayscale, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            HoughCircles(grayscale, saturationCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            if (showDebuggingOutput && showSaturated) {
                cv::imshow("saturationMask - maskedInput grayscale", grayscale);
                cv::imshow("saturationMask - maskedInput grayscale edges", edges);
            }
        }

        // Hough on black masked input
        std::vector<cv::Vec3f> blackCircles;
        {
            cv::Mat grayscale;
            cv::cvtColor(blackMaskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::Mat edges;
            cv::Canny(grayscale, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            HoughCircles(grayscale, blackCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            if (showDebuggingOutput && showBlack) {
                cv::imshow("black - maskedInput grayscale", grayscale);
                cv::imshow("black - maskedInput grayscale edges", edges);
            }
        }

        // Hough on white&pink masked input
        std::vector<cv::Vec3f> whitePinkCircles;
        {
            cv::Mat grayscale;
            cv::cvtColor(whitePinkMaskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::Mat edges;
            cv::Canny(grayscale, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            HoughCircles(grayscale, whitePinkCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            if (showDebuggingOutput && showWhitePink) {
                cv::imshow("white&pink - maskedInput grayscale", grayscale);
                cv::imshow("white&pink - maskedInput grayscale edges", edges);
            }
        }

        std::vector<cv::Vec3f> allCircles;

        {
            std::vector<cv::Vec3f> circles = saturationCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, saturatedBallMask, true);
            for (auto& circle : filteredCircles2) allCircles.push_back(circle);

            if (showDebuggingOutput && showSaturated) {
                cv::Mat houghOnMaskedInput = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();

                drawHoughResult(houghOnMaskedInput, circles);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);

                cv::imshow("saturated - hough on masked input", houghOnMaskedInput);
                cv::imshow("saturated - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
                cv::imshow("saturated - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
            }
        }

        {
            std::vector<cv::Vec3f> circles = blackCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, blackMask, true);
            for (auto& circle : filteredCircles2) allCircles.push_back(circle);

            if (showDebuggingOutput && showBlack) {
                cv::Mat houghOnMaskedInput = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();

                drawHoughResult(houghOnMaskedInput, circles);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);

                cv::imshow("black - hough on masked input", houghOnMaskedInput);
                cv::imshow("black - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
                cv::imshow("black - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
            }
        }

        {
            std::vector<cv::Vec3f> circles = whitePinkCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, whitePinkMask, true);

            // Filter circles by saturated ball mask in order to remove 'duplicate' balls
            std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, saturatedBallMask, false);

            for (auto& circle : filteredCircles3) allCircles.push_back(circle);

            if (showDebuggingOutput && showWhitePink) {
                cv::Mat houghOnMaskedInput = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter3 = input.clone();

                drawHoughResult(houghOnMaskedInput, whitePinkCircles);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter3, filteredCircles3);

                cv::imshow("white&pink - hough on masked input", houghOnMaskedInput);
                cv::imshow("white&pink - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
                cv::imshow("white&pink - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
                cv::imshow("white&pink - hough on masked input after circle filter 3", houghOnMaskedInputAfterCircleFilter3);
            }
        }

        if (showDebuggingOutput) {
            cv::Mat hough = input.clone();
            drawHoughResult(hough, allCircles);
            cv::imshow("hough", hough);
        }

        int expectedBallCount = expectedBallCounts[imageIndex];
        std::cout << "Detected " << allCircles.size() << " circles, expected " << expectedBallCount << std::endl;

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 'b') {
            showBlack = !showBlack;
            cv::destroyAllWindows();
        } else if (key == 's') {
            showSaturated = !showSaturated;
            cv::destroyAllWindows();
        } else if (key == 'w') {
            showWhitePink = !showWhitePink;
            cv::destroyAllWindows();
        } else if (key == 'o') {
            showDebuggingOutput = !showDebuggingOutput;
            cv::destroyAllWindows();
        } else if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

TEST(BallDetectionTests, latest) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    // Filter for saturated balls
    cv::Point2d ballsSaturationFilter{128, 255};

    // Filter for black ball
    cv::Point2d blackBallValueFilter{0, 60};

    // Filter for white ball
    cv::Point2d whiteBallHueFilter{0, 60};
    cv::Point2d whiteBallSaturationFilter{0, 200};
    cv::Point2d whiteBallValueFilter{200, 255};

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original{126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect{(int) (table_rect_original.x * scale),
                        (int) (table_rect_original.y * scale),
                        (int) (table_rect_original.width * scale),
                        (int) (table_rect_original.height * scale)};

    std::vector<cv::Point2i> balls_original{
            {281,  845}, // WHITE
            {472,  283}, // YELLOW
            {463,  643}, // BLUE
            {153,  588}, // RED 1
            {695,  947}, // RED 2
            {903,  630}, // RED 3
            {952,  842}, // GREEN
            {1095, 426}, // RED 4
            {1440, 551}, // RED 5
            {1420, 592}, // RED 6
            {1468, 588}, // RED 7
            {1448, 630}, // RED 8
            {1392, 774}, // PINK
            {1379, 155}, // RED 9
            {1785, 334}, // RED 10
            {1431, 342} // BLACK
    };
    std::vector<cv::Point2i> balls;
    for (auto& ball : balls_original) {
        balls.push_back(ball * scale);
    }

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius)
              << " max radius: " << std::to_string(maxRadius) << " error radius range: "
              << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));

    for(auto& imagePath : imagePaths) {

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        input = input(table_rect);

        std::vector<cv::Mat> images;

        images.push_back(input);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        images.push_back(blurred);

        std::vector<cv::Vec3f> ballCircles;

        cv::Mat houghOnGrayscale;
        {
            cv::Mat grayscale;
            cv::cvtColor(blurred, grayscale, cv::COLOR_BGR2GRAY);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnGrayscale);
            drawHoughResult(houghOnGrayscale, circles);
        }
        images.push_back(houghOnGrayscale);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat houghOnInput;
        {
            cv::Mat grayscale;
            cv::cvtColor(input, grayscale, cv::COLOR_BGR2GRAY);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnInput);
            drawHoughResult(houghOnInput, circles);
        }
        cv::imshow("houghOnInput", houghOnInput);

        cv::Mat valueMaskByFilteredSaturation;
        cv::Mat filteredValue;
        cv::Mat thresholdedValue;
        {
            cv::Point2d saturationFilter{100, 255};
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, valueMaskByFilteredSaturation);

            cv::bitwise_and(value, value, filteredValue, valueMaskByFilteredSaturation);
            cv::imshow("valueMaskByFilteredSaturation", valueMaskByFilteredSaturation);

            cv::Point2d valueFilter{225, 255};
            cv::inRange(value, valueFilter.x, valueFilter.y, thresholdedValue);
        }
        cv::imshow("filteredValue", filteredValue);
        cv::imshow("thresholdedValue", thresholdedValue);

        cv::Mat redMask;
        {
            cv::Point2d redFilter1{0, 10};
            cv::Point2d redFilter2{170, 180};

            cv::Mat redMask1;
            cv::inRange(hue, redFilter1.x, redFilter1.y, redMask1);
            cv::imshow("redMask1", redMask1);

            cv::Mat redMask2;
            cv::inRange(hue, redFilter2.x, redFilter2.y, redMask2);
            cv::imshow("redMask2", redMask2);

            cv::Mat totalMask;
            cv::bitwise_or(redMask1, redMask2, totalMask);

            cv::Mat openedMask;
            cv::morphologyEx(totalMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 3);
            cv::imshow("openedMask", openedMask);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 3);
            cv::imshow("closedMask", closedMask);

            cv::Mat redSaturation;
            cv::Point2d saturationFilter{100, 255};
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, redSaturation);
            cv::imshow("redSaturation", redSaturation);

            cv::Mat redMaskAndedWithRedSaturation;
            cv::bitwise_and(closedMask, redSaturation, redMaskAndedWithRedSaturation);
            cv::imshow("redMaskAndedWithRedSaturation", redMaskAndedWithRedSaturation);

            cv::Mat redValue;
            cv::Point2d valueFilter{215, 255};
            cv::inRange(value, valueFilter.x, valueFilter.y, redValue);
            cv::imshow("redValue", redValue);

            cv::Mat redMaskAndedWithRedValue;
            cv::bitwise_and(closedMask, redValue, redMaskAndedWithRedValue);
            cv::imshow("redMaskAndedWithRedValue", redMaskAndedWithRedValue);

            redMask = redMaskAndedWithRedValue;
        }
        cv::imshow("redMask", redMask);

        cv::Mat redMaskedInput;
        cv::bitwise_and(input, input, redMaskedInput, redMask);
        cv::imshow("redMaskedInput", redMaskedInput);

        cv::Mat invRedMask;
        cv::bitwise_not(redMask, invRedMask);
        cv::Mat redMaskedDelta;
        cv::bitwise_and(input, input, redMaskedDelta, invRedMask);
        cv::imshow("redMaskedDelta", redMaskedDelta);

        cv::Mat houghOnRedMaskedInput;
    //    cv::Mat redMaskedInputWithoutCirclesFirstRound;
    //    redMaskedInput.copyTo(redMaskedInputWithoutCirclesFirstRound);
        {
            cv::Mat grayscale;
            cv::cvtColor(redMaskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::imshow("redMaskedInput grayscale", grayscale);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);
            cv::imshow("redMaskedInput grayscale opened", opened);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            cv::imshow("redMaskedInput grayscale edges", edges);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnRedMaskedInput);
            drawHoughResult(houghOnRedMaskedInput, circles);

    //        drawCircles(redMaskedInputWithoutCirclesFirstRound, circles, cv::Scalar(0));
        }
        cv::imshow("houghOnRedMaskedInput", houghOnRedMaskedInput);
    //    cv::imshow("redMaskedInputWithoutCirclesFirstRound", redMaskedInputWithoutCirclesFirstRound);

    //    cv::Mat houghOnRedMaskedInputSecondRound;
        {
    //        cv::Mat grayscale;
    //        cv::cvtColor(redMaskedInputWithoutCirclesFirstRound, grayscale, cv::COLOR_BGR2GRAY);
    //
    //        cv::imshow("redMaskedInputWithoutCirclesFirstRound grayscale", grayscale);
    //
    //        std::vector<cv::Vec3f> circles;
    //        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
    //                     houghMinDistance, // minimal distance between centers
    //                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    //                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    //                     minRadius, maxRadius
    //        );
    //
    //        input.copyTo(houghOnRedMaskedInputSecondRound);
    //        drawHoughResult(houghOnRedMaskedInputSecondRound, circles);
        }
    //    cv::imshow("houghOnRedMaskedInputSecondRound", houghOnRedMaskedInputSecondRound);

    //    cv::Mat filteredValueWithoutCirclesFirstRound;
    //    filteredValue.copyTo(filteredValueWithoutCirclesFirstRound);

        cv::Mat houghOnFilteredValue;
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(filteredValue, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius / 2, maxRadius
            );

            input.copyTo(houghOnFilteredValue);
            drawHoughResult(houghOnFilteredValue, circles);

    //        drawCircles(filteredValueWithoutCirclesFirstRound, circles, cv::Scalar(0));
        }
        cv::imshow("houghOnFilteredValue", houghOnFilteredValue);
    //    cv::imshow("filteredValueWithoutCirclesFirstRound", filteredValueWithoutCirclesFirstRound);

    //    cv::Mat houghOnFilteredValueSecondRound;
        {
    //        std::vector<cv::Vec3f> circles;
    //        HoughCircles(filteredValueWithoutCirclesFirstRound, circles, cv::HOUGH_GRADIENT, 1,
    //                     houghMinDistance, // minimal distance between centers
    //                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    //                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    //                     minRadius, maxRadius
    //        );
    //
    //        input.copyTo(houghOnFilteredValueSecondRound);
    //        drawHoughResult(houghOnFilteredValueSecondRound, circles);
        }
    //    cv::imshow("houghOnFilteredValueSecondRound", houghOnFilteredValueSecondRound);

        cv::Mat closedSaturation;
        cv::morphologyEx(saturation, closedSaturation, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 3);
        cv::imshow("closedSaturation", closedSaturation);

        cv::Mat houghOnSaturation;
    //    cv::Mat saturationWithoutCirclesFirstRound;
    //    saturation.copyTo(saturationWithoutCirclesFirstRound);
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(closedSaturation, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnSaturation);
            drawHoughResult(houghOnSaturation, circles);

    //        drawCircles(saturationWithoutCirclesFirstRound, circles, cv::Scalar(0));
        }
        cv::imshow("houghOnSaturation", houghOnSaturation);
    //    cv::imshow("saturationWithoutCirclesFirstRound", saturationWithoutCirclesFirstRound);

    //    cv::Mat houghOnSaturationSecondRound;
        {
    //        std::vector<cv::Vec3f> circles;
    //        HoughCircles(saturationWithoutCirclesFirstRound, circles, cv::HOUGH_GRADIENT, 1,
    //                     houghMinDistance, // minimal distance between centers
    //                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    //                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    //                     minRadius, maxRadius
    //        );
    //
    //        input.copyTo(houghOnSaturationSecondRound);
    //        drawHoughResult(houghOnSaturationSecondRound, circles);
        }
    //    cv::imshow("houghOnSaturationSecondRound", houghOnSaturationSecondRound);

        cv::Mat houghOnValue;
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(value, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnValue);
            drawHoughResult(houghOnValue, circles);
        }
        cv::imshow("houghOnValue", houghOnValue);

        cv::Mat whiteBallMask;
        cv::Mat whiteBallSaturation;
        {
            cv::Mat hueMask;
            cv::inRange(hue, whiteBallHueFilter.x, whiteBallHueFilter.y, hueMask);

            cv::Mat saturationMask;
            cv::inRange(saturation, whiteBallSaturationFilter.x, whiteBallSaturationFilter.y, saturationMask);

            cv::Mat valueMask;
            cv::inRange(value, whiteBallValueFilter.x, whiteBallValueFilter.y, valueMask);

            cv::Mat totalMask;
            cv::bitwise_and(hueMask, saturationMask, totalMask);
            cv::bitwise_and(totalMask, valueMask, totalMask);

            cv::Mat totalMaskOpened;
            cv::morphologyEx(totalMask, totalMaskOpened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 3);

            cv::Mat totalMaskDilated;
            cv::morphologyEx(totalMaskOpened, totalMaskDilated, cv::MORPH_DILATE, elementRect3x3, cv::Point(-1, -1), 1);

            whiteBallMask = totalMaskOpened;
            cv::imshow("whiteBallMask", whiteBallMask);

            cv::bitwise_and(saturation, whiteBallMask, whiteBallSaturation);
            cv::imshow("whiteBallSaturation", whiteBallSaturation);
        }

        cv::Mat blackBallMaskClosed;
        {
            cv::Mat blackBallMask;
            cv::inRange(value, blackBallValueFilter.x, blackBallValueFilter.y, blackBallMask);
            cv::imshow("blackBallMask", blackBallMask);

            cv::morphologyEx(blackBallMask, blackBallMaskClosed, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 3);
            cv::imshow("blackBallMaskClosed", blackBallMaskClosed);
        }

        cv::Mat houghOnWhiteBallSaturation;
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(whiteBallSaturation, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnWhiteBallSaturation);
            drawHoughResult(houghOnWhiteBallSaturation, circles);
            for (auto& circle : circles) ballCircles.push_back(circle);
        }
        cv::imshow("houghOnWhiteBallSaturation", houghOnWhiteBallSaturation);

        cv::Mat houghOnBlackBallMask;
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(blackBallMaskClosed, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnBlackBallMask);
            drawHoughResult(houghOnBlackBallMask, circles);
            for (auto& circle : circles) ballCircles.push_back(circle);
        }
        cv::imshow("houghOnBlackBallMask", houghOnBlackBallMask);

        // Saturated balls mask
        cv::Mat saturatedBallsMask;
        cv::inRange(saturation, ballsSaturationFilter.x, ballsSaturationFilter.y, saturatedBallsMask);
        cv::imshow("saturatedBallsMask", saturatedBallsMask);

        cv::Mat saturatedBallsMaskClosed;
        cv::morphologyEx(saturatedBallsMask, saturatedBallsMaskClosed, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);
        cv::imshow("saturatedBallsMaskClosed", saturatedBallsMaskClosed);

        cv::Mat saturatedBallsMaskOpened;
        cv::morphologyEx(saturatedBallsMaskClosed, saturatedBallsMaskOpened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1),2);
        cv::imshow("saturatedBallsMaskOpened", saturatedBallsMaskOpened);

        cv::Mat saturatedBallsMaskDilated;
        cv::morphologyEx(saturatedBallsMaskOpened, saturatedBallsMaskDilated, cv::MORPH_DILATE, elementRect3x3, cv::Point(-1, -1), 1);
        cv::imshow("saturatedBallsMaskDilated", saturatedBallsMaskDilated);

        cv::Mat filteredSaturation;
        cv::bitwise_and(saturation, saturation, filteredSaturation, saturatedBallsMaskDilated);
        cv::imshow("filteredSaturation", filteredSaturation);

        cv::Mat saturationDiff;
        cv::subtract(saturation, filteredSaturation, saturationDiff);
        cv::imshow("saturationDiff", saturationDiff);

        cv::Mat houghOnFilteredSaturation;
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(filteredSaturation, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnFilteredSaturation);
            drawHoughResult(houghOnFilteredSaturation, circles);
            for (auto& circle : circles) ballCircles.push_back(circle);
        }
        cv::imshow("houghOnFilteredSaturation", houghOnFilteredSaturation);

        double totalError = 0.0;
        int n = 0;
        int ballIndex = 0;
        std::vector<cv::Mat> ballImages;
        int discarded_by_table_rect = 0;

        cv::Mat houghResult;
        input.copyTo(houghResult);

        for (auto& c : ballCircles) {
            auto center = cv::Point2i{(int) c[0], (int) c[1]};
            if (!table_rect.contains(center)) {
                discarded_by_table_rect++;
                continue;
            }

            cv::Point2i closest = findClosestPoint(center, balls);
            double dist = (1 / scale) * cv::abs(cv::norm(center - closest));
            cv::Scalar outlineColor{255, 0, 255};
            if (dist < 5.0) {
                std::cout << "Center at " << center.x << "," << center.y << " should be at " << closest.x << ","
                          << closest.y << " dist: " << dist << std::endl;
                outlineColor = {0, 255, 0};
                totalError += dist;
                n++;
            } else if (dist < 10.0) {
                std::cout << "Center at " << center.x << "," << center.y << " should be at " << closest.x << ","
                          << closest.y << " dist: " << dist << std::endl;
                outlineColor = {255, 255, 0};
                totalError += dist;
                n++;
            }
            uint8_t radius = c[2];
            cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
            if (roi.x >= 0 && roi.y >= 0 && roi.width <= houghResult.cols && roi.height <= houghResult.rows) {

                // circle center
                cv::circle(houghResult, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
                // circle outline
                cv::circle(houghResult, center, radius, outlineColor, 1, cv::LINE_AA);

                if (dist < 10.0) {
                    int padding = 10;
                    cv::Mat ball = houghResult(
                            cv::Rect{roi.x - padding, roi.y - padding, roi.width + 2 * padding, roi.height + 2 * padding});
                    cv::Mat ballResized;
                    cv::resize(ball, ballResized, cv::Size(0, 0), 4.0, 4.0);
                    std::stringstream ballTitle;
                    ballTitle << "Ball " << std::to_string(ballIndex);
    //                    cv::imshow(ballTitle.str(), ballResized);
                    ballIndex++;
                    ballImages.push_back(ballResized);
                }
            }
        }
        if (n > 0) {
            double averageError = totalError / n;
            std::cout << "average error: " << std::to_string(averageError) << " given " << n << " samples"
                      << ", discarded by table rect: " << discarded_by_table_rect << std::endl;

            imageGallery("Balls", ballImages);
        }
        cv::imshow("houghResult", houghResult);

        imageGallery("Image", images);

        cv::waitKey();
    }
}
