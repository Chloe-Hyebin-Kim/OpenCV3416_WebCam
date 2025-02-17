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
	//�ܼ� ���� ����
	void SimpleShow();

	//���� ���� �� �������� �����ϰ� ����þ� ������� ���� ��ȯ
	bool ProcessMorphGaussianHough();

	//���� ��� ���͸� �� �������� ���� �����Ͽ� ������ ��� ����
	void Run();

	//wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
	bool ESCKeyUser();

	const bool IsCameraOpened() const;

	VideoCapture& GetVideoCapture() const { return *m_pCapture; }

	void FindCandidateArea();

private:
	//Run() ����
	void ProcessFrame(Mat& outputFrame);// ī�޶� �������� �޾Ƽ� ���� ���� �� �׸� �ڽ� �׸�
	Mat Preprocess();	// ������ ���� �� ��� �� ����ũ ����
	vector<Rect> DetectBalls(const Mat& mask);	//�������� ã�� ���� �����Ͽ� �׸� �ڽ� ��ȯ

	//ProcessMorphGaussianHough() ����
	Mat Hough2d(Mat img, Mat msk);
	Mat MorphOps(Mat img, int x);
	Mat RescaleImage(Mat img, double factor);


private:
	VideoCapture* m_pCapture;  // ī�޶� ��ü
	Mat* m_pOriginFrame;       // ���� ������
};