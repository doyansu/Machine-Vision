#include <iostream>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class ImageLibrary 
{
private:
    Vec3b _colorMap[256];

    // 建立 indexed image 的 color map
    void CreateColorMap() {
        // B 4種, G 8種, R 8種 = 4 * 8 * 8 = 256
        int colorDiv[3] = { 4, 8, 8 };
        for (int i = 0; i < colorDiv[0]; i++)
            for (int j = 0; j < colorDiv[1]; j++)
                for (int k = 0; k < colorDiv[2]; k++) {
                    int index = i * colorDiv[1] * colorDiv[2] + j * colorDiv[2] + k;
                    _colorMap[index] = Vec3b(i * (256 / colorDiv[0]), j * (256 / colorDiv[1]), k * (256 / colorDiv[2]));
                    cout << index << _colorMap[index] << '\n';
                }
    }

    Mat ResizeWithoutInterpolation(Mat colorImage, int scale, bool zoomIn) {
        Mat resizeImage;
        if (zoomIn) {
            resizeImage = Mat(colorImage.rows * scale, colorImage.cols * scale, CV_8UC3);
            for (int i = 0; i < resizeImage.rows; i++)
                for (int j = 0; j < resizeImage.cols; j++)
                    resizeImage.at<Vec3b>(i, j) = colorImage.at<Vec3b>(i / scale, j / scale);
        }
        else {
            resizeImage = Mat(colorImage.rows / scale, colorImage.cols / scale, CV_8UC3);
            for (int i = 0; i < resizeImage.rows; i++)
                for (int j = 0; j < resizeImage.cols; j++)
                    resizeImage.at<Vec3b>(i, j) = colorImage.at<Vec3b>(scale * i, scale * j);
        }
        return resizeImage;
    }

    Mat ResizeWithInterpolation(Mat colorImage, int scale, bool zoomIn) {
        Mat resizeImage;
        if (zoomIn) {
            // Bilinear Interpolation
            // 公式參考 https://www.cnblogs.com/xpvincent/archive/2013/03/15/2961448.html
            int height = colorImage.rows;
            int width = colorImage.cols;

            int new_height = height * scale;
            int new_width = width * scale;

            resizeImage = Mat(new_height, new_width, CV_8UC3);

            for (int i = 0; i < new_height; i++)
                for (int j = 0; j < new_width; j++) 
                    for (int k = 0; k < 3; k++) {
                        double x = (double)j / scale;
                        double y = (double)i / scale;
                        int x1 = (int)x;
                        int x2 = min(x1 + 1, width - 1);
                        int y1 = (int)y;
                        int y2 = min(y1 + 1, height - 1);
                        double fx1 = (x2 - x) / (x2 - x1) * colorImage.at<Vec3b>(y1, x1)[k] + (x - x1) / (x2 - x1) * colorImage.at<Vec3b>(y1, x2)[k];
                        double fx2 = (x2 - x) / (x2 - x1) * colorImage.at<Vec3b>(y2, x1)[k] + (x - x1) / (x2 - x1) * colorImage.at<Vec3b>(y2, x2)[k];
                        double fy = (y2 - y) / (y2 - y1) * fx1 + (y - y1) / (y2 - y1) * fx2;
                        resizeImage.at<Vec3b>(i, j)[k] = (uchar)fy;
                    }
        }
        else {
            resizeImage = Mat(colorImage.rows / scale, colorImage.cols / scale, CV_8UC3);
            for (int w = 0; w < resizeImage.rows; w++)
                for (int h = 0; h < resizeImage.cols; h++) {
                    uchar color[3] = { 0, 0, 0 };
                    for (int k = 0; k < 3; k++) {
                        int sum = 0;
                        for (int i = 0; i < scale; i++)
                            for (int j = 0; j < scale; j++)
                                sum += colorImage.at<Vec3b>(h * scale + i, w * scale + j)[k];
                        color[k] = sum / (scale * scale);
                    }
                    resizeImage.at<Vec3b>(h, w) = Vec3b(color[0], color[1], color[2]);
                }
        }
        return resizeImage;
    }

public:
    ImageLibrary() {
        this->CreateColorMap();
    }

    // 轉灰階
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

    // 灰階二值化
    Mat ConvertToBinary(Mat colorImage, uchar threshold = 128) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int i = 0; i < grayImage.rows; i++)
            for (int j = 0; j < grayImage.cols; j++)
                binaryImage.at<uchar>(i, j) = grayImage.at<uchar>(i, j) > threshold ? 255 : 0;
        return binaryImage;
    }

    // 使用對應表轉成 indexed image
    Mat ConvertToIndexedColor(Mat colorImage, Vec3b colorMap[256] = nullptr) {
        if (colorMap == nullptr)
            colorMap = this->_colorMap;

        Mat indexImage(colorImage.size(), CV_8UC1);
        Mat mappingImage(colorImage.size(), CV_8UC3);
        const int MAX_DIST = 255.0 * 255.0 * 3.0;
        for (int i = 0; i < colorImage.rows; i++) {
            for (int j = 0; j < colorImage.cols; j++) {
                Vec3b color = colorImage.at<Vec3b>(i, j);
                uchar index = 0;
                int minDist = MAX_DIST;
                for (int k = 0; k < 256; k++) {
                    Vec3b mapColor = colorMap[k];
                    int dist =
                        (color[0] - mapColor[0]) * (color[0] - mapColor[0]) +
                        (color[1] - mapColor[1]) * (color[1] - mapColor[1]) +
                        (color[2] - mapColor[2]) * (color[2] - mapColor[2]);
                    if (dist < minDist) {
                        minDist = dist;
                        index = k;
                    }
                }
                indexImage.at<uchar>(i, j) = index;
                mappingImage.at<Vec3b>(i, j) = colorMap[index];
            }
        }
        return mappingImage;
    }

    // Resize scale:放大縮小倍數，zoomIn = true 放大反之縮小，interpolation = true with interpolation
    Mat Resize(Mat colorImage, int scale, bool zoomIn = true, bool interpolation = false) {
        return interpolation ? ResizeWithInterpolation(colorImage, scale, zoomIn) : ResizeWithoutInterpolation(colorImage, scale, zoomIn);
    }
};

