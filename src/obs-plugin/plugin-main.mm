#include <obs-module.h>
#include <obs.hpp>
#include <pthread.h>
#include <QMainWindow.h>
#include <QAction.h>
#include <obs-frontend-api.h>
#include <obs.h>
#include <CoreFoundation/CoreFoundation.h>
#include "MachServer.h"
#include "Defines.generated.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("mac-virtualcam", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "macOS virtual webcam output";
}

obs_output_t *output;
obs_video_info videoInfo;
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
    blog(LOG_DEBUG, "output_create");
    sMachServer = [[MachServer alloc] init];
}

static void virtualcam_output_destroy(void *data)
{
    blog(LOG_DEBUG, "output_destroy");
    sMachServer = nil;
}

static bool virtualcam_output_start(void *data)
{
    blog(LOG_DEBUG, "output_start");
    
    [sMachServer run];

    obs_get_video_info(&videoInfo);
    
    struct video_scale_info conversion = {};
    conversion.format = VIDEO_FORMAT_UYVY;
    conversion.width = videoInfo.output_width;
    conversion.height = videoInfo.output_height;
    obs_output_set_video_conversion(output, &conversion);
    if (!obs_output_begin_data_capture(output, 0)) {
        return false;
    }

    return true;
}

static void virtualcam_output_stop(void *data, uint64_t ts)
{
    blog(LOG_DEBUG, "output_stop");
    obs_output_end_data_capture(output);
    [sMachServer stop];
}

static void virtualcam_output_raw_video(void *data, struct video_data *frame)
{
    uint8_t *outData = frame->data[0];
    if (frame->linesize[0] != (videoInfo.output_width * 2)) {
        blog(LOG_ERROR, "unexpected frame->linesize (expected:%d actual:%d)", (videoInfo.output_width * 2), frame->linesize[0]);
    }

    CGFloat width = videoInfo.output_width;
    CGFloat height = videoInfo.output_height;

    [sMachServer sendFrameWithSize:NSMakeSize(width, height) timestamp:frame->timestamp fpsNumerator:videoInfo.fps_num fpsDenominator:videoInfo.fps_den frameBytes:outData];
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

bool obs_module_load(void)
{
    blog(LOG_DEBUG, "obs_module_load version=%s", PLUGIN_VERSION);

    QMainWindow* main_window = (QMainWindow*)obs_frontend_get_main_window();
    action = (QAction*)obs_frontend_add_tools_menu_qaction(obs_module_text("Start Virtual Camera"));
    auto menu_cb = []
    {
        if (obs_output_active(output)) {
            action->setText(obs_module_text("Start Virtual Camera"));
            obs_output_stop(output);
            obs_output_release(output);
            output = NULL;
        } else {
            action->setText(obs_module_text("Stop Virtual Camera"));
            OBSData settings;
            output = obs_output_create("virtualcam_output", "virtualcam_output", settings, NULL);
            obs_output_start(output);
        }
    };
    action->connect(action, &QAction::triggered, menu_cb);

    obs_register_output(&virtualcam_output_info);
    return true;
}

