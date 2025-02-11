#include "stdafx.h"

int main(int ac, char** av)
{
	int i32Rst = 0;

	VideoCapture vCapture(0);
	vCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
	vCapture.set(CAP_PROP_FRAME_HEIGHT, 1080);

	if (!vCapture.isOpened())
	{	// isOpened() : 카메라 정보가 있는지 없는지 (1: 있다, 0: 없다)
		printf("Can't open the camera");

		i32Rst = -1;
	}
	else
	{
		//Mat img = imread("Lenna.png"); //자신이 저장시킨 이미지 이름이 입력되어야 함, 확장자까지
		//imshow("img", img);
		//waitKey(0);
		//return 0;

		Mat img;

		while (true) {
			vCapture >> img;

			imshow("camera img", img);

			if (waitKey(1) == 27)
				break;//esc 키 입력시 종료
		}
	}

	return i32Rst;
}