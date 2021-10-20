#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_debug/billiard_debug.hpp>
#include <algorithm>

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
#undef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT

namespace billiard::snooker {

    SnookerDetectionConfig config;
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
    };

    billiard::detection::State detect(const billiard::detection::State& previousState, const cv::Mat& original) {

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

        // Saturated balls mask
        cv::Mat saturationMask;
        cv::inRange(saturation, config.saturationFilter.x, config.saturationFilter.y, saturationMask);

        cv::Mat saturationMaskAndedWithRailMask;
        cv::bitwise_and(config.railMask, saturationMask, saturationMaskAndedWithRailMask);

        cv::Mat openedSaturationMask;
        cv::morphologyEx(saturationMaskAndedWithRailMask, openedSaturationMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1),
                         1);

        cv::Mat closedSaturationMask;
        cv::morphologyEx(openedSaturationMask, closedSaturationMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 4);

        cv::Mat saturatedBallMask = closedSaturationMask;

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::imshow("saturationMask", saturationMask);
        cv::imshow("saturationMaskAndedWithRailMask", saturationMaskAndedWithRailMask);
        cv::imshow("openedSaturationMask", openedSaturationMask);
        cv::imshow("closedSaturationMask", closedSaturationMask);
        cv::imshow("saturatedBallMask", saturatedBallMask);
#endif

        cv::Mat saturationMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, saturationMaskedGrayscale, saturatedBallMask);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
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

        // Hough on saturation masked input
        std::vector<cv::Vec3f> saturationCircles;
        HoughCircles(saturationMaskedGrayscale, saturationCircles, cv::HOUGH_GRADIENT, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::Mat edges;
        cv::Canny(saturationMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);

        cv::imshow("saturationMask - maskedInput grayscale edges", edges);
#endif

        // Black ball mask
        cv::Mat blackMask;
        {
            cv::Mat valueMask;
            cv::inRange(value, config.blackValueFilter.x, config.blackValueFilter.y, valueMask);

            cv::Mat valueMaskAndedWithTableMask;
            cv::bitwise_and(valueMask, config.innerTableMask, valueMaskAndedWithTableMask);

            // TODO: remove?
            cv::Mat valueMaskAndedWithRailMask;
            cv::bitwise_and(valueMask, config.railMask, valueMaskAndedWithRailMask);

            cv::Mat closedValueMask;
            cv::morphologyEx(valueMaskAndedWithTableMask, closedValueMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 6);

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

        // Hough on black masked input
        std::vector<cv::Vec3f> blackCircles;
        HoughCircles(blackMaskedGrayscale, blackCircles, cv::HOUGH_GRADIENT, 1,
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

        cv::Mat whitePinkMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, whitePinkMaskedGrayscale, whitePinkMask);

        cv::Mat closedWhitePinkMaskedGrayscale;
        cv::morphologyEx(whitePinkMaskedGrayscale, closedWhitePinkMaskedGrayscale, cv::MORPH_CLOSE, config.morphElementRect3x3, cv::Point(-1, -1), 2);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        {
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
        };
#endif

        // Hough on white&pink masked input
        std::vector<cv::Vec3f> whitePinkCircles;
        HoughCircles(closedWhitePinkMaskedGrayscale, whitePinkCircles, cv::HOUGH_GRADIENT, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        {
            cv::Mat edges;
            cv::Canny(closedWhitePinkMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);
            cv::imshow("white&pink - maskedInput grayscale edges", edges);
        };
#endif

        std::vector<cv::Vec3f> allCircles;

        {
            std::vector<cv::Vec3f> circles = saturationCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.railMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, saturatedBallMask, true);
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

                cv::imshow("saturated - hough on masked input", houghOnMaskedInput);
                cv::imshow("saturated - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
                cv::imshow("saturated - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
                cv::imshow("saturated - hough on masked input after circle filter 3", houghOnMaskedInputAfterCircleFilter3);
            };
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

        {
            std::vector<cv::Vec3f> circles = whitePinkCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.railMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, whitePinkMask, true);

            // Filter circles by saturated ball mask in order to remove 'duplicate' balls
            std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, saturatedBallMask, false);

            for (auto& circle : filteredCircles3) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
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
#endif
        }

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
            state._balls.push_back(ball);
        }

#ifdef BILLIARD_SNOOKER_DETECTION_DEBUG_VISUAL
        cv::Mat detectedBalls = drawDetectedBallsGrid(hough, state, 128, 8);
        cv::imshow("detected balls", detectedBalls);
        cv::waitKey(1);
#endif

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

        Histogram hueHist = histogramByChannels[0];
        Histogram saturationHist = histogramByChannels[1];
        Histogram valueHist = histogramByChannels[2];
        int maxHue = hueHist.maxLocation.y;
        int maxSaturation = saturationHist.maxLocation.y;
        int maxValue = valueHist.maxLocation.y;

        std::string label = "UNKNOWN";

//        const auto between = [](int value, int min, int max)  {
//            return value >= min && value <= max;
//        };

        const auto between = [](int value, cv::Point2d minMax)  {
            return value >= minMax.x && value <= minMax.y;
        };

        cv::Vec2f redPinkSeparatorLine = cv::Point2f { 1.0, 1.0 }; // Separate by (saturation, value)

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

            cv::Point2i brownHue {0, 10};
            if (between(maxHue, brownHue) && between(maxSaturation, classificationConfig.brownSaturation) && between(maxValue, classificationConfig.brownValue)) {
                label = "BROWN";
            }
            else if (between(maxSaturation, classificationConfig.pinkSaturation) && between(maxValue, classificationConfig.pinkValue)) {
                label = "PINK";
            }
            else {
                label = "RED";
            }
//            else {
//                cv::Vec2f point { (float) maxSaturation / 255, (float) maxValue / 255 }; // Separate by (saturation, value)
//
//                float perpProduct = redPinkSeparatorLine[0] * point[1] - redPinkSeparatorLine[1] * point[0];
//#ifdef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
//                std::cout << "redPinkSeparatorLine: " << redPinkSeparatorLine <<  " point: " << point << " perp product: " << perpProduct << std::endl;
//#endif
//                if (perpProduct <= 0) {
//                    label = "RED";
//                } else {
//                    label = "PINK";
//                }
//            }
        }

        ball._type = label;

#ifdef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
        std::cout << "Ball " << ball._id << " at (" << ball._position.x << ", " << ball._position.y << ")" << " classified as " << label << std::endl;

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

    billiard::search::Search nextSearch(const billiard::search::Search& previousSearch,
                                        const std::vector<std::string>& previousTypes) {
        return previousSearch; // TODO: Switch between search types -> From RED to color and vice versa
    }

    billiard::search::node::Layer stateAfterBreak(const billiard::search::node::Layer& layer) {
        return layer; // TODO: Replace colorized balls at their positions. (Pass config and append configuration)
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