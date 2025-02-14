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
//#include <opencv2/tracking.hpp>

using namespace cv;

#include <iostream>
#include <stdio.h>

using namespace std;

#define MAIN_FRAME	"Camera View"
#define DEBUG_FRAME	"Morphology Mask View"

#define MAXLOGSIZE 1024

#define M_PI 3.14159265358979323846

#define MAX_BALLRADIUS 120
#define MIN_BALLRADIUS 20