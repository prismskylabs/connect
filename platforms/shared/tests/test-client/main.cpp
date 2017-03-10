/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <boost/filesystem.hpp>
#include "client.h"
#include "curl/curl.h"
#include "easylogging++.h"
#include "rapidjson/document.h"
#include "public-util.h"

_INITIALIZE_EASYLOGGINGPP

using namespace cv;

namespace prc = prism::connect;

typedef boost::chrono::system_clock::time_point time_point;

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

prc::unique_ptr<VideoWriter>::t writer;

Rect resizeRect(Rect r,float scale)
{
    return Rect(r.x*scale,r.y*scale,r.width*scale,r.height*scale);
}

struct CurlGlobal
{
    CurlGlobal()
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~CurlGlobal()
    {
        curl_global_cleanup();
    }
};

class CustomLogTarget : public easyloggingpp::ILogTarget
{
public:
    CustomLogTarget()
        : count_(0)
    {
    }

    void log(const std::string& msg) {
        ++count_;
        std::cout << "log(" << count_ << "): " << msg << std::endl;
    }

private:
    int count_;
};

class ScopedLogTargetGuard
{
public:
    ScopedLogTargetGuard(easyloggingpp::ILogTarget* target)
        : target_(target)
    {
        easyloggingpp::Loggers::addLogTarget(target);
    }

    ~ScopedLogTargetGuard()
    {
        easyloggingpp::Loggers::removeLogTarget(target_.get());
    }

private:
    prc::unique_ptr<easyloggingpp::ILogTarget>::t target_;
};

bool findInstrumentByName(prc::Client& client, int accountId,
                          const std::string& cameraName, prc::Instrument& instrument)
{
    prc::Instruments instruments;
    prc::Status status = client.queryInstrumentsList(accountId, instruments);

    if (status.isSuccess()  &&  !instruments.empty())
        for (size_t i = 0; i < instruments.size(); ++i)
            if (instruments[i].name == cameraName)
            {
                instrument = instruments[i];
                return true;
            }

    return false;
}

void initLogger()
{
    namespace el = easyloggingpp;

    el::Configurations conf;

    conf.set(el::Level::All, el::ConfigurationType::Format, "%datetime | %level | %log");
    conf.set(el::Level::All, el::ConfigurationType::ToFile, "false");
    conf.set(el::Level::All, el::ConfigurationType::Enabled, "true");

    el::Loggers::reconfigureAllLoggers(conf);
}

void testUploadCount(prc::Client& client, prc::id_t accountId, prc::id_t instrumentId)
{
    prc::Counts counts;
    time_point ftime = boost::chrono::system_clock::now() - boost::chrono::hours(1);

    for (int i = 0; i < 3; ++i)
    {
        ftime += boost::chrono::minutes(1) + boost::chrono::seconds(17);
        counts.push_back(prc::Count(prc::toTimestamp(ftime), i * 10 + 3));
    }

    prc::Status status = client.uploadCount(accountId, instrumentId, counts, false);

    for (int i = 0; i < 3; ++i)
        counts[i].value += i + 1;

    status = client.uploadCount(accountId, instrumentId, counts, true);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage:\n\tyt_camera <camera-name> <input-file>\n" << std::endl;
        return -1;
    }

    // uncomment to test custom log target
