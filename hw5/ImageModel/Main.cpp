#include <iostream>
#include <filesystem> // ISO C++17 標準 (/std:c++17)
#include <opencv2/opencv.hpp>
#include <functional>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

// 定義 Kernel 型別
template <typename content>
using Kernel = vector<vector<content>>;

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
                grayImage.at<Vec3b>(i, j) = Vec3b(grayValue, grayValue, grayValue);
            }
        }
        return grayImage;
    }

    // 二值化
    Mat ConvertToBinary(const Mat& grayImage, uchar threshold = 128) {
        Mat binaryImage(grayImage.size(), CV_8UC3);

        for (int i = 0; i < grayImage.rows; i++)
            for (int j = 0; j < grayImage.cols; j++)
                binaryImage.at<Vec3b>(i, j) = grayImage.at<Vec3b>(i, j)[0] > threshold ? Vec3b(255, 255, 255) : Vec3b(0, 0, 0);
        return binaryImage;
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

    // Sobel Edge Detect
    map<EdgeType, Mat> Sobel(const Mat& sourceImage, uchar threshold = 128) {
        // 定義 Sobel Kernel
        const Kernel<int> sobelKernelX = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
        const Kernel<int> sobelKernelY = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };
        return DetectEdgeBy2Kernel(sourceImage, sobelKernelX, sobelKernelY, threshold);
    }

    // Prewitt Edge Detect
    map<EdgeType, Mat> Prewitt(const Mat& sourceImage, uchar threshold = 128) {
        // 定義 Prewitt Kernel
        const Kernel<int> prewittKernelX = { {-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1} };
        const Kernel<int> prewittKernelY = { {-1, -1, -1}, {0, 0, 0}, {1, 1, 1} };
        return DetectEdgeBy2Kernel(sourceImage, prewittKernelX, prewittKernelY, threshold);
    }

    // Laplacian Edge Detect
    Mat Laplacian(const Mat& sourceImage, const Kernel<int>& kernel, uchar threshold = 128) {
        const Kernel<int> zeroKernelX = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
        return DetectEdgeBy2Kernel(sourceImage, kernel, zeroKernelX, threshold)[EdgeType::Both];
    }

private:
    // 進行邊緣梯度計算
    map<EdgeType, Mat> DetectEdgeBy2Kernel(const Mat& sourceImage, const Kernel<int>& kernelX, const Kernel<int>& kernelY, uchar threshold = 128) {
        map<EdgeType, Mat> resultMap;
        Mat& verticalImage = resultMap[EdgeType::Vertical] = Mat(sourceImage.size(), CV_8UC3);
        Mat& horizonImage = resultMap[EdgeType::Horizon] = Mat(sourceImage.size(), CV_8UC3);
        Mat& bothImage = resultMap[EdgeType::Both] = Mat(sourceImage.size(), CV_8UC3);

        // padding
        Mat padded = this->PadByZero(sourceImage, kernelX.size() / 2);

        // 計算 gx, gy, G = |gx| + |gy|
        int max = 0;
        Kernel<int> tempG(sourceImage.rows, vector<int>(sourceImage.cols));
        for (int i = 0; i < sourceImage.rows; i++) {
            for (int j = 0; j < sourceImage.cols; j++) {
                int gx = 0;
                for (int m = 0; m < kernelX.size(); m++)
                    for (int n = 0; n < kernelX.size(); n++)
                        gx += padded.at<Vec3b>(i + m, j + n)[0] * kernelX[m][n];
                verticalImage.at<Vec3b>(i, j) = Vec3b((gx >> 16) & 255, (gx >> 8) & 255, gx & 255);

                int gy = 0;
                for (int m = 0; m < kernelX.size(); m++)
                    for (int n = 0; n < kernelX.size(); n++)
                        gy += padded.at<Vec3b>(i + m, j + n)[0] * kernelY[m][n];
                horizonImage.at<Vec3b>(i, j) = Vec3b((gy >> 16) & 255, (gy >> 8) & 255, gy & 255);

                int G = abs(gx) + abs(gy);
                tempG[i][j] = G;
                max = max < G ? G : max;
            }
        }
        
        // 正規化、二值化
        for (int i = 0; i < sourceImage.rows; i++)
            for (int j = 0; j < sourceImage.cols; j++)
            {
                int value = tempG[i][j] * 255 / max > threshold ? 255 : 0;
                bothImage.at<Vec3b>(i, j) = Vec3b(value, value, value);
            }
        return resultMap;
    }

    // padding 使用 0 填充
    Mat PadByZero(const Mat& image, int paddingSize = 1) {
        Mat padded = Mat(image.rows + paddingSize * 2, image.cols + paddingSize * 2, image.type(), Scalar(0));
        image.copyTo(padded(Rect(paddingSize, paddingSize, image.cols, image.rows)));
        return padded;
    }

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
};

