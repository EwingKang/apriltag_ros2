#include "apriltag_ros/apriltag_detection.h"

#include <opencv2/highgui/highgui.hpp>

#define CV_RED CV_RGB(255, 0, 0)
#define CV_GREEN CV_RGB(0, 255, 0)
#define CV_BLUE CV_RGB(0, 0, 255)
#define CV_MAGENTA CV_RGB(255, 0, 255)

namespace apriltag_ros {

ApriltagDetection::ApriltagDetection(const AprilTags::TagDetection& td)
    : id(td.id), hamming(td.hammingDistance) {
  c[0] = td.cxy.first;
  c[1] = td.cxy.second;
  for (size_t i = 0; i < 4; ++i) {
    p[i][0] = td.p[i].first;
    p[i][1] = td.p[i].second;
  }
}

ApriltagDetection::ApriltagDetection(const apriltag_detection_t* td)
    : id(td->id), hamming(td->hamming) {
  c[0] = td->c[0];
  c[1] = td->c[1];
  ///@note: the order is different
  for (size_t i = 0; i < 4; ++i) {
    p[i][0] = td->p[3 - i][0];
    p[i][1] = td->p[3 - i][1];
  }
}

ApriltagDetection::operator apriltag_msgs::Apriltag() const {
  apriltag_msgs::Apriltag apriltag;
  apriltag.id = id;
  apriltag.hamming = hamming;
  apriltag.center.x = c[0];
  apriltag.center.y = c[1];
  for (size_t i = 0; i < 4; ++i) {
    geometry_msgs::Point point;
    point.x = p[i][0];
    point.y = p[i][1];
    point.z = 1;
    apriltag.corners.push_back(point);
    if (estimate) {
      apriltag.estimate = estimate;
      apriltag.pose.position.x = t(0);
      apriltag.pose.position.y = t(0);
      apriltag.pose.position.z = t(0);
      apriltag.pose.orientation.w = q.w();
      apriltag.pose.orientation.x = q.x();
      apriltag.pose.orientation.y = q.y();
      apriltag.pose.orientation.z = q.z();
    }
  }

  return apriltag;
}

void ApriltagDetection::Estimate(const cv::Matx33d& K,
                                 const cv::Mat_<double>& D, double tag_size) {
  assert(tag_size > 0);
  const auto s = tag_size / 2.0;

  // tag corners in tag frame
  std::vector<cv::Point3d> p_tag = {
      {-s, -s, 0}, {s, -s, 0}, {s, s, 0}, {-s, s, 0}};

  // pixels coordinates in image frame
  std::vector<cv::Point2d> p_img = {{p[0][0], p[0][1]},
                                    {p[1][0], p[1][1]},
                                    {p[2][0], p[2][1]},
                                    {p[3][0], p[3][1]}};

  // Estimate r and t
  cv::Matx13d rvec, tvec;
  cv::solvePnP(p_tag, p_img, K, D, rvec, tvec);
  t = Eigen::Vector3d(tvec(0), tvec(1), tvec(2));
  Eigen::Vector3d r(rvec(0), rvec(1), rvec(2));

  // Convert r to quat
  const auto angle = r.norm();
  Eigen::Vector3d axis;
  if (angle > std::numeric_limits<double>::epsilon() * 10) {
    axis = r / angle;
  }
  q = Eigen::AngleAxis<double>(angle, axis);

  estimate = true;
}

void ApriltagDetection::Draw(cv::Mat& image, int thickness) const {
  DrawLine(image, 0, 1, CV_RED, thickness);
  DrawLine(image, 0, 3, CV_GREEN, thickness);
  DrawLine(image, 2, 3, CV_BLUE, thickness);
  DrawLine(image, 1, 2, CV_BLUE, thickness);

  cv::putText(image, std::to_string(id), cv::Point2f(c[0] - 5, c[1] + 5),
              cv::FONT_HERSHEY_SIMPLEX, 1, CV_MAGENTA, 2);
}

void ApriltagDetection::DrawLine(cv::Mat& image, int b, int e,
                                 const cv::Scalar& color, int thickness) const {
  cv::line(image, cv::Point2f(p[b][0], p[b][1]), cv::Point2f(p[e][0], p[e][1]),
           color, thickness);
}

}  // apriltag_ros