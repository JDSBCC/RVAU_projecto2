#pragma once

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

class PreProcess
{
	Mat img;
	Mat blur_thre;
public:
	PreProcess(string dir);
	PreProcess(Mat img);
	~PreProcess();

	void processImg();
	Mat getProcessedImg();
};

