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

    void SetMask(int mask) {
        // 確保 mask 至少為3，且是奇數
        this->_mask = (mask < 3) ? 3 : (mask | 1);
    }

    virtual Mat FilterImage(const Mat& sourceImage) = 0;

protected:
    int _mask = 3;

    // padding 填充圖片
    virtual Mat PadImage(const Mat& image, int paddingSize)
    {
        int height = image.rows;
        int width = image.cols;
        Mat padded = Mat(height + paddingSize * 2, width + paddingSize * 2, image.type());

        // 將原始圖像複製到新的 Mat 對像中心區域
        image.copyTo(padded(cv::Rect(paddingSize, paddingSize, width, height)));

        // 複製邊界像素值
        copyMakeBorder(image, padded, paddingSize, paddingSize, paddingSize, paddingSize, BORDER_REPLICATE);

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
    GaussianFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {
        Mat resultImage = Mat(sourceImage.size(), CV_8UC3);
        return resultImage;
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

        // 顯示並儲存圖片
        auto showWriteImage = [extension = imageInfo.Extension()](string name, Mat image) {
            imshow(name, image);
            imwrite(name + extension, image);
        };

        // Filter
        const int FILTER_TIMES = 7;
        for (int times = 1; times <= FILTER_TIMES; times++) {
            showWriteImage(imageInfo.FileName() + " Mean mask3 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Mean, times, 3));
            showWriteImage(imageInfo.FileName() + " Mean mask7 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Mean, times, 7));
            showWriteImage(imageInfo.FileName() + " Median mask3 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Median, times, 3));
            showWriteImage(imageInfo.FileName() + " Median mask7 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Median, times, 7));
            showWriteImage(imageInfo.FileName() + " Gaussian mask5 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Gaussian, times, 5));
        }

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}