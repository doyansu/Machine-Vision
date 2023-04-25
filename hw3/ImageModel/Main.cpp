#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>

using namespace std;
using namespace cv;

class ImageLibrary
{
private:

public:
    ImageLibrary() {
        
    }

    // ��Ƕ�
    Mat ConvertToGray(Mat colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC1);

        for (int i = 0; i < colorImage.rows; i++) {
            for (int j = 0; j < colorImage.cols; j++) {
                Vec3b pixel = colorImage.at<Vec3b>(i, j);
                int grayValue = 0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0];
                grayImage.at<uchar>(i, j) = grayValue;
            }
        }
        return grayImage;
    }

    // �Ƕ��G�Ȥ�
    Mat ConvertToBinary(Mat colorImage, uchar threshold = 128) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int i = 0; i < grayImage.rows; i++)
            for (int j = 0; j < grayImage.cols; j++)
                binaryImage.at<uchar>(i, j) = grayImage.at<uchar>(i, j) > threshold ? 255 : 0;
        return binaryImage;
    }

};
/*
class QuadtreeNode
{
public:
    Rect rect; // ??�x��?��
    int level; // ????
    bool is_leaf; // �O�_????
    Scalar color; // ????��
    QuadtreeNode* children[4]; // �l??��???

    // �۳y��?
    QuadtreeNode(Rect rect, int level)
    {
        this->rect = rect;
        this->level = level;
        this->is_leaf = true;
        this->color = Scalar(0, 0, 0);
        for (int i = 0; i < 4; i++) {
            this->children[i] = nullptr;
        }
    }

    // ����??�A��^�O�_���\
    bool split(Mat& image)
    {
        if (this->is_leaf && this->level < 4) {
            // ?��l??�x��?��
            int x = this->rect.x;
            int y = this->rect.y;
            int w = this->rect.width / 2;
            int h = this->rect.height / 2;

            // ?�ؤl??
            this->children[0] = new QuadtreeNode(Rect(x, y, w, h), this->level + 1);
            this->children[1] = new QuadtreeNode(Rect(x + w, y, w, h), this->level + 1);
            this->children[2] = new QuadtreeNode(Rect(x, y + h, w, h), this->level + 1);
            this->children[3] = new QuadtreeNode(Rect(x + w, y + h, w, h), this->level + 1);

            // ��s��???��
            this->is_leaf = false;
            this->color = Scalar(0, 0, 0);

            // ?�d�l??�O�_�ݭn����
            for (int i = 0; i < 4; i++) {
                if (this->children[i]->check_color(image)) {
                    this->children[i]->split(image);
                }
            }

            return true;
        }

        return false;
    }

    // ?�d??�x��?��?����?��O�_�ۦP
    bool check_color(Mat& image)
    {
        if (this->is_leaf) {
            // ?��x��?��
            int x = this->rect.x;
            int y = this->rect.y;
            int w = this->rect.width;
            int h = this->rect.height;

            // ??�x��?��?����?��?�q
            map<Scalar, int> color_count;
            for (int i = y; i < y + h; i++) {
                for (int j = x; j < x + w; j++) {
                    Scalar color = image.at<uchar>(i, j);
                    color_count[color]++;
                }
            }

            // �p�G?��?�q��? 1
            if (color_count.size() > 1) {
                return true;
            }

            // ��s???��
            this->color = color_count.begin()->first;
        }

        return false;
    }
};*/

int main() {
    const int IMAGE_NUM = 4; // �Ϥ���
    const string IMAGE_FOLDER = "..\\image"; // �Ϥ��s���Ƨ�
    const string IMAGE_PATH_FORMAT = IMAGE_FOLDER + "\\% s.png";

    // image �ɦW
    const string IMAGE_NAMES[IMAGE_NUM] = {
        "1",
        "2",
        "3",
        "4",
    };

    // threshold �]�w
    const int THRESHOLD_SETTRING[IMAGE_NUM] = {
        135,
        245,
        155,
        254,
    };

    ImageLibrary library = ImageLibrary();

    for (int i = 0; i < IMAGE_NUM; i++)
    {
        const string IMAGE_NAME = IMAGE_NAMES[i];
        const string IMAGE_PATH = format(IMAGE_PATH_FORMAT.c_str(), IMAGE_NAME.c_str());
        std::cout << IMAGE_PATH << '\n';

        // Ū���Ϥ�
        Mat colorImage = imread(IMAGE_PATH);
        Mat grayImage = library.ConvertToGray(colorImage);
        Mat binaryImage = library.ConvertToBinary(colorImage, THRESHOLD_SETTRING[i]);

        imshow("true-color " + IMAGE_PATH, colorImage);
        imshow("gray " + IMAGE_PATH, grayImage);
        imshow("binary " + IMAGE_PATH, binaryImage);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}