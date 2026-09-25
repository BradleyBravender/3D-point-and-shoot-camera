import cv2
import numpy as np
import open3d as o3d


# ============================================================
# CAMERA CALIBRATION
# Replace these with your calibration values
# ============================================================

K1 = np.array([
    [544.908, 0.0, 436.372],
    [  0.0, 544.908, 231.858],
    [  0.0,   0.0,   1.0]
], dtype=np.float64)

K2 = np.array([
    [544.908, 0.0, 436.372],
    [  0.0, 544.908, 231.858],
    [  0.0,   0.0,   1.0]
], dtype=np.float64)

# Distortion coefficients
D1 = np.zeros(5)
D2 = np.zeros(5)

# Rotation from camera 1 to camera 2
R = np.eye(3, dtype=np.float64)

# Translation from camera 1 to camera 2.
# IMPORTANT: units here determine the units of your point cloud.
# For example, if T is in meters, the point cloud will be in meters.
T = np.array([
    [-0.0592601],
    [ 0.00],
    [ 0.00]
], dtype=np.float64)


# ============================================================
# INPUT IMAGES
# ============================================================
root = "two_view_test/tunnel_2l/"
left = cv2.imread(root + "im0.png")
right = cv2.imread(root + "im1.png")

if left is None or right is None:
    raise RuntimeError("Could not load left.png or right.png")

if left.shape != right.shape:
    raise RuntimeError("Left and right images must have the same dimensions")

height, width = left.shape[:2]


# ============================================================
# RECTIFICATION
# ============================================================

R1, R2, P1, P2, Q, roi1, roi2 = cv2.stereoRectify(
    K1,
    D1,
    K2,
    D2,
    (width, height),
    R,
    T,
    flags=cv2.CALIB_ZERO_DISPARITY,
    alpha=0
)

map1x, map1y = cv2.initUndistortRectifyMap(
    K1, D1, R1, P1,
    (width, height),
    cv2.CV_32FC1
)

map2x, map2y = cv2.initUndistortRectifyMap(
    K2, D2, R2, P2,
    (width, height),
    cv2.CV_32FC1
)

left_rect = cv2.remap(left, map1x, map1y, cv2.INTER_LINEAR)
right_rect = cv2.remap(right, map2x, map2y, cv2.INTER_LINEAR)


# ============================================================
# STEREO MATCHING
# ============================================================

gray_left = cv2.cvtColor(left_rect, cv2.COLOR_BGR2GRAY)
gray_right = cv2.cvtColor(right_rect, cv2.COLOR_BGR2GRAY)

stereo = cv2.StereoSGBM_create(
    minDisparity=0,
    numDisparities=128,       # Must be divisible by 16
    blockSize=5,

    P1=8 * 1 * 5**2,
    P2=32 * 1 * 5**2,

    disp12MaxDiff=1,
    uniquenessRatio=10,
    speckleWindowSize=100,
    speckleRange=2
)

disparity = stereo.compute(gray_left, gray_right).astype(np.float32) / 16.0


# ============================================================
# DISPARITY -> 3D
# ============================================================

points_3d = cv2.reprojectImageTo3D(disparity, Q)


# ============================================================
# FILTER INVALID POINTS
# ============================================================

valid = (
    (disparity > 0) &
    np.isfinite(points_3d[:, :, 0]) &
    np.isfinite(points_3d[:, :, 1]) &
    np.isfinite(points_3d[:, :, 2])
)

points = points_3d[valid]
colors = left_rect[valid]

# OpenCV uses BGR, Open3D expects RGB
colors = colors[:, ::-1] / 255.0


# ============================================================
# CREATE POINT CLOUD
# ============================================================

cloud = o3d.geometry.PointCloud()

cloud.points = o3d.utility.Vector3dVector(points)
cloud.colors = o3d.utility.Vector3dVector(colors)


# ============================================================
# SAVE + DISPLAY
# ============================================================

o3d.io.write_point_cloud("pointcloud.ply", cloud)

print(f"Generated {len(points):,} points")
print("Saved: pointcloud.ply")

o3d.visualization.draw_geometries([cloud])