int main() {
    const string IMAGE_NAMES[] = {
        "House256",
        "House512",
        "JellyBeans",
        "Lena",
        "Mandrill",
        "Peppers",
    };
    
    ImageLibrary library = ImageLibrary();

    const string IMAGE_PATH_FORMAT = "..\\image\\%s.png";
    for (string name : IMAGE_NAMES)
    {
        const string IMAGE_PATH = format(IMAGE_PATH_FORMAT.c_str(), name.c_str());
        cout << IMAGE_PATH << '\n';

        Mat colorImage = imread(IMAGE_PATH);
        Mat grayImage = library.ConvertToGray(colorImage);
        Mat binaryImage = library.ConvertToBinary(colorImage, 128);
        Mat indexColorImage = library.ConvertToIndexedColor(colorImage);
        Mat zoomInImage = library.Resize(colorImage, 2, true, false);
        Mat zoomOutImage = library.Resize(colorImage, 2, false, false);
        Mat zoomInWithInterpolationImage = library.Resize(colorImage, 2, true, true);
        Mat zoomOutWithInterpolationImage = library.Resize(colorImage, 2, false, true);

        imshow("true-color " + IMAGE_PATH, colorImage);
        imshow("gray " + IMAGE_PATH, grayImage);
        imshow("binary " + IMAGE_PATH, binaryImage);
        imshow("indexed " + IMAGE_PATH, indexColorImage);
        imshow("zoomInImage " + IMAGE_PATH, zoomInImage);
        imshow("zoomOutImage " + IMAGE_PATH, zoomOutImage);
        imshow("zoomInWithInterpolationImage " + IMAGE_PATH, zoomInWithInterpolationImage);
        imshow("zoomOutWithInterpolationImage " + IMAGE_PATH, zoomOutWithInterpolationImage);

        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_gray").c_str()), grayImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_binary").c_str()), binaryImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_indexed").c_str()), indexColorImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_zoomInImage").c_str()), zoomInImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_zoomOutImage").c_str()), zoomOutImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_zoomInWithInterpolationImage").c_str()), zoomInWithInterpolationImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_zoomOutWithInterpolationImage").c_str()), zoomOutWithInterpolationImage);
        
        waitKey(0);
        destroyAllWindows();
    }
    return 0;
}