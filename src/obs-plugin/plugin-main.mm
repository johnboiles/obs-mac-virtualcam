#include <obs-module.h>
#include <obs.hpp>
#include <pthread.h>
#include <QMainWindow.h>
#include <QAction.h>
#include <obs-frontend-api.h>
#include <obs.h>
#include <CoreFoundation/CoreFoundation.h>
#include "MachServer.h"


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
    [sMachServer stop];
}

static void virtualcam_output_raw_video(void *data, struct video_data *frame)
{
    obs_video_info ovi;
    obs_get_video_info(&ovi);
    CGFloat width = ovi.output_width;
    CGFloat height = ovi.output_height;
    uint8_t *outData = frame->data[0];
    if (frame->linesize[0] != (ovi.output_width * 2)) {
        blog(LOG_ERROR, "VIRTUALCAM unexpected frame->linesize (expected:%d actual:%d)", (ovi.output_width * 2), frame->linesize[0]);
    }

    [sMachServer sendFrameWithSize:NSMakeSize(width, height) timestamp:frame->timestamp frameBytes:outData];
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
            action->setText(obs_module_text("Stop Virtual Camera"));
            obs_output_start(output);
        }
    };
    action->connect(action, &QAction::triggered, menu_cb);

    obs_register_output(&virtualcam_output_info);

    start();
    
    return true;
}

