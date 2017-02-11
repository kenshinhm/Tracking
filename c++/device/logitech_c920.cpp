#include "logitech_c920.hpp"

C920::C920()
{
    number = -1;
}

C920::~C920(){}

bool C920::Initialize(int camera_number, int width, int height)
{
    if(number != -1)
        return false;
    number = camera_number;

    if(camera.open(number) == false)
        return false;
    //Initilize Webcam with default property
    camera.set(WIDTH, width);
    camera.set(HEIGHT, height);
    camera.set(BRIGHT, 128);
    camera.set(CONTRAST, 128);
    camera.set(COLOR_INT, 128);
    camera.set(GAIN, 64);
    camera.set(EXPOSURE, -3);
    camera.set(WHITE_BALANCE, 4250);
    camera.set(FOCUS, 0);

    cv::Mat dummy;
    Grab(dummy);

    return true;
}

bool C920::Grab(cv::Mat &image)
{
    if(camera.read(image) == false)
        return false;
    else
        return true;
}

double C920::GetProperty(PROPERTY property)
{
    return camera.get(property);
}

void C920::SetProperty(PROPERTY property, double value)
{
    if(number == -1)
        return;
    camera.set(property, value);

    return;
}
