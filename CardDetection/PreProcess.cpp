#include "stdafx.h"
#include "PreProcess.h"
#include "Image.h"


PreProcess::PreProcess(string dir)
{
	Image img = Image(dir);
	this->img = img.getImage();
	processImg();
}

PreProcess::PreProcess(Mat img)
{
	this->img = img;
	processImg();
}

PreProcess::~PreProcess()
{
}

void PreProcess::processImg() {
	Mat gray, blur, thre;

	cvtColor(img, gray, COLOR_BGR2GRAY);

	GaussianBlur(gray, blur, Size(5, 5), 2, 0);

	adaptiveThreshold(blur, thre, 255, 1, 1, 11, 1);

	GaussianBlur(thre, blur_thre, Size(5, 5), 2, 0);
}

Mat PreProcess::getProcessedImg() {
	return blur_thre;
}
