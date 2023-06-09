#pragma once
#include <opencv2/opencv.hpp>

using namespace cv;

namespace image_model {
    class ImageLibrary
    {
    public:
        ImageLibrary() {};

        // 連通數 4-connected 8-connected
        enum class Connected
        {
            Four = 4,
            Eight = 8
        };

        // 轉灰階
        Mat ConvertToGray(Mat colorImage);
        // 二值化
        Mat ConvertToBinary(Mat colorImage, uchar threshold = 128);
        // Labeling Image，connected: 連通數，objNumber: 寫入 label 的物件數量，sizeFilter: Size Filtering
        Mat ConvertToLabeling(Mat binaryImage, Connected connected = Connected::Four, int* objNumber = nullptr, int sizeFilter = -1);

    private:

    };
}
