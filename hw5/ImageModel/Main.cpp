#include <iostream>
#include <filesystem> // ISO C++17 標準 (/std:c++17)
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

// 定義 Kernel 型別
template <typename content>
using Kernel = std::vector<vector<content>>;

class Filter
{
public:
    Filter() {}

    // 設定 Mask
    void SetMask(int mask) {
        // 確保 mask 至少為3，且是奇數
        this->_mask = (mask < 3) ? 3 : (mask | 1);
    }

    // 各個 Filter 實作 FilterImage 的方法
    virtual Mat FilterImage(const Mat& sourceImage) = 0;

protected:
    int _mask = 3;

    // padding 填充圖片
    virtual Mat PadByReplicated(const Mat& image, int paddingSize)
    {
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
};

class MeanFilter : public Filter
{
public:
    MeanFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {
        Mat resultImage = Mat(sourceImage.size(), CV_8UC3);
        Mat paddedImage = this->PadByReplicated(sourceImage, this->_mask / 2);
        for (int i = 0; i < resultImage.rows; i++)
            for (int j = 0; j < resultImage.cols; j++)
            {
                // 相加後平均
                int value = 0;
                for (int x = 0; x < this->_mask; x++)
                    for (int y = 0; y < this->_mask; y++)
                        value += paddedImage.at<Vec3b>(i + x, j + y)[0];
                value /= this->_mask * this->_mask;
                resultImage.at<Vec3b>(i, j) = Vec3b(value, value, value);
            }
        return resultImage;
    }
};

class MedianFilter : public Filter
{
public:
    MedianFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {
        Mat resultImage = Mat(sourceImage.size(), CV_8UC3);
        Mat paddedImage = this->PadByReplicated(sourceImage, this->_mask / 2);
        for (int i = 0; i < resultImage.rows; i++)
            for (int j = 0; j < resultImage.cols; j++)
            {
                // 取中間值
                int value = 0;
                vector<int> temp;
                for (int x = 0; x < this->_mask; x++)
                    for (int y = 0; y < this->_mask; y++)
                        temp.push_back(paddedImage.at<Vec3b>(i + x, j + y)[0]);
                std::sort(temp.begin(), temp.end());
                value = temp[(this->_mask * this->_mask) / 2];
                resultImage.at<Vec3b>(i, j) = Vec3b(value, value, value);
            }
        return resultImage;
    }
};

class GaussianFilter : public Filter
{
public:
    GaussianFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {
        Mat resultImage = Mat(sourceImage.size(), CV_8UC3);
        Mat paddedImage = this->PadByReplicated(sourceImage, this->_mask / 2);
        Kernel<double> kernel = CreateGaussianKernel(this->_mask);
        for (int i = 0; i < resultImage.rows; i++)
            for (int j = 0; j < resultImage.cols; j++)
            {
                // Gaussian
                double value = 0;
                for (int x = 0; x < this->_mask; x++)
                    for (int y = 0; y < this->_mask; y++)
                        value += paddedImage.at<Vec3b>(i + x, j + y)[0] * kernel[x][y];
                resultImage.at<Vec3b>(i, j) = Vec3b(value, value, value);
            }
        return resultImage;
    }

private:
    // 創建 Gaussian Kernel
    Kernel<double> CreateGaussianKernel(int kernelSize, double sigma = -1)
    {
        // 未給出 sigma 自動計算
        if (sigma <= 0)
            sigma = 0.3 * ((kernelSize - 1.0) * 0.5 - 1.0) + 0.8;

        const int CENTER = kernelSize / 2;
        Kernel<double> kernel(kernelSize, vector<double>(kernelSize, 0.0));

        // 計算 Gaussian Kernel 中每個元素的值
        double sum = 0.0;
        for (int i = 0; i < kernelSize; i++)
        {
            for (int j = 0; j < kernelSize; j++)
            {
                double x = double(i) - double(CENTER);
                double y = double(j) - double(CENTER);
                kernel[i][j] = exp(-(x * x + y * y) / (2.0 * sigma * sigma));
                sum += kernel[i][j];
            }
        }

        // 正規化
        for (int i = 0; i < kernelSize; i++)
            for (int j = 0; j < kernelSize; j++)
                kernel[i][j] /= sum;

        return kernel;
    }
};

class ImageLibrary
{
public:
    // Filter 類型
    enum class FilterType {
        Mean,
        Median,
        Gaussian,
    };

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

    // Filter
    Mat FilterBy(const Mat& sourceImage, FilterType filterType = FilterType::Gaussian, int mask = 3, unsigned int times = 1) {
        Mat resultImage = sourceImage;
        Filter* filter = this->CreateFilter(filterType);
        filter->SetMask(mask);
        while (times--)
            resultImage = filter->FilterImage(resultImage);
        delete filter;
        return resultImage;
    }

private:
    Filter* CreateFilter(FilterType filterType) {
        Filter* filter = nullptr;
        switch (filterType)
        {
        case ImageLibrary::FilterType::Mean:
            filter = new MeanFilter();
            break;
        case ImageLibrary::FilterType::Median:
            filter = new MedianFilter();
            break;
        case ImageLibrary::FilterType::Gaussian:
            filter = new GaussianFilter();
            break;
        default:
            throw "error";
            break;
        }
        return filter;
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

    // 定義 Sobel Kernel
    const Kernel<int> sobelX = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    const Kernel<int> sobelY = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

    // 定義 Prewitt Kernel
    const Kernel<int> prewittX = { {-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1} };
    const Kernel<int> prewittY = { {-1, -1, -1}, {0, 0, 0}, {1, 1, 1} };

    for (ImageInfo imageInfo : images)
    {
        std::cout << imageInfo.Path() << std::endl;

        // 讀取圖片
        Mat sourceImage = imread(imageInfo.Path());

        // 灰階、Gaussian 過濾
        Mat grayImage = library.ConvertToGray(sourceImage);
        Mat filterImage = library.FilterBy(grayImage, ImageLibrary::FilterType::Gaussian, 3);
        
        map<ImageLibrary::EdgeType, Mat> sobelResult = library.DetectEdgeByKernel(filterImage, sobelX, sobelY);
        map<ImageLibrary::EdgeType, Mat> prewittResult = library.DetectEdgeByKernel(filterImage, prewittX, prewittY);
        
        imshow(imageInfo.FileName(), sourceImage);
        imshow(imageInfo.FileName() + "_Sobel_vertical", sobelResult[ImageLibrary::EdgeType::Vertical]);
        imshow(imageInfo.FileName() + "_Sobel_horizon", sobelResult[ImageLibrary::EdgeType::Horizon]);
        imshow(imageInfo.FileName() + "_Sobel_both", sobelResult[ImageLibrary::EdgeType::Both]);
        imshow(imageInfo.FileName() + "_Prewitt_vertical", prewittResult[ImageLibrary::EdgeType::Vertical]);
        imshow(imageInfo.FileName() + "_Prewitt_horizon", prewittResult[ImageLibrary::EdgeType::Horizon]);
        imshow(imageInfo.FileName() + "_Prewitt_both", prewittResult[ImageLibrary::EdgeType::Both]);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}