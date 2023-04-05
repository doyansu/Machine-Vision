#include "ImageLibrary.h"
#include <iostream>
#include <queue>

namespace image_model {
    // 轉灰階
    Mat ImageLibrary::ConvertToGray(Mat colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC1);

        for (int row = 0; row < grayImage.rows; row++) {
            for (int col = 0; col < grayImage.cols; col++) {
                Vec3b pixel = colorImage.at<Vec3b>(row, col);
                int grayValue = (int)(0.34 * pixel[2] + 0.33 * pixel[1] + 0.33 * pixel[0]);
                grayImage.at<uchar>(row, col) = grayValue;
            }
        }
        return grayImage;
    }

    // 二值化
    Mat ImageLibrary::ConvertToBinary(Mat colorImage, uchar threshold) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int row = 0; row < binaryImage.rows; row++)
            for (int col = 0; col < binaryImage.cols; col++)
                binaryImage.at<uchar>(row, col) = grayImage.at<uchar>(row, col) > threshold ? 255 : 0;
        return binaryImage;
    }

    // 轉 Labeling Image
    Mat ImageLibrary::ConvertToLabeling(Mat colorImage, Connected connected, int* objNumber){
        Mat binaryImage = this->ConvertToBinary(colorImage, 160);

        // 將 uchar 改為 int 並將 0 變成 -1 (物件) ， 255 變成 0 (背景)
        Mat labels(binaryImage.size(), CV_32S);
        for (int row = 0; row < binaryImage.rows; row++)
            for (int col = 0; col < binaryImage.cols; col++) 
                labels.at<int>(row, col) = binaryImage.at<uchar>(row, col) > 0 ? 0 : -1;

        // label (Recursive Algorithm)
        int label = 0;
        vector<Point> checkPoints = { Point(0, -1), Point(-1, 0), Point(0, 1), Point(1, 0) }; // 4-connected 的 checkPoint
        if (connected == Connected::Eight)
        {
            vector<Point> eightConnected = { Point(-1, -1), Point(-1, 1), Point(1, -1), Point(1, 1) }; // 8-connected 需加入的 checkPoint
            checkPoints.insert(checkPoints.end(), eightConnected.begin(), eightConnected.end());
        }
        auto InImage = [rows = labels.rows, cols = labels.cols](Point point) { // 檢查 point 座標是否合法
            return point.x >= 0 && point.y >= 0 && point.x < rows && point.y < cols;
        };
        for (int row = 0; row < labels.rows; row++)
            for (int col = 0; col < labels.cols; col++) 
                if (labels.at<int>(row, col) == -1) {
                    labels.at<int>(row, col) = ++label;
                    std::queue<Point> queue;
                    queue.push(Point(row, col));
                    while (!queue.empty()) {
                        Point point = queue.front();
                        queue.pop();
                        for (Point checkPoint : checkPoints)
                        {
                            checkPoint.x += point.x;
                            checkPoint.y += point.y;
                            if (InImage(checkPoint) && labels.at<int>(checkPoint.x, checkPoint.y) == -1)
                            {
                                queue.push(checkPoint);
                                labels.at<int>(checkPoint.x, checkPoint.y) = label;
                            }
                        }
                    }
                }

        // 為每個物件填色
        const int MAX_COLOR = 256 * 256 * 256;
        Mat labelingImage(binaryImage.size(), CV_8UC3);
        for (int row = 0; row < binaryImage.rows; row++)
            for (int col = 0; col < binaryImage.cols; col++) {
                int color = labels.at<int>(row, col) * (MAX_COLOR / (label + 1));
                //labelingImage.at<Vec3b>(row, col) = labels.at<int>(row, col) > 0 ? Vec3b(255, 255, 255) : Vec3b(0, 0, 0);
                labelingImage.at<Vec3b>(row, col) = labels.at<int>(row, col) > 0 ? Vec3b((color >> 16) & 255, (color >> 8) & 255, color & 255) : Vec3b(0, 0, 0);
            }

        // object number
        if (objNumber != nullptr)
            *objNumber = label;

        return labelingImage;
    }
}