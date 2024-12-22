#include <iostream>
#include <opencv2/opencv.hpp>

float distance(cv::Point2f a, cv::Point2f b)
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

void on_trackbar(int, void *) {}

int main()
{
    cv::VideoCapture cap("../armor.avi");
    if (!cap.isOpened())
    {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    // 创建滑动条，修改alpha和beta的值
    int alpha = 12, beta = 50;
    cv::namedWindow("Value", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("alpha value:", "Value", &alpha, 100, on_trackbar);
    cv::createTrackbar("beta value:", "Value", &beta, 200, on_trackbar);

    // !创建滑动条，修改蓝色的HSV范围
    int h_lower = 80, s_lower = 100, v_lower = 100;
    int h_higher = 140, s_higher = 255, v_higher = 255;
    cv::namedWindow("HSV", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("H lower:", "HSV", &h_lower, 180, on_trackbar);
    cv::createTrackbar("S lower:", "HSV", &s_lower, 255, on_trackbar);
    cv::createTrackbar("V lower:", "HSV", &v_lower, 255, on_trackbar);
    cv::createTrackbar("H higher:", "HSV", &h_higher, 180, on_trackbar);
    cv::createTrackbar("S higher:", "HSV", &s_higher, 255, on_trackbar);
    cv::createTrackbar("V higher:", "HSV", &v_higher, 255, on_trackbar);

    cv::Mat frame, hsv, mask, result;
    while (true)
    {
        cv::waitKey(100);
        float alpha_value = alpha / 10.0;
        float beta_value = (beta - 100) / 1.0;

        bool isFrameRead = cap.read(frame);
        if (!isFrameRead)
        {
            break;
        }
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        frame.convertTo(frame, -1, alpha_value, beta_value);

        // 设置蓝色装甲板的阈值范围
        inRange(hsv, cv::Scalar(h_lower, s_lower, v_lower), cv::Scalar(h_higher, s_higher, v_higher), mask);

        // 形态学操作，去除噪声和填补缺口
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
        morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

        // 轮廓检测
        std::vector<std::vector<cv::Point>> contours;
        findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // 轮廓筛选和装甲板识别
        std::vector<cv::RotatedRect> possible_armor;
        for (auto &contour : contours)
        {
            cv::RotatedRect rect = minAreaRect(contour);
            float aspect_ratio = (float)rect.size.width / rect.size.height;
            if (aspect_ratio > 1.0 && aspect_ratio < 5.0)
            { // 假设装甲板长宽比在1到5之间
                possible_armor.push_back(rect);
            }
        }

        // 绘制结果
        for (auto &armor : possible_armor)
        {
            cv::Point2f vertices[4];
            armor.points(vertices);
            for (int i = 0; i < 4; i++)
                line(frame, vertices[i], vertices[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Frame", frame);
        cv::imshow("Mask", mask);

        char key = (char)cv::waitKey(25);
        if (key == 27)
            break;
    }
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
