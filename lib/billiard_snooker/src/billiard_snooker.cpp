#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_debug/billiard_debug.hpp>
#include <billiard_physics/billiard_physics.hpp>
#include <algorithm>
#include <optional>
#include <vector>
#include <ostream>

#ifndef BILLIARD_SNOOKER_DEBUG_PRINT
    #ifdef NDEBUG
        #undef BILLIARD_SNOOKER_DEBUG_PRINT
    #endif
    #ifndef NDEBUG
        #define BILLIARD_SNOOKER_DEBUG_PRINT 1
    #endif
#endif

#ifndef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
    #ifdef NDEBUG
        #undef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
    #endif
    #ifndef NDEBUG
        #define BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL 1
    #endif
#endif

#ifndef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
    #ifdef NDEBUG
        #undef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
    #endif
    #ifndef NDEBUG
        #define BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT 1
    #endif
#endif

// TODO: remove this
//#undef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
//#define BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL 1
#undef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
//#define BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT 1
#define BILLIARD_SNOOKER_CLASSIFICATION_CLUSTERS 1 // TODO: make this the default?

// TODO: remove this
//#define BILLIARD_SNOOKER_TIMING 1

#ifndef BILLIARD_SNOOKER_TIMING
    #ifdef NDEBUG
        #undef BILLIARD_SNOOKER_TIMING
    #endif
    #ifndef NDEBUG
        #define BILLIARD_SNOOKER_TIMING 1
    #endif
#endif

#ifdef BILLIARD_SNOOKER_TIMING
    std::chrono::time_point<std::chrono::high_resolution_clock> start;

    void startTimer() {
        start = std::chrono::high_resolution_clock::now();
    }

    void stopTimer(const std::string& agent) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms_double = end - start;
        double totalTimeMs = ms_double.count();
        std::cout << agent << " took: " << totalTimeMs << "ms" << std::endl;
    }

    #define tick()               \
        do {                     \
            startTimer();        \
        } while(0);

    #define tock(agent)          \
        do {                     \
            stopTimer(agent);    \
        } while(0);
#else
    #define tick() do {} while(0);
    #define tock(agent) do {} while(0);
#endif

namespace billiard::snooker {

    SnookerDetectionConfig config;
    SnookerSearchConfig _searchConfig;
    SnookerClassificationConfig classificationConfig {};

    bool configure(const billiard::detection::DetectionConfig& detectionConfig) {

        if (!detectionConfig.valid) {
            return false;
        }

        int radiusInPixel = detectionConfig.ballRadiusInPixel;
        int errorRadiusLow = ceil(radiusInPixel * (config.radiusErrorLow / 100.0));
        int errorRadiusHigh = ceil(radiusInPixel * (config.radiusErrorHigh / 100.0));
        config.minRadius = radiusInPixel - errorRadiusLow;
        config.maxRadius = radiusInPixel + errorRadiusHigh;
        config.houghMinDistance = config.minRadius * 2;

#ifdef BILLIARD_SNOOKER_DEBUG_PRINT
        std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(config.minRadius) << " max radius: " << std::to_string(config.maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;
#endif
        config.morphElementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));
        config.innerTableMask = detectionConfig.innerTableMask;
        config.railMask = detectionConfig.railMask;

        config.valid = true;

