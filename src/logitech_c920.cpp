#include "webcam.h"

Webcam::Webcam(){}

Webcam::~Webcam(){}

bool Webcam::Initialize(int camera_number)
{
    if(number != -1)
        return false;
    number = camera_number;

    if(camera.open(number) == false)
        return false;
    //Initilize Webcam with default property
    camera.set(PROPERTY::WIDTH, 640);
    camera.set(PROPERTY::HEIGHT, 480);
    camera.set(PROPERTY::BRIGHT, 128);
    camera.set(PROPERTY::CONTRAST, 128);
    camera.set(PROPERTY::COLOR_INT, 128);
    camera.set(PROPERTY::GAIN, 64);
    camera.set(PROPERTY::EXPOSURE, -3);
    camera.set(PROPERTY::WB, 4250);
    camera.set(PROPERTY::FOCUS, 0);

    cv::Mat dummy;
    Grab(dummy);

    return true;
}

bool Webcam::Grab(cv::Mat &image)
{
    if(camera.read(image) == false)
        return false;
    else
        return true;
}

void Webcam::GetProperty(PROPERTY property, double &value)
{
    if(number == -1)
        return;
    value = camera.get(property);

    return;
}

void Webcam::SetProperty(PROPERTY property, double value)
{
    if(number == -1)
        return;
    camera.set(property, value);

    return;
}
