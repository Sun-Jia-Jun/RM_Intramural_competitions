#include <iostream>
#include <opencv2/opencv.hpp>

cv::Mat mask;
cv::Mat bgrnow, blue, light, before;
cv::RotatedRect need, print;
float distance(cv::Point2f a, cv::Point2f b) // ���뺯��
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
int main()
{
    cv::VideoCapture cap;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30)); // �������漣
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cap.open("../armor.avi");
    cv::Mat now;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    if (!cap.isOpened())
    {
        std::cout << "can't open the video";
        return 0;
    }
    for (;;)
    {
        std::vector<cv::RotatedRect> light_bar;
        if (cap.read(now) != false) // ��ȡ
        {
            std::vector<cv::Mat> b;
            cv::cvtColor(now, before, cv::COLOR_BGR2HSV);
            cv::split(before, b); // �޳���������
            mask = b[2] < 245;
            b[2].setTo(0, mask);
            cv::merge(b, before);
            cv::cvtColor(before, bgrnow, cv::COLOR_HSV2BGR);
            cv::split(bgrnow, b); // �޳���ɫ�չ�Ƹ���(��������޸�����ɾ��)
            mask = b[0] - b[1] < 20;
            b[0].setTo(0, mask);
            blue = b[0];                                                               // ��ȡBͨ��
            cv::morphologyEx(blue, blue, cv::MORPH_CLOSE, element, cv::Point(-1, -1)); // �������漣
            cv::morphologyEx(blue, blue, cv::MORPH_OPEN, element1, cv::Point(-1, -1));
            cv::morphologyEx(blue, blue, cv::MORPH_CLOSE, element1, cv::Point(-1, -1)); // ��������
            cv::threshold(blue, blue, 100, 255, cv::THRESH_BINARY);
            cv::imshow("blue", blue);                                                                // ���ڵ���
            cv::findContours(blue, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Ѱ������
            int k = 0;
            for (int i = 0; i < contours.size(); i++)
            {
                need = cv::minAreaRect(contours[i]);
                if (need.size.height * need.size.width > 100) // ����if�е�����ɸѡ������������С��Ӿ���
                {
                    light_bar.push_back(need);
                    k++;

                    cv::Point2f pts[4];
                    need.points(pts);
                    cv::line(now, pts[0], pts[1], cv::Scalar(0, 0, 255), 3);
                    cv::line(now, pts[1], pts[2], cv::Scalar(0, 0, 255), 3);
                    cv::line(now, pts[2], pts[3], cv::Scalar(0, 0, 255), 3);
                    cv::line(now, pts[3], pts[0], cv::Scalar(0, 0, 255), 3); // ���ڵ���
                }
            }
            for (int i = 0; i < k; i++)
            {
                for (int j = i + 1; j < k; j++)
                {
                    cv::Point2f pts[8], draw[4];
                    std::vector<cv::Point2f> pts1;
                    light_bar[i].points(pts);
                    light_bar[j].points(pts + 4);
                    if (0) // ɸѡ������ϣ���δ���
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
                    } // Ѱ�ҵ����γ��ı��ζ˵�
                    cv::line(now, draw[0], draw[1], cv::Scalar(0, 255, 0), 3);
                    cv::line(now, draw[1], draw[3], cv::Scalar(0, 255, 0), 3);
                    cv::line(now, draw[3], draw[2], cv::Scalar(0, 255, 0), 3);
                    cv::line(now, draw[2], draw[0], cv::Scalar(0, 255, 0), 3); // �����ı���
                }
            }
            cv::imshow("now", now);
            cv::waitKey(33);
        }
        else
            break;
    }
    return 0;
}