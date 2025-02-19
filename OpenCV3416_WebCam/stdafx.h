#pragma once

#ifndef _DEBUG
#pragma comment(lib, "opencv_world3416.lib")
#else
#pragma comment(lib, "opencv_world3416d.lib")
#endif

#pragma comment(lib, "winmm.lib")

#include <opencv2/opencv.hpp> // for video
#include <opencv2/core/core.hpp>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;


#include <iostream>
#include <stdio.h>

#include <string>
#include <vector>

//thread
#include <thread>
#include <mutex>

using namespace std;


#define LOWER 0
#define UPPER 1

#define MAIN_FRAME	"Camera View"
#define DEBUG_FRAME	"Morphology Mask View"

#define MAXLOGSIZE 1024

#define COLOR_MAX 255

#define M_PI 3.14159265358979323846

#define MAXBALLRADIUS 120
#define MINBALLRADIUS 35


typedef enum {
	YELLOW = 0,
	GREEN = 1,
	RED = 2,
	NUM_COLOR = 3
} Color;

//#define LOG_INFO(str1,str2) {CString str; str.Format(_T("[ >>>>>>>> %s <<<<<<<< ]  %s \n"),str1,str2); OutputDebugString(str);}
	//CString a;
	//	CString b;
	//	LOG_INFO(a, b);