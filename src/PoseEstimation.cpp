#include "PoseEstimation.h"

#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>

#include "Features.h"

namespace slam::pose {

static cv::Mat cv_Rt(const cv::Mat& R, const cv::Mat& t)
{
    cv::Mat pose = cv::Mat::eye(4, 4, CV_32F);
    R.copyTo(pose(cv::Rect(0, 0, 3, 3)));
    t.copyTo(pose(cv::Rect(3, 0, 1, 3)));
    return pose;
}

static Eigen::Matrix4f recover_pose_from_essential(const cv::Mat& E,
                                                   const cv::Mat& camera_matrix,
                                                   const std::vector<cv::Point2f>& points_from,
                                                   const std::vector<cv::Point2f>& points_to,
                                                   const std::vector<u_char>& inliers)
{
    cv::Mat R1, R2, t;
    cv::decomposeEssentialMat(E, R1, R2, t);
    std::vector<cv::Mat> pose_candidates = {cv_Rt(R1, t),
                                            cv_Rt(R1, -t),
                                            cv_Rt(R2, t),
                                            cv_Rt(R2, -t)};

    int most_visible_points = 0;
    int best_pose_index = 0;
    for (int i = 0; i < pose_candidates.size(); i++) {
        cv::Mat triangulated_pts;
        cv::triangulatePoints(camera_matrix * cv::Mat::eye(3, 4, CV_32F),
                              camera_matrix * pose_candidates[i].rowRange(0, 3),
                              points_from,
                              points_to,
                              triangulated_pts);
        triangulated_pts.convertTo(triangulated_pts, CV_32F);

        for (int j = 0; j < triangulated_pts.cols; j++) {
            triangulated_pts.col(j) /= triangulated_pts.at<double>(3, j);
        }

        cv::Mat cam1_points = cv::Mat::eye(3, 4, CV_32F) * triangulated_pts;
        cv::Mat cam2_points = pose_candidates[i].rowRange(0, 3) * triangulated_pts;

        int visible_points = 0;
        for (int j = 0; j < cam1_points.cols; j++) {
            bool in_front_of_camera1 = cam1_points.at<double>(2, j) > 0;
            bool in_front_of_camera2 = cam2_points.at<double>(2, j) > 0;
            if (inliers[j] && in_front_of_camera1 && in_front_of_camera2) {
                visible_points++;
            }
        }

        if (visible_points > most_visible_points) {
            most_visible_points = visible_points;
            best_pose_index = i;
        }
    }

    Eigen::Matrix4f pose;
    cv::cv2eigen(pose_candidates[best_pose_index], pose);
    return pose;
}

PoseEstimate estimate_pose(const ExtractedFeatures& prev_features,
                           const ExtractedFeatures& features,
                           const std::vector<FeatureMatch>& matches,
                           const Camera& camera)
{
    std::vector<cv::Point2f> matched_points_from, matched_points_to;
    for (const auto& match : matches) {
        matched_points_from.push_back(prev_features.keypoints[match.train_index].pt);
        matched_points_to.push_back(features.keypoints[match.query_index].pt);
    }

    std::vector<uchar> essential_inliers;
    cv::Mat E = cv::findEssentialMat(matched_points_from,
                                     matched_points_to,
                                     cv_utils::intrinsic_mat_cv(camera),
                                     cv::RANSAC,
                                     0.999,
                                     1.0,
                                     essential_inliers);

    PoseEstimate pose_estimate;
    pose_estimate.pose = recover_pose_from_essential(E,
                                                     cv_utils::intrinsic_mat_cv(camera),
                                                     matched_points_from,
                                                     matched_points_to,
                                                     essential_inliers);
    for (int i = 0; i < matches.size(); i++) {
        if (essential_inliers[i]) {
            pose_estimate.inlier_matches.push_back(matches[i]);
        }
    }
    return pose_estimate;
}

} // namespace slam::pose