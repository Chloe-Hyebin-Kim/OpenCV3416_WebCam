#pragma once

#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <vector>


class DetectorUtil
{
public:
	DetectorUtil();
	~DetectorUtil();

public:
	//단순 영상 송출
	void SimpleShow();

	//색상 분할 후 모폴로지 적용하고 가우시안 블러사용후 허프 변환
	bool ProcessMorphGaussianHough();

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


private:
	VideoCapture* m_pCapture;  // 카메라 객체
	Mat* m_pOriginFrame;       // 원본 프레임
};