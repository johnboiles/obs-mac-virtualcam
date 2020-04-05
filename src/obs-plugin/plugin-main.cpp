#include <obs-module.h>
#include <obs.hpp>
#include <pthread.h>
#include <QMainWindow.h>
#include <QAction.h>
#include <obs-frontend-api.h>
#include "CMIO_DPA_Sample_Server_VCamDevice.h"
#include "CMIO_DPA_Sample_Server_VCamInputStream.h"
#include "CAHostTimeBase.h"
#include <iostream>
#include <obs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <QMessageBox>
//#include <video-io.h>

using namespace std;

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("mac-virtualcam", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "macOS virtual webcam output";
}

obs_output_t *output;
// Tools menu action for starting and stopping the virtual camera
QAction *action;

static const char *virtualcam_output_get_name(void *type_data)
{
    (void)type_data;
    return obs_module_text("macOS Virtual Webcam");
}

static void *virtualcam_output_create(obs_data_t *settings, obs_output_t *output)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_create");
}

static void virtualcam_output_destroy(void *data)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_destroy");
}

// TODO(johnboiles): Janky to extern this
extern void *virtualCamMain(void *ptr);


static string knownResoultions[] = {    //Resolutions which are specifies in the enum "FrameType" in CMIO_DPA_Sample_Shared.h
    "640x360",
    "720x480",
    "720x486",
    "720x576",
    "1280x720",
    "1920x1080"
};


static bool virtualcam_output_start(void *data)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_start");
    
    pthread_t thread1;
    pthread_create(&thread1, NULL, virtualCamMain, (void *)"Thread 1");

    obs_video_info ovi;
    obs_get_video_info(&ovi);
    
    // TODO(johnboiles): Right now we're hardcoded for 1280x720 but that should probably change // DONE--gxalpha
    struct video_scale_info conversion = {};
    conversion.format = VIDEO_FORMAT_UYVY;
    conversion.width = ovi.output_width;
    conversion.height = ovi.output_height;
    obs_output_set_video_conversion(output, &conversion);
    if (!obs_output_begin_data_capture(output, 0)) {
        return false;
    }

    return true;
}

static void virtualcam_output_stop(void *data, uint64_t ts)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_stop");
    obs_output_end_data_capture(output);
}

// TODO(johnboiles): Janky to extern this. Make classes and stuff.
extern CMIO::DPA::Sample::Server::VCamDevice *virtualCamDevice;

static void virtualcam_output_raw_video(void *data, struct video_data *frame)
{
    uint8_t *outData = frame->data[0];
    virtualCamDevice->mInputStream->FrameArrived(virtualCamDevice->mFrameSize, outData, frame->timestamp);
}

struct obs_output_info virtualcam_output_info = {
    .id = "virtualcam_output",
    .flags = OBS_OUTPUT_VIDEO,
    .get_name = virtualcam_output_get_name,
    .create = virtualcam_output_create,
    .destroy = virtualcam_output_destroy,
    .start = virtualcam_output_start,
    .stop = virtualcam_output_stop,
    .raw_video = virtualcam_output_raw_video,
};

void start()
{
    OBSData settings;
    
    output = obs_output_create("virtualcam_output", "virtualcam_output", settings, NULL);
    obs_data_release(settings);
}

bool isSupportedResolution();
bool obs_module_load(void)
{
    blog(LOG_DEBUG, "VIRTUALCAM obs_module_load");

    QMainWindow* main_window = (QMainWindow*)obs_frontend_get_main_window();
    action = (QAction*)obs_frontend_add_tools_menu_qaction(obs_module_text("Start Virtual Camera"));
    auto menu_cb = []
    {
        if (obs_output_active(output)) {
            action->setText(obs_module_text("Start Virtual Camera"));
            obs_output_stop(output);
        } else {
            if(isSupportedResolution()){
                action->setText(obs_module_text("Stop Virtual Camera"));
                obs_output_start(output);
            } else {
                
                stringstream msg;
                msg << "Resolution not supported. Please use one of the following:" << endl;
                for(string res : knownResoultions){
                    msg << res << endl;
                }
                QMessageBox msgBox;
                msgBox.setText(msg.str());
            }
            
        }
    };
    action->connect(action, &QAction::triggered, menu_cb);

    obs_register_output(&virtualcam_output_info);

    start();
    
    return true;
}

bool isSupportedResolution()
{
    
    //obs_get_video()->video_ouput_info.width; <--- Maybe someone gets this to work, I didn't xD (Would be using the settings api which would be more elegant)
    obs_video_info ovi;
    obs_get_video_info(&ovi);
    std::cout << "Base Resolution: " << ovi.base_width << "x" << ovi.base_height << endl;
    std::cout << "(Scaled) Output Resolution: " << ovi.output_width << "x" << ovi.output_height << endl;
    
    stringstream stream;
    stream << ovi.output_width << "x" << ovi.output_height;
    string res = stream.str();
    
    for(string orgRes : knownResoultions){
        if(strcmp(orgRes.c_str(), res.c_str())==0){
            return true;
        }
    }
    return false;
}
