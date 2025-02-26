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
	// ���� ��� ���� (���� ���� ����)
	static int m_i32BlockSize;  // �Ӱ谪�� ����� ��� ũ�� (Ȧ��, ���� ũ�� ���� ������ ���) //�⺻�� 11
	static int m_i32Constant;          //���� �Ӱ谪���� �����ϴ� ������ (���� ũ�� ��ο���)        // �⺻�� 2

	static int m_i32KernelSize;  // �ʱ� Ŀ�� ũ�� 15

	// �̹��� �����͸� ������ ���� ���
	static Mat m_GrayFrame;
	static Mat m_BinaryFrame;
	static Mat m_TophatFrame;

public:
	static void OnUpdateAdaptiveThreshold(int, void*);
	static void OnUpdateTopHat(int, void*);

public:
	//�ܼ� ���� ����
	void SimpleShow();
	int CustomProcess0();

	//���� ���� �� �������� �����ϰ� ����þ� ������� ���� ��ȯ
	bool ProcessMorphGaussianHough();
	bool CustomProcess();
	int CustomProcess2();

	bool CustomAdaptiveProcess();

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

	CirInfo CalcRadiusCenter(vector<Point>& Contour, Point pt1, int nNum);
	CirInfof CalcRadiusCenterSub(vector<Point> Contour, Point2f pt1, float nRadius, int nNum);
	void FindCircle(Mat imgSrc, unsigned char Thres, CirInfo& circle);

	bool HistogramStretch(Mat& imgSrc, Mat& imgDst);

	void ShowResult(const Mat& inFrame, Mat& outFrame);
	void DrawBoundingBox(Mat& targetFrame, const vector<Rect>& boundingBoxes);
	vector<Rect> DetectBallArea(const Mat& targetFrame);

private:
	VideoCapture* m_pCapture;  // ī�޶� ��ü
	Mat* m_pOriginFrame;       // ���� ������
};