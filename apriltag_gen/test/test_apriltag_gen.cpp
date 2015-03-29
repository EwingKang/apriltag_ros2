#include <iostream>
#include <opencv2/highgui/highgui.hpp>

int main(int argc, char** argv) {
  unsigned long long tag_code = 0x231bLL;
  int tag_bits = 4;
  int black_border = 2;
  std::cout << "tag_code: " << tag_code << std::endl;
  std::cout << "2 ** tag_bits: " << (1 << tag_bits) << std::endl;

  cv::Mat tag_matrix = cv::Mat::zeros(tag_bits + black_border * 2,
                                      tag_bits + black_border * 2, CV_8UC1);
  for (int i = 0; i < tag_bits; ++i) {
    uchar* p = tag_matrix.ptr<uchar>(i + black_border);
    for (int j = 0; j < tag_bits; ++j) {
      int shift = tag_bits * (tag_bits - i) - j - 1;
      std::cout << shift << std::endl;
      if (tag_code & (1 << shift)) {
        p[j + black_border] = 255;
      }
    }
  }
  std::cout << tag_matrix << std::endl;
  //  cv::transpose(tag_matrix, tag_matrix);
  cv::namedWindow("tag", CV_WINDOW_KEEPRATIO | CV_WINDOW_NORMAL);
  cv::imshow("tag", tag_matrix);
  cv::waitKey(-1);
}
