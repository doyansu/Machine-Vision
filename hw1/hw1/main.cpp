#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

Mat ConvertToGray(Mat colorImg) {
    Mat grayImg(colorImg.rows, colorImg.cols, CV_8UC1);

    for (int i = 0; i < colorImg.rows; i++)
    {
        for (int j = 0; j < colorImg.cols; j++)
        {
            Vec3b pixel = colorImg.at<Vec3b>(i, j);
            int grayValue = 0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0];
            grayImg.at<uchar>(i, j) = grayValue;
        }
    }
    return grayImg;
}

int main() {
    const std::string PATHS[] = {
        "..\\image\\House256.png",
        "..\\image\\House512.png",
        "..\\image\\JellyBeans.png",
        "..\\image\\Lena.png",
        "..\\image\\Mandrill.png",
        "..\\image\\Peppers.png",
    };

    for (std::string path : PATHS)
    {
        Mat colorImg = imread(path, -1);
        Mat grayImg = ConvertToGray(colorImg);
        imshow(path, grayImg);
    }

    waitKey(0);
    destroyAllWindows();
    return 0;
}