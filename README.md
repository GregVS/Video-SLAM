# Video-SLAM
Monocular video SLAM implementation in C++.

## Libraries
- Uses OpenCV for ORB feature extraction, matching, and triangulation (will probably replace with my own triangulation later)
- OpenGL for rendering 3D point cloud and poses

## Roadmap:
- [X] Feature extraction and matching
- [X] Point cloud triangulation and matching
- [X] Essential matrix estimation with RANSAC
- [ ] Replace OpenCV triangulation with my own triangulation
- [ ] Replace OpenGL with Pangolin or something nice
- [ ] Bundle adjustment/pose graph optimization

## Dependencies
- OpenCV
- OpenGL (should be provided with linux via mesa)
- GLM
- GLFW
- G2O
- Eigen

For Linux:
```
sudo apt-get install libopencv-dev libglfw3-dev libglm-dev libeigen3-dev libsuitesparse-dev
```