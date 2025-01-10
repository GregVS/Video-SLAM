#include "Slam.h"
#include "TestHelpers.h"
#include "Visualization.h"

int main()
{
    auto test_data = load_test_data();
    slam::Slam slam(test_data.video_loader, test_data.camera, test_data.static_mask);
    slam.initialize();

    // Display the map matches
    slam::Visualization visualization("Map Matching");
    visualization.initialize();
    visualization.run_threaded();

    while (!visualization.has_quit()) {
        // Draw the camera poses
        visualization.set_camera_poses(slam.poses());

        // Draw the map points
        std::vector<Eigen::Vector3f> points;
        for (const auto& point : slam.map()) {
            points.push_back(point.position());
        }
        visualization.set_points(points);

        // Draw the keypoints and map matches on the frame
        auto const& frame = slam.frame();
        cv::Mat render = frame->image().clone();
        std::vector<cv::KeyPoint> keypoints;
        for (const auto& match : frame->map_matches()) {
            auto feature = frame->keypoint(match.keypoint_index);
            keypoints.push_back(feature);
        }
        cv::drawKeypoints(render,
                          frame->features().keypoints,
                          render,
                          cv::Scalar(255, 0, 0),
                          cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        cv::drawKeypoints(render,
                          keypoints,
                          render,
                          cv::Scalar(0, 255, 0),
                          cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        visualization.set_image(render);

        // Next frame
        visualization.wait_for_keypress();
        slam.step();
    }

    return 0;
}