        classificationConfig.roiRadius = detectionConfig.ballRadiusInPixel * classificationConfig.roiRadiusFactor;
        classificationConfig.valid = true;
        return true;
    }

    void searchConfig(const SnookerSearchConfig& searchConfig) {
        _searchConfig = searchConfig;
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

    inline std::vector<cv::Point2d> circleCenters(const std::vector<cv::Vec3f>& circles) {
        std::vector<cv::Point2d> imagePoints;
        for (auto& circle : circles) {
            imagePoints.emplace_back(circle[0], circle[1]);
        }
        return imagePoints;
    }


    void drawHoughResult(cv::Mat& image, std::vector<cv::Vec3f>& circles, int radius = 0) {
        for(auto c : circles) {
            cv::Point center = cv::Point(c[0], c[1]);
            int R = radius > 0 ? radius : c[2];
            cv::Rect roi(center.x - R, center.y - R, R * 2, R * 2);
            if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

                // circle center
                cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
                // circle outline
                cv::circle(image, center, R, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
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

#ifdef BILLIARD_SNOOKER_DEBUG_PRINT
        std::cout
                << "Ball tiles: " << " "
                << "totalTiles=" << totalTiles << " "
                << "tileSize=" << tileSize << " "
                << "tilesPerLine=" << tilesPerLine << " "
                << "numberOfTileLines=" << numberOfTileLines << " "
                << "imageWidth=" << imageWidth << " "
                << "imageHeight=" << imageHeight << " "
                << std::endl;
#endif

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

#ifdef BILLIARD_SNOOKER_DEBUG_PRINT
                std::cout
                        << "    Ball tile: " << " "
                        << "colIndex=" << colIndex << " "
                        << "rowIndex=" << rowIndex << " "
                        << "x=" << x << " "
                        << "y=" << y << " "
                        << std::endl;
#endif
            } else {
                std::cout << "unable to cut out ball image since roi is not inside image" << std::endl;
            }
            tileIndex++;
        }
        return result;
    }

    billiard::detection::State detect(const billiard::detection::State& previousState, const cv::Mat& original) {

        tick();

        cv::Mat input;
        cv::resize(original, input, cv::Size(), config.scale, config.scale);

        // Apply blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat grayscaleInput;
        cv::cvtColor(blurred, grayscaleInput, cv::COLOR_BGR2GRAY);

        // Convert image into HSV and retrieve separate channels
        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);
#endif

        tock("Input processing");
        tick();

        // Saturated balls mask
        cv::Mat saturatedBallMask;
        {
            cv::Mat saturationMask;
            cv::inRange(saturation, config.saturationFilter.x, config.saturationFilter.y, saturationMask);

            cv::Mat valueMask;
            cv::inRange(value, config.saturatedBallsValueFilter.x, config.saturatedBallsValueFilter.y, valueMask);

            cv::Mat saturationMaskAndedWithValueMask;
            cv::bitwise_and(valueMask, saturationMask, saturationMaskAndedWithValueMask);

            cv::Mat saturationMaskAndedWithRailMask;
            cv::bitwise_and(config.railMask, saturationMaskAndedWithValueMask, saturationMaskAndedWithRailMask);

            cv::Mat openedSaturationMask;
            cv::morphologyEx(saturationMaskAndedWithRailMask, openedSaturationMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1),
                             1);

            cv::Mat closedSaturationMask;
            cv::morphologyEx(openedSaturationMask, closedSaturationMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 4);

            saturatedBallMask = closedSaturationMask;

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
            cv::imshow("saturationMask", saturationMask);
            cv::imshow("saturated balls valueMask", valueMask);
            cv::imshow("saturationMask AND valueMask", saturationMaskAndedWithValueMask);
            cv::imshow("saturationMask AND railMask", saturationMaskAndedWithRailMask);
            cv::imshow("openedSaturationMask", openedSaturationMask);
            cv::imshow("closedSaturationMask", closedSaturationMask);
            cv::imshow("saturatedBallMask", saturatedBallMask);
#endif
        }

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::Mat saturationMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, saturationMaskedGrayscale, saturatedBallMask);

        cv::Mat saturationMaskedInput;
        cv::bitwise_and(input, input, saturationMaskedInput, saturatedBallMask);

        cv::Mat invMask;
        cv::bitwise_not(saturatedBallMask, invMask);
        cv::Mat maskedDelta;
        cv::bitwise_and(input, input, maskedDelta, invMask);

        cv::imshow("saturationMask - masked color", saturationMaskedInput);
        cv::imshow("saturationMask - masked grayscale", saturationMaskedGrayscale);
        cv::imshow("saturationMask - masked delta", maskedDelta);
#endif

        tock("Building saturation mask");
        tick();

        // Black ball mask
        cv::Mat blackMask;
        {
            cv::Mat valueMask;
            cv::inRange(value, config.blackValueFilter.x, config.blackValueFilter.y, valueMask);

            // TODO: remove?
            cv::Mat valueMaskAndedWithTableMask;
            cv::bitwise_and(valueMask, config.innerTableMask, valueMaskAndedWithTableMask);

            cv::Mat valueMaskAndedWithRailMask;
            cv::bitwise_and(valueMask, config.railMask, valueMaskAndedWithRailMask);

            cv::Mat closedValueMask;
            cv::morphologyEx(valueMaskAndedWithRailMask, closedValueMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 6);

            cv::Mat openedValueMask;
            cv::morphologyEx(closedValueMask, openedValueMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1),3);

            blackMask = openedValueMask;

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
            cv::imshow("black - valueMask", valueMask);
            cv::imshow("black - valueMask anded with innerTableMask", valueMaskAndedWithTableMask);
            cv::imshow("black - valueMask anded with railMask", valueMaskAndedWithRailMask);
            cv::imshow("black - closedValueMask", closedValueMask);
            cv::imshow("black - openedValueMask", openedValueMask);
            cv::imshow("black - mask", blackMask);
#endif

        }

        cv::Mat blackMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, blackMaskedGrayscale, blackMask);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        {
            cv::Mat blackMaskedInput;
            cv::bitwise_and(input, input, blackMaskedInput, blackMask);
            cv::Mat invMask;
            cv::bitwise_not(blackMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            cv::imshow("black - masked color", blackMaskedInput);
            cv::imshow("black - masked grayscale", blackMaskedGrayscale);
            cv::imshow("black - masked delta", maskedDelta);
        };
#endif

        tock("Building black mask");
        tick();

        // White & Pink Mask
        cv::Mat whitePinkMask;
        {
            cv::Mat valueMask;
            cv::inRange(value, config.whitePinkValueFilter.x, config.whitePinkValueFilter.y, valueMask);

            cv::Mat valueMaskAndedWithRailMask;
            cv::bitwise_and(config.railMask, valueMask, valueMaskAndedWithRailMask);

            cv::Mat openedMask;
            cv::morphologyEx(valueMaskAndedWithRailMask, openedMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1), 3);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, config.morphElementRect3x3, cv::Point(-1, -1), 1);

            whitePinkMask = closedMask;

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
            cv::imshow("white&pink - valueMask", valueMask);
            cv::imshow("white&pink - valueMask anded with railMask", valueMaskAndedWithRailMask);
            cv::imshow("white&pink - openedMask", openedMask);
            cv::imshow("white&pink - closedMask", closedMask);
            cv::imshow("white&pink - mask", whitePinkMask);
