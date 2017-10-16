#include <stdio.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>
using namespace cv;
using namespace cv::dnn;
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream> // stringstream
#include <iomanip> // setprecision
#include <time.h>
using namespace std;

class IhsorihGoogLeNetObjectRecognizer {
public:
	void setImage(cv::Mat img) {
		_img = img;
	}
	/* Find best class for the blob (i. e. class with maximal probability) */
	void getNthClass(int rank, dnn::Blob &probBlob, int *classId, double *classProb)
	{
		Mat probMat = probBlob.matRefConst().reshape(1, 1); //reshape the blob to 1x1000 matrix
		// 701.5.0106 hiroshi: sort prob. <<<
		Mat sortedProb, sortedIndex;
		cv::sortIdx(probMat, sortedIndex, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
		//	cv::sort(probMat, sortedProb, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);

		cv::Size2d s = sortedIndex.size();
		classId[rank] = sortedIndex.at<int>(s.width - 1 - rank);
		classProb[rank] = probMat.at<float>(0, classId[rank]);
		// 701.5.0106 hiroshi >>>
	}

	void getMaxClass(dnn::Blob &probBlob, int *classId, double *classProb)
	{
		Mat probMat = probBlob.matRefConst().reshape(1, 1); //reshape the blob to 1x1000 matrix
		Point classNumber;
		minMaxLoc(probMat, NULL, classProb, NULL, &classNumber);
		*classId = classNumber.x;
	}

	std::vector<String> readClassNames(const char* _synsetPath)
	{
		std::vector<String> classNames;
		std::ifstream fp(_synsetPath);
		if (!fp.is_open())
		{
			std::cerr << "File with classes labels not found: " << _synsetPath << std::endl;
			exit(-1);
		}
		std::string name;
		while (!fp.eof())
		{
			std::getline(fp, name);
			if (name.length())
				classNames.push_back(name.substr(name.find(' ') + 1));
		}
		fp.close();
		return classNames;
	}

	void putProbBar(cv::Mat img) {

		// init parameters
		cv::Point2d ULbase = Point(0, .75*img.rows);
		double fontScale = 0.5;
		int d = 1. / 32 * img.rows; // drop
		int w = 1. / 32 * img.rows; // barWidth
		int l = 3. / 8 * img.cols; // barFullLength
		int s0 = 1. / 32 * img.cols; // shift0
		int s1 = 1. / 4 * img.cols; //


		cv::Point2d barUL[3], barDR[3];
		cv::Scalar bgr[3];
		bgr[0] = Scalar(230, 0, 0);
		bgr[1] = Scalar(0, 230, 0);
		bgr[2] = Scalar(0, 0, 230);

		stringstream ss[3];
		string str[3];

		for (int i = 0; i < 3; i++) {
			barUL[i] = Point(ULbase.x + s0, ULbase.y + i * w + (i + 1)*d);
			barDR[i] = Point(barUL[i].x + l*_classProb[i], barUL[i].y + w);

			cv::rectangle(img, barUL[i], barDR[i], bgr[i], -1, 8);

			ss[i] << std::fixed << std::setprecision(1) << _classProb[i] * 100;
			str[i] = ss[i].str() + "%";
			cv::putText(img, str[i], Point(barDR[i].x + s0, barDR[i].y),
				FONT_HERSHEY_SIMPLEX, fontScale, bgr[i]);

			cv::putText(img, _className[i], Point(barDR[i].x + s1, barDR[i].y),
				FONT_HERSHEY_SIMPLEX, fontScale, bgr[i]);

		}
	}

	void init(string modelTxt, string modelBin, const char* synsetPath) {
		//ocl::setUseOpenCL(false);

		cv::dnn::initModule();  //Required if OpenCV is built as static libs
		_nameList = readClassNames(synsetPath);

		_net = dnn::readNetFromCaffe(modelTxt, modelBin);
		if (_net.empty())
		{
			std::cerr << "Can't load network by using the following files: " << std::endl;
			std::cerr << "prototxt:   " << modelTxt << std::endl;
			std::cerr << "caffemodel: " << modelBin << std::endl;
			std::cerr << "bvlc_googlenet.caffemodel can be downloaded here:" << std::endl;
			std::cerr << "http://dl.caffe.berkeleyvision.org/bvlc_googlenet.caffemodel" << std::endl;
			exit(-1);
		}
	}

	void predict() {
		if (_img.empty())
		{
			std::cerr << "Empty frame... " << std::endl;
			exit(-1);
		}
	int start;
	double duration;
	start = clock();
		resize(_img, _img, Size(224, 224));                   //GoogLeNet accepts only 224x224 RGB-images
		dnn::Blob inputBlob = dnn::Blob::fromImages(_img);   //Convert Mat to dnn::Blob batch of images
		_net.setBlob(".data", inputBlob);        //set the network input
		_net.forward();                          //compute output
		dnn::Blob prob = _net.getBlob("prob");   //gather output of "prob" layer
	duration = (clock() - start)/(double)CLOCKS_PER_SEC;
	std::cout<<endl<<"prediction: "<<duration<< "s";

		//	getMaxClass(prob, &classId[0], &classProb[0]);//find the best class
		for (int i = 0; i < 3; i++) {
			getNthClass(i, prob, _classId, _classProb);
			_className[i] = _nameList.at(_classId[i]);
			std::cout << i + 1 << " - class: #" << _classId[i] << " '" << _className[i] << "'" << std::endl;
			std::cout << "Prob.: " << _classProb[i] * 100 << "%" << std::endl;
		}

	}
private:
	int _classId[3]; // 701.5.0106 hiroshi: change from only 1 ranking to 1,2,3 ranking
	double _classProb[3];
	string _className[3];
	Net _net;
	Mat _img;
	std::vector<String> _nameList;
};