// 存圖片路徑資訊
class ImageInfo 
{
public:
    uchar _sobelThreshold, _prewittThreshold, _laplacian1Threshold, _laplacian2Threshold;

    ImageInfo(string path, uchar prewittThreshold, uchar sobelThreshold, uchar laplacian1Threshold, uchar laplacian2Threshold) {
        this->_path = fs::path(path);
        this->_sobelThreshold = prewittThreshold;
        this->_prewittThreshold = sobelThreshold;
        this->_laplacian1Threshold = laplacian1Threshold;
        this->_laplacian2Threshold = laplacian2Threshold;
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
    images.push_back(ImageInfo(IMAGE_FOLDER + "House512.png", 32, 32, 10, 12));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Lena.png", 40, 40, 10, 12));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Mandrill.png", 45, 45, 12, 15));

    ImageLibrary library = ImageLibrary();

    // 定義 Laplacian Kernel
    const Kernel<int> laplacianKernel1 = { {0, 1, 0}, {1, -4, 1}, {0, 1, 0} };
    const Kernel<int> laplacianKernel2 = { {1, 1, 1}, {1, -8, 1}, {1, 1, 1} };

    for (ImageInfo imageInfo : images)
    {
        std::cout << "process " + imageInfo.Path() << std::endl;

        // 讀取圖片
        Mat sourceImage = imread(imageInfo.Path());

        // 灰階、Gaussian 過濾
        Mat grayImage = library.ConvertToGray(sourceImage);
        Mat filterImage = library.FilterBy(grayImage, ImageLibrary::FilterType::Gaussian, 3);
        
        // 邊緣偵測
        map<ImageLibrary::EdgeType, Mat> sobelResult = library.Sobel(filterImage, imageInfo._sobelThreshold);
        map<ImageLibrary::EdgeType, Mat> prewittResult = library.Prewitt(filterImage, imageInfo._prewittThreshold);
        Mat laplacianResult1 = library.Laplacian(filterImage, laplacianKernel1, imageInfo._laplacian1Threshold);
        Mat laplacianResult2 = library.Laplacian(filterImage, laplacianKernel2, imageInfo._laplacian2Threshold);
        
        // 顯示結果
        imshow(imageInfo.FileName(), sourceImage);
        imshow(imageInfo.FileName() + "_Sobel_vertical", sobelResult[ImageLibrary::EdgeType::Vertical]);
        imshow(imageInfo.FileName() + "_Sobel_horizon", sobelResult[ImageLibrary::EdgeType::Horizon]);
        imshow(imageInfo.FileName() + "_Sobel_both", sobelResult[ImageLibrary::EdgeType::Both]);
        imshow(imageInfo.FileName() + "_Prewitt_vertical", prewittResult[ImageLibrary::EdgeType::Vertical]);
        imshow(imageInfo.FileName() + "_Prewitt_horizon", prewittResult[ImageLibrary::EdgeType::Horizon]);
        imshow(imageInfo.FileName() + "_Prewitt_both", prewittResult[ImageLibrary::EdgeType::Both]);
        imshow(imageInfo.FileName() + "_Laplacian_1", laplacianResult1);
        imshow(imageInfo.FileName() + "_Laplacian_2", laplacianResult2);

        // 儲存圖片
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Sobel_vertical" + imageInfo.Extension(), sobelResult[ImageLibrary::EdgeType::Vertical]);
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Sobel_horizon" + imageInfo.Extension(), sobelResult[ImageLibrary::EdgeType::Horizon]);
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Sobel_both" + imageInfo.Extension(), sobelResult[ImageLibrary::EdgeType::Both]);
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Prewitt_vertical" + imageInfo.Extension(), prewittResult[ImageLibrary::EdgeType::Vertical]);
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Prewitt_horizon" + imageInfo.Extension(), prewittResult[ImageLibrary::EdgeType::Horizon]);
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Prewitt_both" + imageInfo.Extension(), prewittResult[ImageLibrary::EdgeType::Both]);
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Laplacian_1" + imageInfo.Extension(), laplacianResult1);
        imwrite(IMAGE_OUTPUT_FOLDER + imageInfo.FileName() + "_Laplacian_2" + imageInfo.Extension(), laplacianResult2);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    std::cout << "processing complete!" << std::endl;

    return 0;
}