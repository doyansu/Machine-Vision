#include<opencv2/opencv.hpp>

using namespace cv;

int main() {
	const std::string path = "../image/House256.png";
	Mat img = imread(path, -1);
	if (img.empty()) return -1;
	namedWindow("HelloCV", WINDOW_AUTOSIZE);
	imshow("HelloCV", img);
	waitKey(0);
	destroyWindow("HelloCV");
	return 0;
}