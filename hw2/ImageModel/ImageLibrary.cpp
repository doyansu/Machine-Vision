#include "ImageLibrary.h"
#include <iostream>
#include <queue>

namespace image_model {
    // ��Ƕ�
    Mat ImageLibrary::ConvertToGray(Mat colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC1);

        for (int row = 0; row < grayImage.rows; row++) {
            for (int col = 0; col < grayImage.cols; col++) {
                Vec3b pixel = colorImage.at<Vec3b>(row, col);
                //int grayValue = (int)(0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0]);
                int grayValue = (int)(0.34 * pixel[2] + 0.33 * pixel[1] + 0.33 * pixel[0]);
                grayImage.at<uchar>(row, col) = grayValue;
            }
        }
        return grayImage;
    }

    // �G�Ȥ�
    Mat ImageLibrary::ConvertToBinary(Mat colorImage, uchar threshold) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int row = 0; row < binaryImage.rows; row++)
            for (int col = 0; col < binaryImage.cols; col++)
                binaryImage.at<uchar>(row, col) = grayImage.at<uchar>(row, col) > threshold ? 255 : 0;
        return binaryImage;
    }

    // binaryImage �� Labeling Image
    Mat ImageLibrary::ConvertToLabeling(Mat binaryImage, Connected connected, int* objNumber, int sizeFilter){
        // �N uchar �אּ int �ñN 0 �ܦ� -1 (����) �A 255 �ܦ� 0 (�I��)
        Mat labels(binaryImage.size(), CV_32S);
        for (int row = 0; row < binaryImage.rows; row++)
            for (int col = 0; col < binaryImage.cols; col++) 
                labels.at<int>(row, col) = binaryImage.at<uchar>(row, col) > 0 ? 0 : -1;

        // labeling (Recursive Algorithm)
        vector<Point> checkPoints = { Point(0, -1), Point(-1, 0), Point(0, 1), Point(1, 0) }; // 4-connected �� checkPoint
        vector<Point> eightConnected = { Point(-1, -1), Point(-1, 1), Point(1, -1), Point(1, 1) }; // 8-connected �ݥ[�J�� checkPoint
        if (connected == Connected::Eight)
            checkPoints.insert(checkPoints.end(), eightConnected.begin(), eightConnected.end());
        auto InImage = [rows = labels.rows, cols = labels.cols](Point point) { // �ˬd point �y�ЬO�_�X�k
            return point.x >= 0 && point.y >= 0 && point.x < rows && point.y < cols;
        };
        vector<int> objSize = { 0 }; // �����C�� object ���j�p
        int label = 0; // ����ƶq
        for (int row = 0; row < labels.rows; row++)
            for (int col = 0; col < labels.cols; col++) 
                if (labels.at<int>(row, col) == -1) {
                    labels.at<int>(row, col) = ++label;
                    objSize.push_back(1);
                    std::queue<Point> queue;
                    queue.push(Point(row, col));
                    while (!queue.empty()) {
                        Point point = queue.front();
                        queue.pop();
                        for (Point checkPoint : checkPoints) {
                            checkPoint.x += point.x;
                            checkPoint.y += point.y;
                            if (InImage(checkPoint) && labels.at<int>(checkPoint.x, checkPoint.y) == -1) {
                                queue.push(checkPoint);
                                labels.at<int>(checkPoint.x, checkPoint.y) = label;
                                objSize[label]++;
                            }
                        }
                    }
                }

        // ���C�Ӫ�����
        const int MAX_COLOR = 256 * 256 * 256;
        Mat labelingImage(binaryImage.size(), CV_8UC3);
        for (int row = 0; row < binaryImage.rows; row++)
            for (int col = 0; col < binaryImage.cols; col++) {
                int objLabel = labels.at<int>(row, col);
                int color = objLabel * (MAX_COLOR / (label + 1));
                labelingImage.at<Vec3b>(row, col) = objLabel > 0 && objSize[objLabel] > sizeFilter ? Vec3b((color >> 16) & 255, (color >> 8) & 255, color & 255) : Vec3b(0, 0, 0);
            }

        // object number
        for (int objLabel = 1; objLabel < objSize.size(); objLabel++)
            if (objSize[objLabel] <= sizeFilter)
                label--;
        if (objNumber != nullptr)
            *objNumber = label;

        return labelingImage;
    }
}