#endif
        }

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        {
            cv::Mat whitePinkMaskedGrayscale;
            cv::bitwise_and(grayscaleInput, grayscaleInput, whitePinkMaskedGrayscale, whitePinkMask);

            cv::Mat closedWhitePinkMaskedGrayscale;
            cv::morphologyEx(whitePinkMaskedGrayscale, closedWhitePinkMaskedGrayscale, cv::MORPH_CLOSE, config.morphElementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat whitePinkMaskedInput;
            cv::bitwise_and(input, input, whitePinkMaskedInput, whitePinkMask);

            cv::Mat invMask;
            cv::bitwise_not(whitePinkMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            cv::imshow("white&pink - masked color", whitePinkMaskedInput);
            cv::imshow("white&pink - masked grayscale", whitePinkMaskedGrayscale);
            cv::imshow("white&pink - masked and closed grayscale", closedWhitePinkMaskedGrayscale);
            cv::imshow("white&pink - masked delta", maskedDelta);
        }
#endif

        tock("Building white&pink mask");
        tick();

        cv::Mat saturationAndWhitePinkMask;
        cv::bitwise_or(saturatedBallMask, whitePinkMask, saturationAndWhitePinkMask);

        cv::Mat saturationAndWhitePinkMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, saturationAndWhitePinkMaskedGrayscale, saturationAndWhitePinkMask);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::imshow("saturationAndWhitePinkMask", saturationAndWhitePinkMask);
        cv::imshow("saturationAndWhitePinkMaskedGrayscale", saturationAndWhitePinkMaskedGrayscale);
#endif

        // Hough on saturation and white&pink masked input
        std::vector<cv::Vec3f> saturationAndWhitePinkCircles;
        HoughCircles(saturationAndWhitePinkMaskedGrayscale, saturationAndWhitePinkCircles, config.houghMethod, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::Mat edges;
        cv::Canny(saturationAndWhitePinkMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);

        cv::imshow("saturationAndWhitePinkMask - maskedInput grayscale edges", edges);
#endif

        // Hough on black masked input
        std::vector<cv::Vec3f> blackCircles;
        HoughCircles(blackMaskedGrayscale, blackCircles, config.houghMethod, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        {
            cv::Mat edges;
            cv::Canny(blackMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);

            cv::imshow("black - maskedInput grayscale edges", edges);
        };
#endif

        tock("hough");
        tick();

        std::vector<cv::Vec3f> allCircles;

        {
            std::vector<cv::Vec3f> circles = saturationAndWhitePinkCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.railMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, saturationAndWhitePinkMask, true);
            std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, blackMask, false);
            for (auto& circle : filteredCircles3) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
            {
                cv::Mat houghOnMaskedInput = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter3 = input.clone();

                drawHoughResult(houghOnMaskedInput, circles);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter3, filteredCircles3);

                cv::imshow("saturated&whitepink - hough on masked input", houghOnMaskedInput);
                cv::imshow("saturated&whitepink - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
                cv::imshow("saturated&whitepink - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
                cv::imshow("saturated&whitepink - hough on masked input after circle filter 3", houghOnMaskedInputAfterCircleFilter3);
            }
#endif
        }

        {
            std::vector<cv::Vec3f> circles = blackCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.innerTableMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, blackMask, true);
            for (auto& circle : filteredCircles2) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
            cv::Mat houghOnMaskedInput = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();

            drawHoughResult(houghOnMaskedInput, circles);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);

            cv::imshow("black - hough on masked input", houghOnMaskedInput);
            cv::imshow("black - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
            cv::imshow("black - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
#endif
        }

        tock("filtering circles");
        tick();

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::Mat hough = input.clone();
        drawHoughResult(hough, allCircles);
        cv::imshow("hough", hough);
#endif

        std::vector<cv::Point2d> imagePoints = circleCenters(allCircles);

        billiard::detection::State state;
        for (int i = 0; i < imagePoints.size(); i++) {
            auto& point = imagePoints[i];
            billiard::detection::Ball ball;
            ball._type = "UNKNOWN";
            ball._id = std::to_string(i);
            ball._position = glm::vec2 {point.x, point.y};
            ball._trackingCount = 0;
            state._balls.push_back(ball);
        }

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::Mat detectedBalls = drawDetectedBallsGrid(hough, state, 128, 8);
        cv::imshow("detected balls", detectedBalls);
        cv::waitKey(1);
#endif

        tock("preparing state");

        return state;
    }

    /**
     * Based on https://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html#code
     */
    std::vector<Histogram> histogram(const cv::Mat& input) {
        std::vector<cv::Mat> planes;
        cv::split(input, planes);

        int histSize = 256;
        float range[] = { 0, 256 } ;
        const float* histRange = { range };
        bool uniform = true;
        bool accumulate = false;

        std::vector<Histogram> histograms;
        for (const auto& plane : planes) {
            cv::Mat histogramPlane;
            cv::calcHist(&plane, 1, nullptr, cv::Mat(), histogramPlane, 1, &histSize, &histRange, uniform, accumulate);

            double min, max;
            cv::Point minLocation, maxLocation;
            cv::minMaxLoc(histogramPlane, &min, &max, &minLocation, &maxLocation);

            histograms.emplace_back(Histogram{histogramPlane, maxLocation});
        }

        return histograms;
    }


    /**
     * Based on https://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html#code
     */
    void showHistograms(const std::vector<Histogram>& histograms) {
        int bins = 256;
        int hist_w = 512;
        int hist_h = 400;
        int bin_w = cvRound( (double) hist_w/bins );

        cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

        for (int i = 0; i < histograms.size(); i++) {
            auto histogram = histograms[i].histogram;
            cv::normalize(histogram, histogram, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

            int blue = i == 0 || i > 2 ? 255 : 0;
            int green = i == 1 || i > 2 ? 255 : 0;
            int red = i == 2 || i > 2 ? 255 : 0;

            for (int bin = 1; bin < bins; bin++) {
                cv::line(histImage, cv::Point(bin_w * (bin - 1), hist_h - cvRound(histogram.at<float>(bin - 1))),
                         cv::Point(bin_w * (bin), hist_h - cvRound(histogram.at<float>(bin))),
                         cv::Scalar(blue, green, red), 2, 8, 0);
            }
        }

        std::cout << "Max Hue" << ": " << std::setfill(' ') << std::setw(3) << histograms[0].maxLocation.y << " "
                    << "Max Saturation" << ": " << std::setfill(' ') << std::setw(3) << histograms[1].maxLocation.y << " "
                    << "Max Value" << ": " << std::setfill(' ') << std::setw(3) << histograms[2].maxLocation.y
                    << std::endl;

        cv::imshow("Histogram", histImage);
    }

    void classify(const billiard::detection::State& previousState,
                  billiard::detection::State& currentState,
                  const cv::Mat& image) {

        double radius = classificationConfig.roiRadius;

        for (auto& ball : currentState._balls) {

            const glm::vec2& pixelPosition = ball._position;
            const cv::Rect& roi = cv::Rect{
                    (int) (pixelPosition.x - radius),
                    (int) (pixelPosition.y - radius),
                    (int) (2 * radius),
                    (int) (2 * radius)
            };
            cv::Mat ballBgr = image(roi);

            classify(ball, ballBgr);
        }

    }

    void classify(billiard::detection::Ball& ball, const cv::Mat& original) {

        cv::Mat blurred;
        cv::GaussianBlur(original, blurred, classificationConfig.blurSize, 0, 0);

        cv::Mat hsv;
        cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

        auto histogramByChannels = histogram(hsv);

        Histogram hueHist        = histogramByChannels[0];
        Histogram saturationHist = histogramByChannels[1];
        Histogram valueHist      = histogramByChannels[2];
        int maxHue        = hueHist.maxLocation.y;
        int maxSaturation = saturationHist.maxLocation.y;
        int maxValue      = valueHist.maxLocation.y;

        std::string label = "UNKNOWN";

        // TODO: remove uncommented code in this function

        const auto between = [](int value, cv::Point2d minMax)  {
            return value >= minMax.x && value <= minMax.y;
        };

#ifdef BILLIARD_SNOOKER_CLASSIFICATION_CLUSTERS

        std::vector<std::string> potentialLabels;

        if (between(maxHue, classificationConfig.yellowHue) && between(maxSaturation, classificationConfig.yellowSaturation) && between(maxValue, classificationConfig.yellowValue)) {
            potentialLabels.emplace_back("YELLOW");
        }
        if (between(maxHue, classificationConfig.whiteHue) && between(maxSaturation, classificationConfig.whiteSaturation) && between(maxValue, classificationConfig.whiteValue)) {
            potentialLabels.emplace_back("WHITE");
        }
        if (between(maxValue, classificationConfig.blackValue)) {
            potentialLabels.emplace_back("BLACK");
        }
        if (between(maxHue, classificationConfig.blueHue)) {
            potentialLabels.emplace_back("BLUE");
        }
        if (between(maxHue, classificationConfig.greenHue)) {
            potentialLabels.emplace_back("GREEN");
        }
        if (between(maxHue, classificationConfig.brownHue) && between(maxSaturation, classificationConfig.brownSaturation) && between(maxValue, classificationConfig.brownValue)) {
            potentialLabels.emplace_back("BROWN");
        }
        if ((between(maxHue, classificationConfig.redHue1) || between(maxHue, classificationConfig.redHue2)) && between(maxSaturation, classificationConfig.pinkSaturation) && between(maxValue, classificationConfig.pinkValue)) {
            potentialLabels.emplace_back("PINK");
        }
        if(between(maxHue, classificationConfig.redHue1) || between(maxHue, classificationConfig.redHue2)) {
            potentialLabels.emplace_back("RED");
        }

        if (potentialLabels.empty()) {
            label = "UNKNOWN";
        } else if (potentialLabels.size() == 1) {
            label = potentialLabels[0];
        } else {

            // TODO: max distance to a cluster, as to classify as UNKNOWN if too far away
            float minDistanceSquared = 100000.0f;
            for (auto& cluster : classificationConfig.clusters) {

                if (std::find(potentialLabels.begin(), potentialLabels.end(), cluster.type) == potentialLabels.end()) {
                    continue;
                }

                glm::vec3 dataPoint {maxHue, maxSaturation, maxValue};
                glm::vec3& clusterPoint = cluster.point;
                glm::vec3 delta = dataPoint - clusterPoint;

                float distanceSquared = glm::dot(delta, delta);
                if (distanceSquared < minDistanceSquared) {
                    label = cluster.type;
                    minDistanceSquared = distanceSquared;
                }
            }
        }

#else

//        cv::Vec2f redPinkSeparatorLine = cv::Point2f { 1.0, 1.0 }; // Separate by (saturation, value)

        if (between(maxHue, classificationConfig.yellowHue) && between(maxSaturation, classificationConfig.yellowSaturation) && between(maxValue, classificationConfig.yellowValue)) {
            label = "YELLOW";
        }
        else if (between(maxHue, classificationConfig.whiteHue) && between(maxSaturation, classificationConfig.whiteSaturation) && between(maxValue, classificationConfig.whiteValue)) {
            label = "WHITE";
        }
        else if (between(maxValue, classificationConfig.blackValue)) {
            label = "BLACK";
        }
        else if (between(maxHue, classificationConfig.blueHue)) {
            label = "BLUE";
        }
        else if (between(maxHue, classificationConfig.greenHue)) {
            label = "GREEN";
        }
        else if(between(maxHue, classificationConfig.redHue1) || between(maxHue, classificationConfig.redHue2)) {
            // BROWN or RED or PINK

            if (between(maxHue, classificationConfig.brownHue) && between(maxSaturation, classificationConfig.brownSaturation) && between(maxValue, classificationConfig.brownValue)) {
                label = "BROWN";
            }
            else if (between(maxSaturation, classificationConfig.pinkSaturation) && between(maxValue, classificationConfig.pinkValue)) {
                label = "PINK";
            }
            else {
                label = "RED";
            }
        }
#endif

        ball._type = label;

#ifdef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
        std::cout << "Ball " << ball._id << " at (" << ball._position.x << ", " << ball._position.y << ")"
                  << " classified as " << label << " ";

        std::cout << "potential classes: [";
        for(auto& label : potentialLabels) {
            std::cout << label << " ";
        }
        std::cout << "]";
        std::cout << std::endl;

        cv::Mat rgbDebug {original.rows + 100, 3*original.cols + 100, CV_8UC3, cv::Scalar{120, 120, 120}};
        original.copyTo(rgbDebug(cv::Rect{cv::Point(0, 0), cv::Size{original.cols, original.rows}}));
        blurred.copyTo(rgbDebug(cv::Rect {cv::Point(original.cols, 0), cv::Size {blurred.cols, blurred.rows}}));
        cv::imshow("original, blurred", rgbDebug);

        cv::Mat hue, saturation, value;
        {
            std::vector<cv::Mat> channels;
            cv::split(hsv, channels);
            hue = channels[0];
            saturation = channels[1];
            value = channels[2];
        }

        cv::Mat hsvDebug {hue.rows + 100, 3*hue.cols + 100, CV_8UC1, cv::Scalar{120, 120, 120}};
        hue.copyTo(hsvDebug(cv::Rect{cv::Point(0, 0), cv::Size{hue.cols, hue.rows}}));
        saturation.copyTo(hsvDebug(cv::Rect {cv::Point(hsv.cols, 0), cv::Size {saturation.cols, saturation.rows}}));
        value.copyTo(hsvDebug(cv::Rect {cv::Point(2*hsv.cols, 0), cv::Size {value.cols, value.rows}}));
        cv::imshow("hsv", hsvDebug);

        showHistograms(histogramByChannels);

//        cv::waitKey();
#endif

    }

#define MAX_POINTS 7.0
    std::unordered_map<std::string, int> pointsPerColor({
                                                                {"RED", 1},
                                                                {"YELLOW", 2},
                                                                {"GREEN", 3},
                                                                {"BROWN", 4},
                                                                {"BLUE", 5},
                                                                {"PINK", 6},
                                                                {"BLACK", 7}
                                                        });

    std::unordered_map<std::string, std::string> nextSearchType({
                                                                {"RED", "YELLOW"},
                                                                {"YELLOW", "GREEN"},
                                                                {"GREEN", "BROWN"},
                                                                {"BROWN", "BLUE"},
                                                                {"BLUE", "PINK"},
                                                                {"PINK", "BLACK"},
                                                                {"BLACK", ""}
                                                        });
    std::vector<std::string> typesToReplace({
        "YELLOW",
        "GREEN",
        "BROWN",
        "BLUE",
        "PINK",
        "BLACK"
    });

    int pointsForPottedBall(const std::string& ballType) {
        if (pointsPerColor.count(ballType)) {
            return pointsPerColor.at(ballType);
        }
        return 0;
    }

    double scoreForPottedBall(const std::string& ballType) {
        // Number between 0 and 1
        return ((double) pointsForPottedBall(ballType)) / MAX_POINTS;
    }

    billiard::search::Search nextSearch(const billiard::search::State& state,
                                        const std::vector<std::string>& previousTypes) {
        static std::string agent = "[nextSearch] ";

        uint8_t reds = 0;
        for (auto& ball : state._balls) {
            reds += ball._type == "RED" ? 1 : 0;
        }

        if (std::count(previousTypes.begin(), previousTypes.end(), "RED")) { // 1. Red was searched
            if (reds > 0) { // 1.1 It has more reds in layer -> search by all colors
                DEBUG(agent << "prev was red, search by all colors" << std::endl);
                return billiard::search::Search{"", std::vector<std::string>{"BLACK", "PINK", "BLUE", "BROWN", "GREEN", "YELLOW"}};
            } else { // 1.2 It has no more reds in layer -> search yellow
                DEBUG(agent << "prev was red, search by yellow" << std::endl);
                return billiard::search::Search{"", std::vector<std::string>{nextSearchType[previousTypes.at(0)]}};
            }
        } else { // 2. Color was searched
            if (reds > 0) {// 1.1 It has red/s in layer -> search by red
                DEBUG(agent << "prev was color, search by red" << std::endl);
                return billiard::search::Search{"", std::vector<std::string>{"RED"}};
            } else { // 1.2 It has no red in layer -> search by next higher color
                DEBUG(agent << "prev was color, search next color: " << nextSearchType[previousTypes.at(0)] << std::endl);
                return billiard::search::Search{"", std::vector<std::string>{nextSearchType[previousTypes.at(0)]}};
            }
        }
    }

    billiard::search::Ball createBall(const glm::vec2& position, const std::string& type, const std::string& id) {

        return billiard::search::Ball {
            position,
            type,
            id
        };
    }

    enum ReplacePositionAction {
        EXPAND_1,
        EXPAND_2,
        EXPAND_3,
        EXPAND_4,
        NONE
    };

    std::unordered_map<ReplacePositionAction, ReplacePositionAction> revertActions {
            {ReplacePositionAction::EXPAND_1, ReplacePositionAction::EXPAND_4},
            {ReplacePositionAction::EXPAND_4, ReplacePositionAction::EXPAND_1},
            {ReplacePositionAction::EXPAND_2, ReplacePositionAction::EXPAND_3},
            {ReplacePositionAction::EXPAND_3, ReplacePositionAction::EXPAND_2}
    };

    struct ReplacePosition {
        glm::vec2 _position;
        ReplacePositionAction _action;

        bool operator==(const ReplacePosition& other) const {
            return _position == other._position;
        }
    };

#ifdef BILLIARD_DEBUG
    std::ostream& operator<<(std::ostream& os, const ReplacePosition& pos){
        os << "ReplacePosition { "
           << "position=(" << pos._position.x << ";" << pos._position.y << " "
           << "action=" << pos._action <<
           " }";
        return os;
    }
#endif

    std::vector<ReplacePosition>
    expandBySpace(const glm::vec2& spot, float searchRadiusSquared, const std::vector<ReplacePosition>& fringe,
                  const glm::vec2& space, const glm::vec2& minimum, const glm::vec2& maximum, const float radius,
                  const ReplacePositionAction& action) {
        static std::string agent = "[expandBySpace] ";
        std::vector<ReplacePosition> newPositions;

        const auto minX = minimum.x + radius;
        const auto minY = minimum.y + radius;
        const auto maxX = maximum.x - radius;
        const auto maxY = maximum.y - radius;

        for (auto& position : fringe) {
            if (position._action == ReplacePositionAction::NONE || action != revertActions.at(position._action)) {
                auto nextPosition = position._position + space;

                if (nextPosition.x < minX ||
                    nextPosition.y < minY ||
                    nextPosition.x > maxX ||
                    nextPosition.y > maxY) {
                    DEBUG(agent << "position (" << nextPosition.x << ";" << nextPosition.y << ") is lower than" << " "
                                << "(" << minX << ";" << minY << ") or greater than" << " "
                                << "(" << maxX << ";" << maxY << ")"
                                << std::endl);
                    continue;
                }

                auto distance = nextPosition - spot;
                auto squaredDistance = glm::dot(distance, distance);

                if (squaredDistance < searchRadiusSquared) {
                    DEBUG(agent << " add possible position: (" << nextPosition.x << ";" << nextPosition.y << ")" << std::endl);
                    newPositions.emplace_back(ReplacePosition{nextPosition, action});
                } else {
                    DEBUG(agent << " position: (" << nextPosition.x << ";" << nextPosition.y << ")" << " "
                                << "is not possible." << " "
                                << "max squared distance: " << searchRadiusSquared << " "
                                << "squared distance: " << squaredDistance
                                << std::endl);
                }
            }
        }

        return newPositions;
    }

    template<class T>
    std::vector<T> operator-(const std::vector<T>& from, const std::vector<T>& subtract) {
        static std::string agent = "[vector::operator::-] ";

        std::vector<T> subtracted = from;
        auto iterator = subtracted.begin();
        while (iterator != subtracted.end()) {
            if (std::find(subtract.begin(), subtract.end(), *iterator) != subtract.end()) {
                iterator = subtracted.erase(iterator);
            } else {
                iterator++;
            }
        }

        DEBUG(agent << " result: ");
        for (auto& sub : subtracted) {
            DEBUG("val: " << sub);
        }
        DEBUG(" finish result" << std::endl);

        return subtracted;
    }

    std::optional<billiard::search::Ball> tryToSetBallAtPositions(const std::vector<ReplacePosition>& positions,
                                                                  float diameterSquared,
                                                                  const std::string& type,
                                                                  const std::string& id,
                                                                  const billiard::search::State& state) {
        static std::string agent = "[tryToSetBallAtPositions] ";
        for(auto& position : positions) {
            bool isValidPosition = true;
            DEBUG(agent << "try to set" << " "
                        << id << " at position (" << position._position.x << ";" << position._position.y << ")"
                        << std::endl);

            for (auto& ball : state._balls) {
                auto s = ball._position - position._position;
                auto distanceSquared = glm::dot(s, s);
                if (distanceSquared < diameterSquared) {
                    DEBUG(agent << "cannot set ball because " << ball._id << " is too close." << " "
                                << "distance squared: " << distanceSquared << " "
                                << "min distance squared: " << diameterSquared
                                << std::endl);
                    isValidPosition = false;
                    break;
                }
            }

            if (isValidPosition) {
                return createBall(position._position, type, id);
            }
        }

        return std::nullopt;
    }

    std::optional<billiard::search::Ball> replaceInArea(float searchRadius,
                                                               float diameterSquared,
                                                               const glm::vec2& minimum,
                                                               const glm::vec2& maximum,
                                                               const float radius,
                                                               const glm::vec2& spot,
                                                               const std::string& type,
                                                               const std::string& id,
                                                               const billiard::search::State& state) {
        static std::string agent = "[replaceInArea] ";

        auto perpRailDirection = billiard::physics::perp(_searchConfig.headRailDirection);
        float expandStepSize = _searchConfig.radius / 2.0f;
        auto expandSpace1 = expandStepSize * _searchConfig.headRailDirection;
        auto expandSpace2 = expandStepSize * perpRailDirection;
        auto expandSpace3 = expandStepSize * -perpRailDirection;
        auto expandSpace4 = expandStepSize * -_searchConfig.headRailDirection;

        DEBUG(agent
                    << "radius: " << _searchConfig.radius << " "
                    << "expand step size: " << expandStepSize << " "
                    << "expand space 1: (" << expandSpace1.x << ";" << expandSpace1.y << ")" << " "
                    << "expand space 2: (" << expandSpace2.x << ";" << expandSpace2.y << ")" << " "
                    << "expand space 3: (" << expandSpace3.x << ";" << expandSpace3.y << ")" << " "
                    << "expand space 4: (" << expandSpace4.x << ";" << expandSpace4.y << ")" << " "
                    << std::endl);

        std::vector<ReplacePosition> fringe {ReplacePosition{spot, ReplacePositionAction::NONE}};
        auto searchRadiusSquared = searchRadius * searchRadius;

        while (!fringe.empty()) {
            DEBUG(agent << "do expansion 1" << std::endl);
            auto nextPositions1 = expandBySpace(spot, searchRadiusSquared, fringe, expandSpace1, minimum, maximum, radius,
                                                ReplacePositionAction::EXPAND_1) - fringe;
            auto replaced1 = tryToSetBallAtPositions(nextPositions1, diameterSquared, type, id, state);
            if (replaced1) {
                return replaced1;
            }

            DEBUG(agent << "do expansion 2" << std::endl);
            auto nextPositions2 = expandBySpace(spot, searchRadiusSquared, fringe, expandSpace2, minimum, maximum, radius,
                                                ReplacePositionAction::EXPAND_2) - fringe - nextPositions1;
            auto replaced2 = tryToSetBallAtPositions(nextPositions2, diameterSquared, type, id, state);
            if (replaced2) {
                return replaced2;
            }

            DEBUG(agent << "do expansion 3" << std::endl);
            auto nextPositions3 = expandBySpace(spot, searchRadiusSquared, fringe, expandSpace3, minimum, maximum, radius,
                                                ReplacePositionAction::EXPAND_3) - fringe - nextPositions1 - nextPositions2;
            auto replaced3 = tryToSetBallAtPositions(nextPositions3, diameterSquared, type, id, state);
            if (replaced3) {
                return replaced3;
            }

            DEBUG(agent << "do expansion 4" << std::endl);
            auto nextPositions4 = expandBySpace(spot, searchRadiusSquared, fringe, expandSpace4, minimum, maximum, radius,
                                                ReplacePositionAction::EXPAND_4) - fringe - nextPositions1 - nextPositions2 - nextPositions3;
            auto replaced4 = tryToSetBallAtPositions(nextPositions4, diameterSquared, type, id, state);
            if (replaced4) {
                return replaced4;
            }

            fringe.clear();
            fringe.insert(fringe.end(), nextPositions1.begin(), nextPositions1.end());
            fringe.insert(fringe.end(), nextPositions2.begin(), nextPositions2.end());
            fringe.insert(fringe.end(), nextPositions3.begin(), nextPositions3.end());
            fringe.insert(fringe.end(), nextPositions4.begin(), nextPositions4.end());

            DEBUG(agent << "fringe consists of: " << std::endl);
            for(auto& fringeValue : fringe) {
                DEBUG(agent << "fringe position: (" << fringeValue._position.x << ";" << fringeValue._position.y << ")" << std::endl);
            }
            DEBUG(agent << "end of fringe" << std::endl);
        }

        return std::nullopt;
    }

    struct BallReplacement {
        std::optional<billiard::search::Ball> _ball;
        bool _mustBeReplaced;
    };

    BallReplacement replace(const std::string& type, const std::string& id,
                                                  const billiard::search::State& state) {
        static std::string agent = "[replace] ";

        for (auto& ball : state._balls) {
            if (ball._type == type) {
                DEBUG(agent << "found ball of type " << type << ". No replace needed!" << std::endl);
                return BallReplacement{std::nullopt, false};
            }
        }

        auto spotType = type;

        do {
            auto spot = _searchConfig.spots[spotType];
            DEBUG(agent << "try to replace " << type << " at (" << spot._position.x << ";" << spot._position.y << ")" << std::endl);

            bool positionIsValid = true;
            for (auto& ball : state._balls) {
                auto s = ball._position - spot._position;
                auto distanceSquared = glm::dot(s, s);
                if (distanceSquared < _searchConfig.diameterSquared) {
                    DEBUG(agent << "cannot replace because ball " << ball._id << " is too close!" << " "
                                << "squared distance: " << distanceSquared << " "
                                << "min distance squared: " << _searchConfig.diameterSquared << std::endl);
                    positionIsValid = false;
                    break;
                }
            }

            if (positionIsValid) {
                DEBUG(agent << "replaces ball " << type << std::endl);
                return BallReplacement{createBall(spot._position, type, id), true};
            }
        } while (!(spotType = nextSearchType[spotType]).empty());

        auto spot = _searchConfig.spots[type];
        return BallReplacement{
                replaceInArea(_searchConfig.radius * SPOT_REPLACE_RADIUS_MULTIPLIER, _searchConfig.diameterSquared,
                              _searchConfig._minimum, _searchConfig._maximum, _searchConfig.radius,
                              spot._position, type, id, state),
            true};
    }

    billiard::search::State stateAfterBreak(const billiard::search::State& state,
                                            const std::unordered_map<std::string, std::string>& ids) {
        bool hasRed = false;

        for (auto& ball : state._balls) {
            if (ball._type == "RED") {
                hasRed = true;
                break;
            }
        }

        if (!hasRed) {
            return state;
        }

        auto balls = state._balls;

        for(auto& type : typesToReplace) {
            if (ids.count(type)) {
                auto replaced = replace(type, ids.at(type), state);
                if (replaced._ball) {
                    balls.push_back(*replaced._ball);
                } else if (replaced._mustBeReplaced) {
                    return billiard::search::State{std::vector<billiard::search::Ball>{}};
                }
            }
        }

        return billiard::search::State{balls};
    }

    bool validEndState(const std::vector<std::string>& expectedTypes, const billiard::search::node::Layer& layer) {
        int potted = 0;
        for (auto& node : layer._nodes) {
            if (node.second._type == billiard::search::node::NodeType::BALL_POTTING) {
                // Cue ball should never be potted
                if (node.second._ballType == "WHITE") {
                    DEBUG("[validEndState] Invalid end state: cue ball potted." << std::endl);
                    return false;
                }
                // No unexpected type of ball should be potted
                if (std::find(expectedTypes.begin(), expectedTypes.end(), node.second._ballType) == expectedTypes.end()) {
                    DEBUG("[validEndState] Invalid end state: ball of type " << node.second._ballType << " potted." << std::endl);
                    return false;
                }

                potted++;
            }
        }
        // At least one ball of the expected types should be potted
        if (potted == 0) {
            DEBUG("[validEndState] Invalid end state: no balls potted." << std::endl);
            return false;
        }

        // TODO: Failing to hit any other ball with the cue ball? No such solution should be found.
        // TODO: First hitting a ball "not-on" with the cue ball?
        // TODO: It is sometimes erroneously believed that potting two or more balls in one shot is an automatic foul.
        //  This is only true if one of the potted balls is not "on" (e.g. a red and a colour, or two different colours).
        //  When the reds are "on", two or more of them may be legally potted in the same shot and are worth one point each;
        //  however, the player may only nominate and attempt to pot one colour on the next shot.
        // TODO: Source: https://en.wikipedia.org/wiki/Rules_of_snooker
        // TODO: nominating colors: https://indoorgamebunker.com/why-do-snooker-players-nominate-a-colour/
        return true;
    }
}