#include <iostream>
#include <filesystem> // ISO C++17 標準 (/std:c++17)
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

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
    virtual Mat PadImage(const Mat& image, int paddingSize)
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
        Mat paddedImage = this->PadImage(sourceImage, this->_mask / 2);
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
        Mat paddedImage = this->PadImage(sourceImage, this->_mask / 2);
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
    // 定義 Kernel 型別
    typedef vector<vector<double>> Kernel;

    GaussianFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {
        Mat resultImage = Mat(sourceImage.size(), CV_8UC3);
        Mat paddedImage = this->PadImage(sourceImage, this->_mask / 2);
        Kernel kernel = CreateGaussianKernel(this->_mask);
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
    Kernel CreateGaussianKernel(int kernelSize, double sigma = -1)
    {
        // 未給出 sigma 自動計算
        if (sigma <= 0)
            sigma = 0.3 * ((kernelSize - 1.0) * 0.5 - 1.0) + 0.8;

        const int CENTER = kernelSize / 2;
        Kernel kernel(kernelSize, vector<double>(kernelSize, 0.0));

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

    ImageLibrary() {
        
    }

    // Filter
    Mat FilterBy(const Mat& sourceImage, FilterType filterType = FilterType::Gaussian, int mask = 3, unsigned int times = 1) {
        Mat resultImage = sourceImage;
        Filter *filter = this->CreateFilter(filterType);
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

// 存 Filter 圖片時的資訊
class FilterCase {
public:
    string _caseName;
    ImageLibrary::FilterType _filterType;
    int _mask;
    Mat _resultImage;

    FilterCase(string caseName, ImageLibrary::FilterType filterType, int mask, Mat sourceImage) {
        _caseName = caseName;
        _filterType = filterType;
        _mask = mask;
        _resultImage = sourceImage;
    }
};

int main() {
    const string IMAGE_FOLDER = "..\\image\\"; // 圖片存放資料夾
    vector<ImageInfo> images;
    
    // 圖片檔名設定
    images.push_back(ImageInfo(IMAGE_FOLDER + "House256_noise.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Lena_gray.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Mandrill_gray.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Peppers_noise.png"));

    ImageLibrary library = ImageLibrary();

    for (ImageInfo imageInfo : images)
    {
        std::cout << imageInfo.Path() << '\n';

        // 讀取圖片
        Mat sourceImage = imread(imageInfo.Path());
        imshow(imageInfo.FileName(), sourceImage);

        // Filter Case 設定
        vector<FilterCase> filterCases = { 
            FilterCase("Mean 3x3", ImageLibrary::FilterType::Mean, 3, sourceImage),
            FilterCase("Mean 7x7", ImageLibrary::FilterType::Mean, 7, sourceImage),
            FilterCase("Median 3x3", ImageLibrary::FilterType::Median, 3, sourceImage),
            FilterCase("Median 7x7", ImageLibrary::FilterType::Median, 7, sourceImage),
            FilterCase("Gaussian 5x5", ImageLibrary::FilterType::Gaussian, 5, sourceImage)
        };

        // Filter
        const int FILTER_TIMES = 7;
        for (int times = 1; times <= FILTER_TIMES; times++) {
            for (FilterCase& filterCase : filterCases) {
                filterCase._resultImage = library.FilterBy(filterCase._resultImage, filterCase._filterType, filterCase._mask);
                string caseName = imageInfo.FileName() + " " + filterCase._caseName + " times" + to_string(times);
                string saveName = IMAGE_FOLDER + caseName + imageInfo.Extension();
                imshow(caseName, filterCase._resultImage);
                imwrite(saveName, filterCase._resultImage);
            }
        }

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}