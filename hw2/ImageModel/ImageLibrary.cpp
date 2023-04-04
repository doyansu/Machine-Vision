#include "ImageLibrary.h"

namespace image_model {
    // 轉灰階
    Mat ImageLibrary::ConvertToGray(Mat colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC1);

        for (int row = 0; row < colorImage.rows; row++) {
            for (int col = 0; col < colorImage.cols; col++) {
                Vec3b pixel = colorImage.at<Vec3b>(row, col);
                int grayValue = (int)(0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0]);
                grayImage.at<uchar>(row, col) = grayValue;
            }
        }
        return grayImage;
    }

    // 二值化
    Mat ImageLibrary::ConvertToBinary(Mat colorImage, uchar threshold) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int row = 0; row < grayImage.rows; row++)
            for (int col = 0; col < grayImage.cols; col++)
                binaryImage.at<uchar>(row, col) = grayImage.at<uchar>(row, col) > threshold ? 255 : 0;
        return binaryImage;
    }

    // 轉 Labeling Image
    Mat ImageLibrary::ConvertToLabeling(Mat colorImage, Connected connected, int* objNumber){
        int count = 0;
        Mat binaryImage = this->ConvertToBinary(colorImage);
        Mat labelingImage(binaryImage.size(), CV_8UC3);

        if (objNumber != nullptr)
            *objNumber = count;

        return labelingImage;
    }
}