//    ScopedLogTargetGuard logTarget(new CustomLogTarget());

    initLogger();

    std::string cameraName(argv[1]);
    std::string inputFile(argv[2]);

    LOG(INFO) << "App name: " << argv[0];
    LOG(INFO) << "Camera name: " << cameraName;
    LOG(INFO) << "Input file: " << inputFile;

    const char* envBuf = std::getenv("API_ROOT");

    if (!envBuf)
    {
        LOG(ERROR) << "API_ROOT environment variable is undefined";
        return -1;
    }

    std::string apiRoot(envBuf);

    envBuf = std::getenv("API_TOKEN");

    if (!envBuf)
    {
        LOG(ERROR) << "API_TOKEN environment variable is undefined";
        return -1;
    }

    std::string token(envBuf);

    prc::SdkVersion ver = prc::getSdkVersion();

    LOG(INFO) << "SDK version: " << ver.toString();

    CurlGlobal cg;

    prc::Client client(apiRoot, token);
    client.setLogFlags(prc::Client::LOG_INPUT | prc::Client::LOG_INPUT_JSON);
    prc::Status status = client.init();

    LOG(INFO) << "client.init(): " << status;

    client.setConnectionTimeoutMs(5000);
    client.setLowSpeed(5);

    prc::Accounts accounts;
    status = client.queryAccountsList(accounts);

    if (status.isError()  ||  accounts.empty())
    {
        LOG(ERROR) << "No accounts associated with given token, exiting";
        return -1;
    }

    prc::id_t accountId = accounts[0].id;

    LOG(INFO) << "Account ID: " << accountId;

    prc::Instrument instrument;

    if (!findInstrumentByName(client, accountId, cameraName, instrument))
    {
        prc::Instrument newInstrument;
        newInstrument.name = cameraName;
        newInstrument.type = "camera";
        status = client.registerInstrument(accountId, newInstrument);

        if (status.isError())
        {
            LOG(ERROR) << "Failed to register camera with name: " << cameraName;
            return -1;
        }

        if (!findInstrumentByName(client, accountId, cameraName, instrument))
        {
            LOG(ERROR) << "Failed to find just registered instrument: " << cameraName;
            return -1;
        }
    }

    prc::id_t instrumentId = instrument.id;

    LOG(INFO) << "Instrument ID: " << instrumentId;

    testUploadCount(client, accountId, instrumentId);

    // Open video stream
    VideoCapture cap(inputFile.c_str());

    if(!cap.isOpened()) // check if we succeeded
        return -1;

    int fps = cap.get(CV_CAP_PROP_FPS);
    
    LOG(INFO) << "Processing...";

    // create Background Subtractor objects
    
    Mat fgMaskMOG2; // fg mask fg mask generated by MOG2 method
    Ptr<BackgroundSubtractor> pMOG2; // MOG2 Background subtractor
    pMOG2 = createBackgroundSubtractorMOG2(); // MOG2 approach
    

    int fnum = 0;
    time_point ftime;
    int last_fnum = -10000;
    int saved_frames = 0;
    time_point flipbook_start_time;
    bool motion = false;
    bool was_motion = false;
    int blob_id = 0;
    int last_blob_fnum = -10000;

    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95);

    //Set current timepoint to 1h ago since we processing faster than real time
    //and will be posting to future
    ftime = boost::chrono::system_clock::now() - boost::chrono::hours(1);
    int prevMinute = -1;

    // use this to enable/disable uploading particular types of data
    // useful for testing/debugging

    bool enableObjectStream = true;
    bool enableEvents = true;
    bool enableBackground = true;
    bool enableFlipbook = true;

    client.setLogFlags(prc::Client::LOG_INPUT | prc::Client::LOG_INPUT_JSON);

    for(;;)
    {
        Mat frame, gray_frame;

        cap >> frame; // get a new frame from camera
        //get frame timestamp
        ftime += boost::chrono::milliseconds(1000 / fps);

        if (frame.empty())
            break;

        cvtColor(frame, gray_frame, COLOR_BGR2GRAY); //switch to grayscale
        
        float scale =(float) resize_to_height/gray_frame.rows;
        int resized_width = gray_frame.cols*scale;
        resize(gray_frame, gray_frame, Size(resized_width,resize_to_height), 0, 0, CV_INTER_CUBIC); // resize
        
        GaussianBlur(gray_frame, gray_frame, Size(KERNEL_SIZE,KERNEL_SIZE), SIGMA, SIGMA);
        pMOG2->apply(gray_frame, fgMaskMOG2);
        
        Mat dilated;
        Mat element = getStructuringElement( MORPH_ELLIPSE,
                                            Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                            Point( dilation_size, dilation_size ) );
        dilate(fgMaskMOG2,dilated,element); // dilate the thresholded image to fill in holes, then find contours on thresholded image
        
        std::vector<std::vector<Point> > contours;
        std::vector<Vec4i> hierarchy;
        findContours( dilated, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0) ); //find contours
        
        motion = false;
        for( int i = 0; i< contours.size(); i++ ) // loop over the contours
        {
            double area = contourArea(contours[i]);
            if(area < MIN_AREA)
                continue;
            
            motion = true;
            CvRect r = boundingRect(contours[i]);
            r = resizeRect(r,1.0/scale);

            //Mark motion on frame
            Scalar color = Scalar(255, 0,0 );
            rectangle( frame, r,color);

            if (fnum - last_blob_fnum > fps/FLIPBOOK_FPS  &&  enableObjectStream)
            {
                //Add motion blob to object stream
                prc::ObjectStream os;
                os.objectId = blob_id;
                os.streamType = "foreground";
                os.collected = prc::toTimestamp(ftime);
                os.locationX = r.x;
                os.locationY = r.y;
                os.width = r.width;
                os.height = r.height;
                os.origImageWidth = frame.cols;
                os.origImageHeight = frame.rows;

                LOG(DEBUG) << "Posting object stream";

                Mat blob = Mat(frame, r);

#if BLOB_FROM_FILE
                imwrite(BLOB_TMP_FILE, blob, compression_params);
                status = client.uploadObjectStream(accountId, instrumentId,
                                                   os, prc::Payload(BLOB_TMP_FILE));
#else
                std::vector<uchar> buf;
                bool rv = imencode(".jpg", blob, buf, compression_params);

                LOG(DEBUG) << "Encoded blob to memory " << rv << ", uploading, blob size, bytes "
                           << buf.size();

                status = client.uploadObjectStream(accountId, instrumentId,
                                                   os, prc::Payload(buf.data(), buf.size(), "image/jpeg"));
#endif

                last_blob_fnum = fnum;
            }

        }
        
        //Update blob id when motion ended
        if(!motion && was_motion)
            blob_id++;

        //Update flipbook
        int currentMinute = boost::chrono::duration_cast<boost::chrono::minutes>(ftime.time_since_epoch()).count();
        if (prevMinute < 0)
            prevMinute = currentMinute;

        bool needUpdateData = prevMinute < currentMinute;

        if (needUpdateData)
        {
            long long totalMs = boost::chrono::duration_cast<boost::chrono::milliseconds>(ftime.time_since_epoch()).count();
            long long currMinMs = totalMs % 60000LL;

            LOG(DEBUG) << "totalMs: " << totalMs;
            LOG(DEBUG) << "currMinMs: " << currMinMs;

            time_point currentMinuteStart = ftime - boost::chrono::milliseconds(currMinMs);
            time_point prevMinuteStart = currentMinuteStart - boost::chrono::minutes(1);
            time_point prevMinuteEnd = prevMinuteStart + boost::chrono::seconds(59);

            {
                time_t ttp = boost::chrono::system_clock::to_time_t(ftime);
                LOG(DEBUG) << "ftime: " << ctime(&ttp);
            }

            {
                time_t ttp = boost::chrono::system_clock::to_time_t(currentMinuteStart);
                LOG(DEBUG) << "currentMinuteStart: " << ctime(&ttp);
            }

            {
                time_t ttp = boost::chrono::system_clock::to_time_t(prevMinuteStart);
                LOG(DEBUG) << "prevMinuteStart: " << ctime(&ttp);
            }

            {
                time_t ttp = boost::chrono::system_clock::to_time_t(prevMinuteEnd);
                LOG(DEBUG) << "prevMinuteEnd: " << ctime(&ttp);
            }

            // Update Flipbook Data
            if (writer.get()  &&  enableFlipbook)
            {
                //finalize stream, upload data
                LOG(DEBUG) << "Close file: " << FLIPBOOK_TMP_FILE;\

                writer->release();
                last_fnum =-1;

                LOG(DEBUG) << "Posting flipbook file " << FLIPBOOK_TMP_FILE;

                prc::Flipbook fb;
                fb.startTimestamp = prc::toTimestamp(prevMinuteStart);
                fb.stopTimestamp = prc::toTimestamp(currentMinuteStart);
                fb.width = FLIPBOOK_SIZE.width;
                fb.height = FLIPBOOK_SIZE.height;
                fb.numberOfFrames = saved_frames;

                // it will log error code and message, if anything goes wrong
                status = client.uploadFlipbook(accountId, instrumentId, fb, prc::Payload(FLIPBOOK_TMP_FILE));
            }

            // update background
            if (enableBackground)
            {
                Mat background;
                pMOG2->getBackgroundImage(background);

#if BACKGROUND_FROM_FILE
                imwrite(BACKGROUND_TMP_FILE, background, compression_params); // save image

                LOG(DEBUG) << "Posting background file " << BACKGROUND_TMP_FILE;

                status = client.uploadBackground(accountId, instrumentId,
                                                 prc::toTimestamp(prevMinuteStart),
                                                 prc::Payload(BACKGROUND_TMP_FILE));
#else
                std::vector<uchar> buf;
                bool rv = imencode(".jpg", background, buf, compression_params);

                LOG(DEBUG) << "Encoded background to memory " << rv << ", uploading";

                status = client.uploadBackground(accountId, instrumentId,
                                                 prc::toTimestamp(prevMinuteStart),
                                                 prc::Payload(buf.data(), buf.size(), "image/jpeg"));
#endif
            }

            // Update event
            if (enableEvents)
            {
                prc::Event event;
                prc::Events events;
                event.timestamp = prc::toTimestamp(
                            boost::chrono::time_point_cast<boost::chrono::minutes>(ftime));
                events.push_back(event);

                LOG(DEBUG) << "Posting event";

                status = client.uploadEvent(accountId, instrumentId,
                                            prc::toTimestamp(prevMinuteEnd), events);
            }
        }

        if ((!writer.get() || needUpdateData)  &&  enableFlipbook)
        {
            boost::filesystem::remove(FLIPBOOK_TMP_FILE);

            LOG(DEBUG) << "Open file: " << FLIPBOOK_TMP_FILE;

            writer.reset(new VideoWriter(FLIPBOOK_TMP_FILE, VideoWriter::fourcc('H','2','6','4'),
                                         FLIPBOOK_FPS, FLIPBOOK_SIZE, 1));
            saved_frames = 0;
            flipbook_start_time = ftime;
        }

        //Write frames with given FPS
        if (writer.get()  &&  fnum - last_fnum >= fps/FLIPBOOK_FPS  &&  enableFlipbook)
        {
            //Write frame
            LOG(DEBUG) << "Write flipbook video frame";
            Mat flip_frame;
            resize(frame, flip_frame, FLIPBOOK_SIZE, 0, 0, CV_INTER_CUBIC);
            writer->write(flip_frame);
            saved_frames++;
            last_fnum = fnum;
        }

        prevMinute = currentMinute;

        fnum++;
        was_motion = motion;
    }
    
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
