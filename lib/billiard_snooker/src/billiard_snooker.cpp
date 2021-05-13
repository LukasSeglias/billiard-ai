#include <billiard_snooker/billiard_snooker.hpp>

namespace billiard::snooker {

    SnookerDetectionConfig config;

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

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;
#endif
        config.morphElementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));
        config.innerTableMask = detectionConfig.innerTableMask;

        config.valid = true;
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

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);
#endif

        // Saturated balls mask
        cv::Mat saturationMask;
        cv::inRange(saturation, config.saturationFilter.x, config.saturationFilter.y, saturationMask);

        cv::Mat openedSaturationMask;
        cv::morphologyEx(saturationMask, openedSaturationMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1),
                         1);

        cv::Mat closedSaturationMask;
        cv::morphologyEx(openedSaturationMask, closedSaturationMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 2);

        cv::Mat saturatedBallMask = closedSaturationMask;

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::imshow("saturationMask", saturationMask);
        cv::imshow("openedSaturationMask", openedSaturationMask);
        cv::imshow("closedSaturationMask", closedSaturationMask);
        cv::imshow("saturatedBallMask", saturatedBallMask);
#endif

        cv::Mat saturationMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, saturationMaskedGrayscale, saturatedBallMask);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
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

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
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

            cv::Mat closedValueMask;
            cv::morphologyEx(valueMaskAndedWithTableMask, closedValueMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 4);

            cv::Mat openedValueMask;
            cv::morphologyEx(closedValueMask, openedValueMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1),3);

            blackMask = openedValueMask;

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
            cv::imshow("black - valueMask", valueMask);
            cv::imshow("black - valueMask anded with innerTableMask", valueMaskAndedWithTableMask);
            cv::imshow("black - closedValueMask", closedValueMask);
            cv::imshow("black - openedValueMask", openedValueMask);
            cv::imshow("black - mask", blackMask);
#endif

        }

        cv::Mat blackMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, blackMaskedGrayscale, blackMask);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::Mat blackMaskedInput;
        cv::bitwise_and(input, input, blackMaskedInput, blackMask);
        cv::Mat invMask;
        cv::bitwise_not(blackMask, invMask);
        cv::Mat maskedDelta;
        cv::bitwise_and(input, input, maskedDelta, invMask);

        cv::imshow("black - masked color", blackMaskedInput);
        cv::imshow("black - masked grayscale", blackMaskedGrayscale);
        cv::imshow("black - masked delta", maskedDelta);
#endif

        // Hough on black masked input
        std::vector<cv::Vec3f> blackCircles;
        HoughCircles(blackMaskedGrayscale, blackCircles, cv::HOUGH_GRADIENT, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::Mat edges;
        cv::Canny(blackMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);

        cv::imshow("black - maskedInput grayscale edges", edges);
#endif

        // White & Pink Mask
        cv::Mat whitePinkMask;
        {
            cv::Mat valueMask;
            cv::inRange(value, config.whitePinkValueFilter.x, config.whitePinkValueFilter.y, valueMask);

            cv::Mat openedMask;
            cv::morphologyEx(valueMask, openedMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, config.morphElementRect3x3, cv::Point(-1, -1), 1);

            whitePinkMask = closedMask;

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
            cv::imshow("white&pink - valueMask", valueMask);
            cv::imshow("white&pink - openedMask", openedMask);
            cv::imshow("white&pink - closedMask", closedMask);
            cv::imshow("white&pink - mask", whitePinkMask);
#endif
        }

        cv::Mat whitePinkMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, whitePinkMaskedGrayscale, whitePinkMask);

        cv::Mat closedWhitePinkMaskedGrayscale;
        cv::morphologyEx(whitePinkMaskedGrayscale, closedWhitePinkMaskedGrayscale, cv::MORPH_CLOSE, config.morphElementRect3x3, cv::Point(-1, -1), 2);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
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
#endif

        // Hough on white&pink masked input
        std::vector<cv::Vec3f> whitePinkCircles;
        HoughCircles(closedWhitePinkMaskedGrayscale, whitePinkCircles, cv::HOUGH_GRADIENT, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::Mat edges;
        cv::Canny(whitePinkMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);
        cv::imshow("white&pink - maskedInput grayscale edges", edges);
#endif

        std::vector<cv::Vec3f> allCircles;

        {
            std::vector<cv::Vec3f> circles = saturationCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.innerTableMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, saturatedBallMask, true);
            std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, blackMask, false);
            for (auto& circle : filteredCircles3) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
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
#endif
        }

        {
            std::vector<cv::Vec3f> circles = blackCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.innerTableMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, blackMask, true);
            for (auto& circle : filteredCircles2) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
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
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.innerTableMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, whitePinkMask, true);

            // Filter circles by saturated ball mask in order to remove 'duplicate' balls
            std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, saturatedBallMask, false);

            for (auto& circle : filteredCircles3) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
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

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::Mat hough = input.clone();
        drawHoughResult(hough, allCircles);
        cv::imshow("hough", hough);
#endif

        std::vector<cv::Point2d> imagePoints = circleCenters(allCircles);

        billiard::detection::State state;
        for (int i = 0; i < imagePoints.size(); i++) {
            auto& point = imagePoints[i];
            billiard::detection::Ball ball;
            ball._type = "RED"; // TODO: remove this!!!!!!!!!!
            ball._id = std::to_string(i);
            ball._position = glm::vec2 {point.x, point.y};
            state._balls.push_back(ball);
        }

        return state;
    }

}