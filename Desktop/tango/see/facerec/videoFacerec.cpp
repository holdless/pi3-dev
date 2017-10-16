#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <raspicam/raspicam_cv.h>
#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include <string>

using namespace cv;
using namespace std;

// face detection classifier
String face_cascade_name = "/usr/share/opencv/lbpcascades/lbpcascade_frontalface.xml";
String eyes_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
string window_name = "Face-recognition";


static Mat norm_0_255(InputArray _src) 
{
    Mat src = _src.getMat();
    // Create and return normalized image:
    Mat dst;
    switch(src.channels()) 
    {
    case 1:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
        break;
    case 3:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
        break;
    default:
        src.copyTo(dst);
        break;
    }
    return dst;
}

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') 
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) 
	{
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) 
	{
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) 
		{
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

enum{_COL, _ROW};
void covMat(Mat& samples, Mat& covar, Mat& mean, int option)
{
	if (option == _ROW)
	{
		calcCovarMatrix(samples, covar, mean, CV_COVAR_NORMAL | CV_COVAR_ROWS);
		covar = covar/(samples.rows - 1);
	}
	else if (option == _COL)
	{
		calcCovarMatrix(samples, covar, mean, CV_COVAR_NORMAL | CV_COVAR_COLS);
		covar = covar/(samples.cols - 1);
	}
}

void vectorMat2Mat(vector<Mat>& A, Mat& B)
{
	for (int i = 0; i < A.size(); i++)
		B.push_back(A.at(i));
}
////////////////////////////////////////////////////
/////////////////// main ///////////////////////////
////////////////////////////////////////////////////
int main(int argc, const char *argv[]) 
{	
	string fn_csv, db_csv;
	bool train_flag = true;
    // Check for valid command line arguments, print usage
    // if no arguments were given.
    if (argc < 3) 
	{		
        cout << "usage: " << argv[0] << " <csv.ext> <num_components> " << endl;
        exit(1);
    }

	if (argc == 3)
		train_flag = true;
	 
    if (argc == 4) 
	{
		train_flag = false;
		db_csv = string(argv[3]);
    }
    
    //////////////////////////////
    //// init face-recogntion ////
    //////////////////////////////

    // Get the path to your CSV.
    fn_csv = string(argv[1]);
	// Get num_components
	int num_components = atoi(argv[2]);

	
    // These vectors hold the images and corresponding labels.
    vector<Mat> images;
    vector<int> labels;
    // Read in the data. This can fail if no valid
    // input filename is given.
    try 
    {
        read_csv(fn_csv, images, labels);
    } 
    catch (cv::Exception& e) 
    {
        cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }
    // Quit if there are not enough images for this demo.
    if(images.size() <= 1) 
	{
        string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
        CV_Error(CV_StsError, error_message);
    }
    // Get the height from the first image. We'll need this
    // later in code to reshape the images to their original
    // size:
    int im_width = images[0].cols;
    int im_height = images[0].rows;

// 546-7(11/15), hiroshi: remove 
/*
    // The following lines simply get the last images from
    // your dataset and remove it from the vector. This is
    // done, so that the training data (which we learn the
    // cv::FaceRecognizer on) and the test data we test
    // the model with, do not overlap.
    Mat testSample = images[images.size() - 1];
    int testLabel = labels[labels.size() - 1];
    images.pop_back();
    labels.pop_back();
*/
	////////////////////////
	//// train database ////
	////////////////////////
	
    // The following lines create an Eigenfaces model for
    // face recognition and train it with the images and
    // labels read from the given CSV file.
    // This here is a full PCA, if you just want to keep
    // 10 principal components (read Eigenfaces), then call
    // the factory method like this:
    //
    //      cv::createEigenFaceRecognizer(10);
    //
    // If you want to create a FaceRecognizer with a
    // confidence threshold (e.g. 123.0), call it with:
    //
    //      cv::createEigenFaceRecognizer(10, 123.0);
    //
    // If you want to use _all_ Eigenfaces and have a threshold,
    // then call the method like this:
    //
    //      cv::createEigenFaceRecognizer(0, 123.0);
    //	
	Ptr<FaceRecognizer> model;
	
	if (train_flag)
	{
		if (num_components > 0)
			model = createEigenFaceRecognizer(num_components);
		else
			model = createEigenFaceRecognizer();
		// train model
		model->train(images, labels);
		// save model
		// 548.1.1123.hiroshi: save & load facerec model
		model->save("facerec_model.yml");
	}
	else
	{
		model = createEigenFaceRecognizer();
		model->load(db_csv);		
	}	
	
	// 548.1.1123.hiroshi: get model parameters from loaded model
	Mat W = model->getMat("eigenvectors");
	Mat mean = model->getMat("mean");
	vector<Mat> projs = model->getMatVector("projections");
	Mat projections;
	vectorMat2Mat(projs, projections);
	Mat evs;
	if (num_components > 0)
		evs = Mat(W, Range::all(), Range(0, num_components));
	else
		evs = W;
	
	// cal inversion of covariance matrix
	Mat cov, invcov, projmean;
	covMat(projections, cov, projmean, _ROW);
	invcov = cov.inv(DECOMP_CHOLESKY);
	
	
    /////////////////////////////
    //// init face-detection ////
    /////////////////////////////
	if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };
	if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

    /////////////////////
    //// init camera ////
    /////////////////////
    raspicam::RaspiCam_Cv Camera;
    //  raspicam setup
    Camera.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    Camera.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    Camera.set(CV_CAP_PROP_FORMAT, CV_8UC3);
    
    cv::Mat frame;
    char key;
	 
    //Open camera
    if (!Camera.open())
		return 0; 

    while (Camera.isOpened()) 
    {
        Camera.grab();
        Camera.retrieve (frame);
        flip(frame, frame, 0);
        // face detection
        vector<Rect> faces;
		Mat frame_gray;
		cvtColor( frame, frame_gray, CV_BGR2GRAY );
		equalizeHist( frame_gray, frame_gray );
		face_cascade.detectMultiScale( 	frame_gray, 
										faces, 
										1.1, 
										2, 
										0|CV_HAAR_SCALE_IMAGE, 
										Size(30, 30) );
		
		for (int i = 0; i < faces.size(); i++)
		{
            Rect face_i = faces[i];
            // Crop the face from the image. So simple with OpenCV C++:
            Mat face = frame_gray(face_i);
            // Resizing the face is necessary for Eigenfaces and Fisherfaces. You can easily
            // verify this, by reading through the face recognition tutorial coming with OpenCV.
            // Resizing IS NOT NEEDED for Local Binary Patterns Histograms, so preparing the
            // input data really depends on the algorithm used.
            //
            // I strongly encourage you to play around with the algorithms. See which work best
            // in your scenario, LBPH should always be a contender for robust face recognition.
            //
            // Since I am showing the Fisherfaces algorithm here, I also show how to resize the
            // face you have just found:
            Mat face_resized;
            cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);
            
            equalizeHist(face_resized,  face_resized);
                                imshow("resized",  face_resized);

			// Now perform the prediction, see how easy that is:
			// 1.
            int prediction = model->predict(face_resized);
            // 2.
            // 548.1.1123.hiroshi: try using loaded model parameter and determine the label myself
            Mat thisProj = subspaceProject(evs, mean, face_resized.reshape(1,1));
			float DIS(10000), MDIS(1000), SIM(0);
			int iDIS(0), iMDIS(0), iSIM(0);
            for (int i = 0; i < projections.rows; i++)
			{
				float dis = norm(thisProj, projections.row(i));
				float mdis = Mahalanobis(thisProj, projections.row(i), invcov);
				float sim = thisProj.dot(projections.row(i))/( norm(thisProj)*norm(projections.row(i)) );
				
				if (dis < DIS)
				{
					DIS = dis;
					iDIS = labels[i];
				}
				if (mdis < MDIS)
				{
					MDIS = mdis;
					iMDIS = labels[i];
				}
				if (sim < SIM)
				{
					SIM = sim;
					iSIM = labels[i];
				}
			}
			
			// And finally write all we've found out to the original image!
            // First of all draw a green rectangle around the detected face:
            rectangle(frame, face_i, CV_RGB(0, 255,0), 1);
            // Create the text we will annotate the box with:
			string box_text;
/*            if (fn_csv == "att_face_db.csv" && prediction == 30 || fn_csv == "blend_face_db.csv" && prediction ==  3)
				box_text = "Jack";
            else if (fn_csv == "blend_face_db.csv" && prediction ==  0)
				box_text = "Frances";
			else		*/
				box_text = format("Prediction = %d,%d,%d,%d", prediction, iDIS, iMDIS, iSIM);
            // Calculate the position for annotated text (make sure we don't
            // put illegal values in there):
            int pos_x = std::max(face_i.tl().x - 10, 0);
            int pos_y = std::max(face_i.tl().y - 10, 0);
            // And now put it into the image:
            putText(frame, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);			
		}
	  
        // show window
        imshow(window_name, frame);
	
		key = waitKey(1);
		if (char(key) == 27)
			break;
    }

    Camera.release();
    destroyAllWindows();
    
    return 0;
}