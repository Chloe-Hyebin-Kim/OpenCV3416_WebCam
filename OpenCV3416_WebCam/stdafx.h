#pragma once

#ifndef _DEBUG
#pragma comment(lib, "opencv_world3416.lib")
#else
#pragma comment(lib, "opencv_world3416d.lib")
#endif

#pragma comment(lib, "winmm.lib")




#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;

#include <iostream>
#include <stdio.h>

using namespace std;