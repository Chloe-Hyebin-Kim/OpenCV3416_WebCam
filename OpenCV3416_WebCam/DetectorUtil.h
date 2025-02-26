#pragma once

#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <vector>

typedef struct {
	cv::Point center;
	int radius;
	int AccumPt;
}CirInfo;

typedef struct
{
	cv::Point pt;
	int nAccum;
	cv::Rect rt;
}BallCounter;

typedef struct {
	cv::Point2f center;
	float radius;
	int AccumPt;
}CirInfof;

typedef struct {
	double dSize;
	double dCircle_ratio;
	int nAvr;
	int nAvrSq;
	float fWHR;
	float fBallScore;
	cv::Rect BallRect;
	CirInfo BallData;
}BallDecisionDat;

class DetectorUtil
{
public:
	DetectorUtil();
	~DetectorUtil();

public:
	// 정적 멤버 변수 (전역 변수 역할)
	static int m_i32BlockSize;  // 임계값을 계산할 블록 크기 (홀수, 값이 크면 넓은 영역을 고려) //기본값 11
	static int m_i32Constant;          //계산된 임계값에서 조정하는 보정값 (값이 크면 어두워짐)        // 기본값 2

	static int m_i32KernelSize;  // 초기 커널 크기 15

	// 이미지 데이터를 저장할 정적 멤버
	static Mat m_GrayFrame;
	static Mat m_BinaryFrame;
	static Mat m_TophatFrame;

public:
	static void OnUpdateAdaptiveThreshold(int, void*);
	static void OnUpdateTopHat(int, void*);

public:
	//단순 영상 송출
	void SimpleShow();
	int CustomProcess0();

	//색상 분할 후 모폴로지 적용하고 가우시안 블러사용후 허프 변환
	bool ProcessMorphGaussianHough();
	bool CustomProcess();
	int CustomProcess2();

	bool CustomAdaptiveProcess();

	//색상 기반 필터링 후 모폴로지 연산 적용하여 윤곽선 기반 검출
	void Run();

	//wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
	bool ESCKeyUser();

	const bool IsCameraOpened() const;

	VideoCapture& GetVideoCapture() const { return *m_pCapture; }

	void FindCandidateArea();

private:
	//Run() 전용
	void ProcessFrame(Mat& outputFrame);// 카메라 프레임을 받아서 공을 검출 후 네모 박스 그림
	Mat Preprocess();	// 노이즈 제거 및 흰색 공 마스크 생성
	vector<Rect> DetectBalls(const Mat& mask);	//윤곽선을 찾고 공을 검출하여 네모 박스 반환

	//ProcessMorphGaussianHough() 전용
	Mat Hough2d(Mat img, Mat msk);
	Mat MorphOps(Mat img, int x);
	Mat RescaleImage(Mat img, double factor);

	CirInfo CalcRadiusCenter(vector<Point>& Contour, Point pt1, int nNum);
	CirInfof CalcRadiusCenterSub(vector<Point> Contour, Point2f pt1, float nRadius, int nNum);
	void FindCircle(Mat imgSrc, unsigned char Thres, CirInfo& circle);

	bool HistogramStretch(Mat& imgSrc, Mat& imgDst);

	void ShowResult(const Mat& inFrame, Mat& outFrame);
	void DrawBoundingBox(Mat& targetFrame, const vector<Rect>& boundingBoxes);
	vector<Rect> DetectBallArea(const Mat& targetFrame);

private:
	VideoCapture* m_pCapture;  // 카메라 객체
	Mat* m_pOriginFrame;       // 원본 프레임
};