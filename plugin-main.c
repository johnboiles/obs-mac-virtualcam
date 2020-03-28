#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("mac-virtualcam", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "macOS virtual webcam output";
}

static const char *virtualcam_output_get_name(void *type_data)
{
    (void)type_data;
    return obs_module_text("macOS Virtual Webcam");
}

static void *virtualcam_output_create(obs_data_t *settings, obs_output_t *output)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_create");
//    auto *decklinkOutput = new DeckLinkOutput(output, deviceEnum);
//
//    decklinkOutput->deviceHash = obs_data_get_string(settings, DEVICE_HASH);
//    decklinkOutput->modeID = obs_data_get_int(settings, MODE_ID);
//    decklinkOutput->keyerMode = (int)obs_data_get_int(settings, KEYER);
//
//    return decklinkOutput;
}

static void virtualcam_output_destroy(void *data)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_destroy");
//
//    auto *decklink = (DeckLinkOutput *)data;
//    delete decklink;
}

static bool virtualcam_output_start(void *data)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_start");
//
//    auto *decklink = (DeckLinkOutput *)data;
//    struct obs_audio_info aoi;
//
//    if (!obs_get_audio_info(&aoi)) {
//        blog(LOG_WARNING, "No active audio");
//        return false;
//    }
//
//    if (!decklink->deviceHash || !*decklink->deviceHash)
//        return false;
//
//    decklink->audio_samplerate = aoi.samples_per_sec;
//    decklink->audio_planes = 2;
//    decklink->audio_size =
//        get_audio_size(AUDIO_FORMAT_16BIT, aoi.speakers, 1);
//
//    decklink->start_timestamp = 0;
//
//    ComPtr<DeckLinkDevice> device;
//
//    device.Set(deviceEnum->FindByHash(decklink->deviceHash));
//
//    DeckLinkDeviceMode *mode = device->FindOutputMode(decklink->modeID);
//
//    decklink->SetSize(mode->GetWidth(), mode->GetHeight());
//
//    struct video_scale_info to = {};
//
//    if (decklink->keyerMode != 0) {
//        to.format = VIDEO_FORMAT_BGRA;
//    } else {
//        to.format = VIDEO_FORMAT_UYVY;
//    }
//    to.width = mode->GetWidth();
//    to.height = mode->GetHeight();
//
//    obs_output_set_video_conversion(decklink->GetOutput(), &to);
//
//    device->SetKeyerMode(decklink->keyerMode);
//
//    if (!decklink->Activate(device, decklink->modeID))
//        return false;
//
//    struct audio_convert_info conversion = {};
//    conversion.format = AUDIO_FORMAT_16BIT;
//    conversion.speakers = SPEAKERS_STEREO;
//    conversion.samples_per_sec = 48000; // Only format the decklink supports
//
//    obs_output_set_audio_conversion(decklink->GetOutput(), &conversion);
//
//    if (!obs_output_begin_data_capture(decklink->GetOutput(), 0))
//        return false;

    return true;
}

static void virtualcam_output_stop(void *data, uint64_t ts)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_stop");

//    auto *decklink = (DeckLinkOutput *)data;
//
//    obs_output_end_data_capture(decklink->GetOutput());
//
//    ComPtr<DeckLinkDevice> device;
//
//    device.Set(deviceEnum->FindByHash(decklink->deviceHash));
//
//    decklink->Deactivate();
}

static void virtualcam_output_raw_video(void *data, struct video_data *frame)
{
    blog(LOG_DEBUG, "VIRTUALCAM output_raw_video");

//    auto *decklink = (DeckLinkOutput *)data;
//
//    if (!decklink->start_timestamp)
//        decklink->start_timestamp = frame->timestamp;
//
//    decklink->DisplayVideoFrame(frame);
}

struct obs_output_info virtualcam_output_info = {
    .id = "virtual_webcam_output",
    .flags = OBS_OUTPUT_VIDEO,
    .get_name = virtualcam_output_get_name,
    .create = virtualcam_output_create,
    .destroy = virtualcam_output_destroy,
    .start = virtualcam_output_start,
    .stop = virtualcam_output_stop,
    .raw_video = virtualcam_output_raw_video,
    //.raw_audio = virtualcam_output_raw_audio;
    // Optional
    //.get_properties = virtualcam_output_properties;
    //.update = virtualcam_output_update;
};

//extern struct obs_output_info virtualcam_output_info;

bool obs_module_load(void)
{
    blog(LOG_WARNING, "LOADING VIRTUALCAM");
    obs_register_output(&virtualcam_output_info);
	return true;
}
