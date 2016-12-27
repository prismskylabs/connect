#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <boost/move/unique_ptr.hpp>
#include <boost/chrono/chrono.hpp>
#include "client.h"
#include "curl/curl.h"
#include "easylogging++.h"
#include "rapidjson/document.h"
#include "util.h"

_INITIALIZE_EASYLOGGINGPP
//#include "api/client.h"
//#include "api/response.h"
//#include "api/environment.h"
//#include "processors/track.h"
//#include "processors/track-collector.h"
//#include "processors/object-stream.h"
//#include "util/time.h"
//#include <chrono>

using namespace cv;

using boost::movelib::unique_ptr;
using std::string;

//Motion detection parameters
const int           MIN_AREA = 3000;
const int           KERNEL_SIZE = 7;
const int           dilation_size = 5;
const int           resize_to_height = 480;
const double        SIGMA = 4.5;

//
const int           FLIPBOOK_FPS = 1;
const Size          FLIPBOOK_SIZE = Size(480,360);
const int           FLIPBOOK_UPDATE_S = 60;
const char*         FLIPBOOK_TMP_FILE = "flip.mp4";
const int           BACKGROUND_UPDATE_MIN = 1;
const char*         BACKGROUND_TMP_FILE = "back.jpg";
const char*         BLOB_TMP_FILE = "blob.jpg";
const int           EVENT_UPDATE_MIN = 1;

unique_ptr<prism::connect::Client> client;
//prism::connect::api::Client client{prism::connect::api::environment::ApiRoot(),
//	                                       prism::connect::api::environment::ApiToken()};
unique_ptr<prism::connect::Instrument> this_camera;
unique_ptr<VideoWriter> writer;
//unique_ptr<prism::connect::processors::Track>  track;


Rect resizeRect(Rect r,float scale)
{
    return Rect(r.x*scale,r.y*scale,r.width*scale,r.height*scale);
}

//class Event{
//public:
//	void AddTimestamp(const std::chrono::system_clock::time_point& timestamp)
//	{
//		nlohmann::json record;
//		record["timestamp"] = prism::connect::util::IsoTime(timestamp);
//		data_.push_back(record);
//	}

//	nlohmann::json ToJson(){
//		return data_;
//	}
//private:
//	nlohmann::json data_;
//};

//unique_ptr<Event>  event;

//void initPrismService(std::string camera_name)
//{
//	auto accounts = client.QueryAccounts();

//	std::cout<<"API Endpoint"<<prism::connect::api::environment::ApiRoot()<<std::endl;
//	for (const auto& account : accounts) {
//	        std::cout << "Account[" << account.id_ << "]:" << std::endl;
//	        std::cout << "Name: " << account.name_ << std::endl;
//	        std::cout << "Url: " << account.url_ << std::endl;
//	        std::cout << "Instruments Url: " << account.instruments_url_ << std::endl;
//	        std::cout << std::endl;

//	        // Get the list of Instruments belonging to an Account
//	        std::vector<prism::connect::api::Instrument> instruments = client.QueryInstruments(account);

//	        //Find our camera by name
//	        bool found = false;
//	        for(auto& instrument : instruments)
//	        {
//	        	if(instrument.name_ == camera_name)
//	        	{
//	        		found = true;
//	        		this_camera.reset(new prism::connect::api::Instrument(instrument));
//	        	}
//                else
//                    std::cout << "Camera: " << instrument.name_ << std::endl;
//	        }

//	        //If camera does not exist - create it
//	        if(!found)
//	        {
//	        	this_camera.reset(new prism::connect::api::Instrument());
//	        	this_camera->name_ = camera_name;
//	        	this_camera->instrument_type_ = "camera";

//	        	// Register an unregistered Instrument to an Account
//	        	prism::connect::api::Response result = client.RegisterInstrument(account, *this_camera.get());
//                std::cerr << "Camera registration status " << result.status_code
//                          << " {" << result.text << "}" << std::endl;
//	        }
//	        std::cout << "Instrument[" << this_camera->name_ << "]:" << std::endl;

//	        break;
//	}
//}

struct CurlGlobal {
    CurlGlobal() { curl_global_init(CURL_GLOBAL_ALL); }
    ~CurlGlobal() { curl_global_cleanup(); }
};

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage:\n\tyt_camera <camera-name> <input-file>\n" << std::endl;
        return -1;
    }

    string cameraName(argv[1]);
    string inputFile(argv[2]);

    LINFO << "App name: " << argv[0];
    LINFO << "Camera name: " << cameraName;
    LINFO << "Input file: " << inputFile;

//    std::string cmd, stream_id, stream_URL, FormatCode;
//    std::vector<int> compression_params;
//    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
//    compression_params.push_back(95);
    
