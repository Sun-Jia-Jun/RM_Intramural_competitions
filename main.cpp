#include <iostream>
#include <opencv2/opencv.hpp>

float distance(cv::Point2f a, cv::Point2f b) // 距离函数
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
int main()
{
    // 打开视频
    cv::VideoCapture cap;
    cap.open("../armor.avi");
    if (!cap.isOpened())
    {
        std::cout << "can't open the video";
        return 0;
    }
    // 定义变量
    cv::Mat now;
    std::vector<std::vector<cv::Point>> contours;                                  // 储存轮廓的容器
    std::vector<cv::Vec4i> hierarchy;                                              // 储存轮廓的层级关系
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30)); // 大力出奇迹 卷积核 // ! XD
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat mask;                        // 掩码
    cv::Mat bgrnow, blue, light, before; // 过程所需的帧缓存
    cv::RotatedRect need, print;         // 用于储存灯条信息

    // 创建滑动条，用于调试
    int v_value = 245;
    cv::namedWindow("Value", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("V Value:", "Value", &v_value, 255, NULL); // 调节V通道阈值的滑动条

    // 循环处理视频帧
    while (1) // for(;;)改成while true了
    {
        // 监听按键并处理,同时帧间隔为33ms
        int key = cv::waitKey(33); // 按p或空格暂停
        if (key == 'p' || key == ' ')
        {
            std::cout << "pause" << std::endl;
            std::cout << "press any key to continue" << std::endl;
            key = cv::waitKey(0); // 按任意键继续
        }
        if (key == 27) // 按ESC退出
        {
            std::cout << "exit" << std::endl;
            break;
        }

        // 从视频中读取帧
        if (!cap.read(now))
        {
            std::cout << "can't read the video" << std::endl;
            break;
        }

        // 预处理
        now.convertTo(now, -1, 1.2, -50); // 调整对比度和亮度
        std::vector<cv::Mat> b;
        cv::cvtColor(now, before, cv::COLOR_BGR2HSV); // //
        cv::split(before, b);                         // 剔除过暗区域 -->让所有亮度小于阈值的像素现在都被设置成0
        mask = b[2] < v_value;                        // 对V通道阈值化生成掩码mask (小于245变成255，大于245变成0)
        b[2].setTo(0, mask);                          // 应用掩码，将掩码中非0的位置在b[2]中变为0
        cv::merge(b, before);                         // 合并通道到before

        cv::cvtColor(before, bgrnow, cv::COLOR_HSV2BGR); // //
        cv::split(bgrnow, b);                            // 剔除白色日光灯干扰(如规则中无干扰则删除)
        mask = b[0] - b[1] < 20;                         // 蓝色绿色通道差异小于20的区域为255，差异大的区域为0
        b[0].setTo(0, mask);                             // 应用掩码，将掩码中非0的位置在b[0]中变为0

        blue = b[0]; // 提取B通道
        // ? Debug: cv::imshow("blue before", blue);

        // 形态学操作：闭运算->开运算->闭运算
        cv::morphologyEx(blue, blue, cv::MORPH_CLOSE, element, cv::Point(-1, -1));               // 大力出奇迹
        cv::morphologyEx(blue, blue, cv::MORPH_OPEN, element1, cv::Point(-1, -1));               // //
        cv::morphologyEx(blue, blue, cv::MORPH_CLOSE, element1, cv::Point(-1, -1));              // //
        cv::imshow("blue before", blue);                                                         // 测试
        cv::threshold(blue, blue, 100, 255, cv::THRESH_BINARY);                                  // ! Mat blue本来就是二值图，无需再二值化了
        cv::imshow("blue", blue);                                                                // 用于调试
        cv::findContours(blue, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 寻找轮廓，只检测最外层轮廓，且仅保存轮廓的拐点信息

        // 遍历轮廓，寻找目标灯条
        int k = 0;                              // 记录灯条的总数
        std::vector<cv::RotatedRect> light_bar; // 储存灯条信息

        for (int i = 0; i < contours.size(); i++)
        {
            need = cv::minAreaRect(contours[i]); // 计算给定轮廓的最小面积外接矩形,返回RotatedRect类型

            if (need.size.height * need.size.width > 100) // 去掉过小面积的轮廓
            {
                light_bar.push_back(need); // 将符合条件的灯条信息加入light_bar容器
                k++;

                // 绘制所选矩形(红色)，用于调试
                cv::Point2f pts[4];
                need.points(pts);
                cv::line(now, pts[0], pts[1], cv::Scalar(0, 0, 255), 3);
                cv::line(now, pts[1], pts[2], cv::Scalar(0, 0, 255), 3);
                cv::line(now, pts[2], pts[3], cv::Scalar(0, 0, 255), 3);
                cv::line(now, pts[3], pts[0], cv::Scalar(0, 0, 255), 3);
            }
        }
        // 遍历灯条组合，寻找符合条件的组合
        for (int i = 0; i < k; i++)
        {
            for (int j = i + 1; j < k; j++)
            {
                cv::Point2f pts[8], draw[4];
                std::vector<cv::Point2f> pts1;
                light_bar[i].points(pts);
                light_bar[j].points(pts + 4);
                if (0) // TODO 筛选灯条组合，尚未完成
                    continue;
                if (distance(pts[0], pts[1]) > 1.5 * light_bar[i].size.height || distance(pts[0], pts[1]) > 1.5 * light_bar[i].size.width)
                {
                    draw[0].x = 0.5 * (pts[1].x + pts[2].x);
                    draw[0].y = 0.5 * (pts[1].y + pts[2].y);
                    draw[1].x = 0.5 * (pts[3].x + pts[0].x);
                    draw[1].y = 0.5 * (pts[3].y + pts[0].y);
                }
                else
                {
                    draw[0].x = 0.5 * (pts[1].x + pts[0].x);
                    draw[0].y = 0.5 * (pts[1].y + pts[0].y);
                    draw[1].x = 0.5 * (pts[3].x + pts[2].x);
                    draw[1].y = 0.5 * (pts[3].y + pts[2].y);
                }
                if (distance(pts[4], pts[5]) > 1.5 * light_bar[j].size.height || distance(pts[4], pts[5]) > 1.5 * light_bar[j].size.width)
                {
                    draw[2].x = 0.5 * (pts[5].x + pts[6].x);
                    draw[2].y = 0.5 * (pts[5].y + pts[6].y);
                    draw[3].x = 0.5 * (pts[7].x + pts[4].x);
                    draw[3].y = 0.5 * (pts[7].y + pts[4].y);
                }
                else
                {
                    draw[2].x = 0.5 * (pts[5].x + pts[4].x);
                    draw[2].y = 0.5 * (pts[5].y + pts[4].y);
                    draw[3].x = 0.5 * (pts[7].x + pts[6].x);
                    draw[3].y = 0.5 * (pts[7].y + pts[6].y);
                } // 寻找灯条形成四边形端点
                cv::line(now, draw[0], draw[1], cv::Scalar(0, 255, 0), 3);
                cv::line(now, draw[1], draw[3], cv::Scalar(0, 255, 0), 3);
                cv::line(now, draw[3], draw[2], cv::Scalar(0, 255, 0), 3);
                cv::line(now, draw[2], draw[0], cv::Scalar(0, 255, 0), 3); // 绘制四边形
            }
        }
        cv::imshow("now", now);
    }
    return 0;
}