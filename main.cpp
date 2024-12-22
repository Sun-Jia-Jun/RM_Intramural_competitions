#include <iostream>
#include <opencv2/opencv.hpp>

float distance(cv::Point2f a, cv::Point2f b) // ���뺯��
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
int main()
{
    // ����Ƶ
    cv::VideoCapture cap;
    cap.open("../armor.avi");
    if (!cap.isOpened())
    {
        std::cout << "can't open the video";
        return 0;
    }
    // �������
    cv::Mat now;
    std::vector<std::vector<cv::Point>> contours;                                  // ��������������
    std::vector<cv::Vec4i> hierarchy;                                              // ���������Ĳ㼶��ϵ
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30)); // �������漣 ����� // ! XD
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat mask;                        // ����
    cv::Mat bgrnow, blue, light, before; // ���������֡����
    cv::RotatedRect need, print;         // ���ڴ��������Ϣ

    // ���������������ڵ���
    int v_value = 245;
    cv::namedWindow("Value", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("V Value:", "Value", &v_value, 255, NULL); // ����Vͨ����ֵ�Ļ�����

    // ѭ��������Ƶ֡
    while (1) // for(;;)�ĳ�while true��
    {
        // ��������������,ͬʱ֡���Ϊ33ms
        int key = cv::waitKey(33); // ��p��ո���ͣ
        if (key == 'p' || key == ' ')
        {
            std::cout << "pause" << std::endl;
            std::cout << "press any key to continue" << std::endl;
            key = cv::waitKey(0); // �����������
        }
        if (key == 27) // ��ESC�˳�
        {
            std::cout << "exit" << std::endl;
            break;
        }

        // ����Ƶ�ж�ȡ֡
        if (!cap.read(now))
        {
            std::cout << "can't read the video" << std::endl;
            break;
        }

        // Ԥ����
        now.convertTo(now, -1, 1.2, -50); // �����ԱȶȺ�����
        std::vector<cv::Mat> b;
        cv::cvtColor(now, before, cv::COLOR_BGR2HSV); // //
        cv::split(before, b);                         // �޳��������� -->����������С����ֵ���������ڶ������ó�0
        mask = b[2] < v_value;                        // ��Vͨ����ֵ����������mask (С��245���255������245���0)
        b[2].setTo(0, mask);                          // Ӧ�����룬�������з�0��λ����b[2]�б�Ϊ0
        cv::merge(b, before);                         // �ϲ�ͨ����before

        cv::cvtColor(before, bgrnow, cv::COLOR_HSV2BGR); // //
        cv::split(bgrnow, b);                            // �޳���ɫ�չ�Ƹ���(��������޸�����ɾ��)
        mask = b[0] - b[1] < 20;                         // ��ɫ��ɫͨ������С��20������Ϊ255������������Ϊ0
        b[0].setTo(0, mask);                             // Ӧ�����룬�������з�0��λ����b[0]�б�Ϊ0

        blue = b[0]; // ��ȡBͨ��
        // ? Debug: cv::imshow("blue before", blue);

        // ��̬ѧ������������->������->������
        cv::morphologyEx(blue, blue, cv::MORPH_CLOSE, element, cv::Point(-1, -1));               // �������漣
        cv::morphologyEx(blue, blue, cv::MORPH_OPEN, element1, cv::Point(-1, -1));               // //
        cv::morphologyEx(blue, blue, cv::MORPH_CLOSE, element1, cv::Point(-1, -1));              // //
        cv::imshow("blue before", blue);                                                         // ����
        cv::threshold(blue, blue, 100, 255, cv::THRESH_BINARY);                                  // ! Mat blue�������Ƕ�ֵͼ�������ٶ�ֵ����
        cv::imshow("blue", blue);                                                                // ���ڵ���
        cv::findContours(blue, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Ѱ��������ֻ���������������ҽ����������Ĺյ���Ϣ

        // ����������Ѱ��Ŀ�����
        int k = 0;                              // ��¼����������
        std::vector<cv::RotatedRect> light_bar; // ���������Ϣ

        for (int i = 0; i < contours.size(); i++)
        {
            need = cv::minAreaRect(contours[i]); // ���������������С�����Ӿ���,����RotatedRect����

            if (need.size.height * need.size.width > 100) // ȥ����С���������
            {
                light_bar.push_back(need); // �����������ĵ�����Ϣ����light_bar����
                k++;

                // ������ѡ����(��ɫ)�����ڵ���
                cv::Point2f pts[4];
                need.points(pts);
                cv::line(now, pts[0], pts[1], cv::Scalar(0, 0, 255), 3);
                cv::line(now, pts[1], pts[2], cv::Scalar(0, 0, 255), 3);
                cv::line(now, pts[2], pts[3], cv::Scalar(0, 0, 255), 3);
                cv::line(now, pts[3], pts[0], cv::Scalar(0, 0, 255), 3);
            }
        }
        // ����������ϣ�Ѱ�ҷ������������
        for (int i = 0; i < k; i++)
        {
            for (int j = i + 1; j < k; j++)
            {
                cv::Point2f pts[8], draw[4];
                std::vector<cv::Point2f> pts1;
                light_bar[i].points(pts);
                light_bar[j].points(pts + 4);
                if (0) // TODO ɸѡ������ϣ���δ���
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
    }
    return 0;
}