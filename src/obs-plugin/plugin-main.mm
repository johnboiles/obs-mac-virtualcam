#include <obs-module.h>
#include <obs.hpp>
#include <pthread.h>
#include <QMainWindow.h>
#include <QAction.h>
#include <obs-frontend-api.h>
#include <obs.h>
#include <sstream>
#include <QMessageBox>
#include <QString>
#include <CoreFoundation/CoreFoundation.h>
#include "MachServer.h"

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
static MachServer *sMachServer;

static const char *virtualcam_output_get_name(void *type_data)
{
    (void)type_data;
    return obs_module_text("macOS Virtual Webcam");
}

static void *virtualcam_output_create(obs_data_t *settings, obs_output_t *output)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_create");
    sMachServer = [[MachServer alloc] init];
}

static void virtualcam_output_destroy(void *data)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_destroy");
}

static string knownResolutions[] = {    //Resolutions which are specifies in the enum "FrameType" in CMIO_DPA_Sample_Shared.h
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
    
    [sMachServer run];

    obs_video_info ovi;
    obs_get_video_info(&ovi);
    
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

static void virtualcam_output_raw_video(void *data, struct video_data *frame)
{
    uint8_t *outData = frame->data[0];
    // TODO: Send frame here
//    virtualCamDevice->mInputStream->FrameArrived(virtualCamDevice->mFrameSize, outData, frame->timestamp);
    [sMachServer sendFrame];
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
                msg << obs_module_text("Your output resolution not supported. Please use one of the following:") << endl;
                for(string res : knownResolutions){
                    msg << res << endl;
                }
                QString title = QString::fromStdString(obs_module_text("Unsupported resolution"));
                QString qstr = QString::fromStdString(msg.str());
                QMessageBox msgBox;
                msgBox.setText(title);
                msgBox.setInformativeText(qstr);
                msgBox.exec();
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
    
    obs_video_info ovi;
    obs_get_video_info(&ovi);
    
    stringstream stream;
    stream << ovi.output_width << "x" << ovi.output_height;
    string res = stream.str();
    
    for(string orgRes : knownResolutions){
        if(strcmp(orgRes.c_str(), res.c_str())==0){
            return true;
        }
    }
    return false;
}