//    char* camera_name = argv[1];

    const char* envBuf = std::getenv("API_ROOT");

    if (envBuf == nullptr) {
        LERROR << "API_ROOT environment variable is undefined";
        return -1;
    }

    string apiRoot(envBuf);

    envBuf = std::getenv("API_TOKEN");

    if (envBuf == nullptr) {
        LERROR << "API_TOKEN environment variable is undefined";
        return -1;
    }

    string token(envBuf);

    LINFO << "API root: " << apiRoot;
    LINFO << "Token: " << token;
    CurlGlobal cg;

    prism::connect::Client client(apiRoot, token);
    prism::connect::status_t status = client.init();

    LINFO << "client.init(): " << status;

//    prism::connect::Account account;
//    status = client.queryAccount(100382, account);

//    LINFO << "client.queryAccount(100382): " << status;
//    LINFO << account.name;

    prism::connect::InstrumentsList instruments;
    status = client.queryInstrumentsList(100382, instruments);

//    prism::connect::AccountsList accounts;
//    client.queryAccountsList(accounts);

//    LINFO << "Accounts #: " << accounts.size();

    //Init connect service
//    initPrismService(camera_name);

//    //Open video stream
//    char* fname = argv[2];
//    std::cerr<<fname;
//    VideoCapture cap(fname);
//    if(!cap.isOpened()) // check if we succeeded
//        return -1;
//    int fps = cap.get(CV_CAP_PROP_FPS);
    
//    std::cout << "Processing..." << std::endl;
    



//    // create Background Subtractor objects
    
//    Mat fgMaskMOG2; // fg mask fg mask generated by MOG2 method
//    Ptr<BackgroundSubtractor> pMOG2; // MOG2 Background subtractor
//    pMOG2 = createBackgroundSubtractorMOG2(); // MOG2 approach
    
//    Mat firstFrame;
//    int fnum = 0;
//    std::chrono::system_clock::time_point ftime;
//    int last_fnum = -10000;
//    int saved_frames = 0;
//    std::chrono::system_clock::time_point flipbook_start_time;
//    bool motion = false;
//    bool was_motion = false;
//    std::chrono::system_clock::time_point  last_b_update_time;
//    std::chrono::system_clock::time_point  last_event_update_time;
//    std::chrono::system_clock::time_point  motion_start;
//    int blob_id = 0;
//    int last_blob_fnum = -10000;


//    //Set current timepoint to 1h ago since we processing faster than real time
//    //and will be posting to future
//    ftime = std::chrono::system_clock::now() - std::chrono::hours(1);
//    for(;;)
//    {
//        Mat frame, gray_frame;

//        cap >> frame; // get a new frame from camera
//        //get frame timestamp
//        ftime += std::chrono::milliseconds(1000 / fps);

//        if (frame.empty())
//            {
//                // reach to the end of the video file

//        	    if(writer.get()) //if we were writing something
//        	    {
//        	       //finalize stream, upload data
//        	       prism::connect::api::Response result;
//        	       std::cout<<"Close file:"<<FLIPBOOK_TMP_FILE<<std::endl;
//        	       writer->release();
//        	       last_fnum =-1;
//        	       //Post flipbook
//        	       std::cerr<<"Posting flipbook file "<<FLIPBOOK_TMP_FILE<<std::endl;
//        	       result = client.PostVideoFileFlipbook(*this_camera.get(),flipbook_start_time ,ftime,FLIPBOOK_SIZE.width,FLIPBOOK_SIZE.height,saved_frames,FLIPBOOK_TMP_FILE);
//        	       std::cerr<<" status"<<result.status_code<<" {"<<result.text<<"}"<<std::endl;

//        	       Event event;
//        	       //Write event timestamp
//        	       std::chrono::system_clock::time_point rounded_to_min = std::chrono::time_point_cast<std::chrono::minutes>(ftime);
//        	       event.AddTimestamp(rounded_to_min);
//        	       //Post Event
//        	       std::cerr<<"Posting event"<<std::endl;
//        	       result = client.PostTimeSeriesEvents(*this_camera.get(),ftime,event.ToJson());
//        	       std::cerr<<" status"<<result.status_code<<" {"<<result.text<<"}"<<std::endl;
//        	       last_event_update_time = ftime;
//        	    }

//                break;
//            }
        

//        cvtColor(frame, gray_frame, COLOR_BGR2GRAY); //switch to grayscale
        
//        float scale =(float) resize_to_height/gray_frame.rows;
//        int resized_width = gray_frame.cols*scale;
//        resize(gray_frame, gray_frame, Size(resized_width,resize_to_height), 0, 0, CV_INTER_CUBIC); // resize
        
//        GaussianBlur(gray_frame, gray_frame, Size(KERNEL_SIZE,KERNEL_SIZE), SIGMA, SIGMA);
//        pMOG2->apply(gray_frame, fgMaskMOG2);
        
//        Mat dilated;
//        Mat element = getStructuringElement( MORPH_ELLIPSE,
//                                            Size( 2*dilation_size + 1, 2*dilation_size+1 ),
//                                            Point( dilation_size, dilation_size ) );
//        dilate(fgMaskMOG2,dilated,element); // dilate the thresholded image to fill in holes, then find contours on thresholded image
        
