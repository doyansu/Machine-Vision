#include <iostream>
#include <filesystem> // ISO C++17 標準 (/std:c++17)
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

class ImageLibrary
{
public:
    enum class EdgeType {
        Vertical,
        Horizon,
        Both
    };

    ImageLibrary() {
        
    }

    // 灰階
    Mat ConvertToGray(const Mat& colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC3);

        for (int i = 0; i < colorImage.rows; i++) {
            for (int j = 0; j < colorImage.cols; j++) {
                Vec3b pixel = colorImage.at<Vec3b>(i, j);
                int grayValue = 0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0];
                grayImage.at<uchar>(i, j) = grayValue;
            }
        }
        return grayImage;
    }

    // 二值化
    Mat ConvertToBinary(const Mat& colorImage, uchar threshold = 128) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int i = 0; i < grayImage.rows; i++)
            for (int j = 0; j < grayImage.cols; j++)
                binaryImage.at<uchar>(i, j) = grayImage.at<uchar>(i, j) > threshold ? 255 : 0;
        return binaryImage;
    }

    Mat SobelEdgeDetection(const Mat& sourceImage, EdgeType edgeType = EdgeType::Vertical) {

    }



private:
    // padding 複製邊界像素值
    Mat PadByReplicated(const Mat& image, int paddingSize = 1) {
        Mat padded = Mat(image.rows + paddingSize * 2, image.cols + paddingSize * 2, image.type());

        // 複製邊界像素值
        for (int i = 0; i < padded.rows; i++) {
            for (int j = 0; j < padded.cols; j++) {
                int ii = i - paddingSize;
                int jj = j - paddingSize;

                if (ii < 0)
                    ii = 0;
                else if (ii >= image.rows)
                    ii = image.rows - 1;

                if (jj < 0)
                    jj = 0;
                else if (jj >= image.cols)
                    jj = image.cols - 1;

                padded.at<Vec3b>(i, j) = image.at<Vec3b>(ii, jj);
            }
        }
        return padded;
    }

    // padding 使用 0 填充
    Mat PadByZero(const Mat& image, int paddingSize = 1) {
        Mat padded = Mat(image.rows + paddingSize * 2, image.cols + paddingSize * 2, image.type(), Scalar(0));
        image.copyTo(padded(Rect(paddingSize, paddingSize, image.cols, image.rows)));
        return padded;
    }
};

// 存圖片路徑資訊
class ImageInfo 
{
public:
    ImageInfo(string path) {
        this->_path = fs::path(path);
    };

    string Path() {
        return this->_path.relative_path().string();
    }

    string FileName() {
        return this->_path.stem().string();
    }

    string Extension() {
        return this->_path.extension().string();
    }

private:
    fs::path _path;
};

int main() {
    const string IMAGE_FOLDER = "..\\image\\"; // 資源圖片資料夾
    const string IMAGE_OUTPUT_FOLDER = "..\\image\\output\\"; // 結果存放資料夾

    if (!fs::exists(IMAGE_OUTPUT_FOLDER))
        std::cout << (fs::create_directory(IMAGE_OUTPUT_FOLDER) ? "Output folder :" + IMAGE_OUTPUT_FOLDER + " created successfully." : "Failed to create output folder.") << std::endl;

    // 圖片檔名設定
    vector<ImageInfo> images;
    images.push_back(ImageInfo(IMAGE_FOLDER + "House512.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Lena.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Mandrill.png"));

    ImageLibrary library = ImageLibrary();

    for (ImageInfo imageInfo : images)
    {
        std::cout << imageInfo.Path() << std::endl;

        // 讀取圖片
        Mat sourceImage = imread(imageInfo.Path());
        Mat grayImage = library.ConvertToGray(sourceImage);

        imshow(imageInfo.FileName(), sourceImage);
        imshow(imageInfo.FileName(), grayImage);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}