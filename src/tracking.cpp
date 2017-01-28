#include "tracking.h"

void STracking::Initialize(int image_width, int image_height, int max_corners)
{
    this->image_width = image_width;
    this->image_height = image_height;
    this->max_corners = max_corners;

    cv::RNG rng(12345);
    for(int i = 0 ; i < max_corners ; i++)
        colors.push_back(cv::Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255)));

}

cv::Rect STracking::GetTrackingROI()
{
    return tracking_roi;
}

void STracking::SetTrackingROI(cv::Rect roi)
{
    this->SetTrackingROI(roi.x, roi.y, roi.width, roi.height);
}

void STracking::SetTrackingROI(int x, int y, int width, int height)
{
    tracking_roi.x = MIN(MAX(x,0), image_width);
    tracking_roi.y = MIN(MAX(y,0), image_height);
    tracking_roi.width = MIN(width, image_width - tracking_roi.x);
    tracking_roi.height = MIN(height, image_height - tracking_roi.y);
}