//        std::vector<std::vector<Point> > contours;
//        std::vector<Vec4i> hierarchy;
//        findContours( dilated, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0) ); //find contours
        
//        motion = false;
//        for( int i = 0; i< contours.size(); i++ ) // loop over the contours
//        {
//            double area = contourArea(contours[i]);
//            if(area < MIN_AREA)
//                continue;
            
//            motion = true;
//            CvRect r = boundingRect(contours[i]);
//            r = resizeRect(r,1.0/scale);

//            //Mark motion on frame
//            Scalar color = Scalar(255, 0,0 );
//            rectangle( frame, r,color);

//            if (fnum - last_blob_fnum > fps/FLIPBOOK_FPS)
//            {
//				//Add motion blob to object stream
//				Mat blob = Mat(frame, r);
//				imwrite(BLOB_TMP_FILE,blob,compression_params);
//				prism::connect::processors::ObjectStream object_s(blob_id,ftime,r.x,r.y,r.width,r.height,frame.cols,frame.rows);
//				std::cerr<<"Posting object stream"<<std::endl;
//				prism::connect::api::Response result = client.PostImageFileObjectStream(*this_camera.get(),object_s.ToJson(),BLOB_TMP_FILE);
//				std::cerr<<" status"<<result.status_code<<" {"<<result.text<<"}"<<std::endl;
//				last_blob_fnum = fnum;
//            }

//        }
        
//        //Update blob id when motion ended
//        if(!motion && was_motion)
//        	blob_id++;

//        //Update flipbook
//        if (std::chrono::duration_cast<std::chrono::seconds>(ftime - flipbook_start_time).count() >=  FLIPBOOK_UPDATE_S)
//        {
//        	prism::connect::api::Response result;
//        	if(writer.get()) //if we were writing something
//        	{
//        		//finalize stream, upload data
//        		std::cout<<"Close file:"<<FLIPBOOK_TMP_FILE<<std::endl;
//        		writer->release();
//        		last_fnum =-1;
//        		//Post flipbook
//        		std::cerr<<"Posting flipbook file "<<FLIPBOOK_TMP_FILE<<std::endl;
//        		result = client.PostVideoFileFlipbook(*this_camera.get(),flipbook_start_time ,ftime,FLIPBOOK_SIZE.width,FLIPBOOK_SIZE.height,saved_frames,FLIPBOOK_TMP_FILE);
//        		std::cerr<<" status"<<result.status_code<<" {"<<result.text<<"}"<<std::endl;

//            	Event event;
//            	//Write event timestamp
//            	std::chrono::system_clock::time_point rounded_to_min = std::chrono::time_point_cast<std::chrono::minutes>(ftime);
//            	event.AddTimestamp(rounded_to_min);
//            	//Post Event
//            	std::cerr<<"Posting event"<<std::endl;
//            	result = client.PostTimeSeriesEvents(*this_camera.get(),ftime,event.ToJson());
//            	std::cerr<<" status"<<result.status_code<<" {"<<result.text<<"}"<<std::endl;
//            	last_event_update_time = ftime;
//        	}

//        	//Remove old file
//        	std::remove(FLIPBOOK_TMP_FILE);
//        	//Open video file
//        	std::cout<<"Open file:"<<FLIPBOOK_TMP_FILE<<std::endl;
//        	writer.reset(new VideoWriter(FLIPBOOK_TMP_FILE,VideoWriter::fourcc('H','2','6','4'),FLIPBOOK_FPS,FLIPBOOK_SIZE,1));
//        	saved_frames = 0;
//        	flipbook_start_time = ftime;
//        }

//        //Write frames with given FPS
//        if (fnum - last_fnum >= fps/FLIPBOOK_FPS)
//		{
//        	//Write frame
//        	Mat flip_frame;
//        	resize(frame, flip_frame, FLIPBOOK_SIZE, 0, 0, CV_INTER_CUBIC);
//        	writer->write(flip_frame);
//        	saved_frames++;
//        	last_fnum = fnum;
//		}

//        //update background every BACKGROUND_UPDATE_MIN minutes
//        if( std::chrono::duration_cast<std::chrono::minutes>(ftime-last_b_update_time).count() >= BACKGROUND_UPDATE_MIN)
//        {
//        	Mat background;
//        	pMOG2->getBackgroundImage(background);
//        	imwrite(BACKGROUND_TMP_FILE, background, compression_params); // save image
//            std::cerr<<"Posting background file "<<BACKGROUND_TMP_FILE<<std::endl;
//            prism::connect::api::Response result = client.PostImageFile(*this_camera.get(),"BACKGROUND",ftime,ftime,BACKGROUND_TMP_FILE);
//            std::cerr<<" status"<<result.status_code<<" {"<<result.text<<"}"<<std::endl;
//            last_b_update_time = ftime;
//        }
//        fnum++;
//        was_motion = motion;
//    }
    
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
