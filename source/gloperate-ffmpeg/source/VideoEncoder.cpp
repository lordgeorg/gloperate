
#include <gloperate-ffmpeg/VideoEncoder.h>

#include <globjects/base/baselogging.h>

extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/common.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/samplefmt.h>

    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixdesc.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/intreadwrite.h>
}


#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif


static const int C_STREAM_DURATION          = 5.0;
static const int C_STREAM_FRAME_RATE        = 25;
static const int C_STREAM_NB_FRAMES         = ((int)(C_STREAM_DURATION * C_STREAM_FRAME_RATE));
static const AVPixelFormat C_STREAM_PIX_FMT = AV_PIX_FMT_YUV420P;


using namespace globjects;


namespace gloperate_ffmpeg
{


VideoEncoder::VideoEncoder()
: m_context(nullptr)
, m_videoStream(nullptr)
, m_frame(nullptr)
, m_frameCounter(0)
{
    // Register codecs and formats
    avcodec_register_all();
    av_register_all();
}

VideoEncoder::~VideoEncoder()
{
}

void VideoEncoder::createVideoEncoder(const std::string & filename)
{
    // Choose video format from file name
    AVOutputFormat * format = av_guess_format(NULL, filename.c_str(), NULL);
    if (!format) {
        debug() << "Could not deduce output format from file extension: using MPEG.";
        format = av_guess_format("mpeg", NULL, NULL);
    }
    if (!format) {
        critical() << "Could not find suitable output format";
        return;
    }

    // Create context
    m_context = avformat_alloc_context();
    if (!m_context) {
        critical() << "Could not create video context.";
        return;
    }
    m_context->oformat = format;
//  m_context->max_b_frames = 1; // ?
//  snprintf(m_context->filename, sizeof(m_context->filename), "%s", filename.c_str());

    // Create video stream
    m_videoStream = avformat_new_stream(m_context, 0);
    if (!m_videoStream) {
        critical() << "Could not alloc stream";
        return;
    }

    // Set video stream type and options
    m_videoStream->codec->codec_type    = AVMEDIA_TYPE_VIDEO;
    m_videoStream->codec->codec_id      = format->video_codec;
    m_videoStream->codec->bit_rate      = 400000;
    m_videoStream->codec->width         = 352;
    m_videoStream->codec->height        = 288;
    m_videoStream->codec->time_base.num = 1;
    m_videoStream->codec->time_base.den = C_STREAM_FRAME_RATE;
    m_videoStream->codec->gop_size      = 12;
    m_videoStream->codec->pix_fmt       = C_STREAM_PIX_FMT;

    // Some formats want stream headers to be separate
    if (m_context->oformat->flags & AVFMT_GLOBALHEADER) {
        m_videoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    // [DEBUG] Output video stream info
    av_dump_format(m_context, 0, filename.c_str(), 1);

    // Find video encoder
    AVCodec * codec = avcodec_find_encoder(m_videoStream->codec->codec_id);
    if (!codec) {
        critical() << "Codec not found";
        return;
    }

    // Open codec
    if (avcodec_open2(m_videoStream->codec, codec, nullptr) < 0) {
        critical() << "Could not open codec";
        return;
    }

    // Allocate picture for encoding
    m_frame = av_frame_alloc();
    if (!m_frame) {
        critical() << "Could not allocate frame";
        return;
    }

    // Allocate frame buffer
    int size = avpicture_get_size(m_videoStream->codec->pix_fmt, m_videoStream->codec->width, m_videoStream->codec->height);
    uint8_t * picture_buf = (uint8_t *)av_malloc(size);
    if (!picture_buf) {
        critical() << "Could not allocate picture";
        av_free(m_frame);
        return;
    }

    // Assign frame buffer to picture
    avpicture_fill((AVPicture *)m_frame, picture_buf, m_videoStream->codec->pix_fmt, m_videoStream->codec->width, m_videoStream->codec->height);

    // Output to file
    /*
    if (!(format->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_context->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
            critical() << "Could not open  " << filename;
            return;
        }
    }
    */

    // Output to stream
    if (avio_open_dyn_buf(&m_context->pb)) {
        debug() << "Could not create output stream";
    }

    // Write video header
    avformat_write_header(m_context, NULL);
}

void VideoEncoder::putFrame(char * data, int width, int height)
{
    // Put input image into picture structure
    AVPicture inputPicture;
    avpicture_fill(&inputPicture, (uint8_t*)data, AV_PIX_FMT_RGB24, width, height);

    // Convert input image to output frame
    SwsContext * converter = sws_getContext(width,                     height,                     AV_PIX_FMT_RGB24,
                                            m_videoStream->codec->width, m_videoStream->codec->height, AV_PIX_FMT_YUV420P,
                                            SWS_BICUBIC, NULL, NULL, NULL);

    sws_scale(converter, inputPicture.data, inputPicture.linesize, 0, height, m_frame->data, m_frame->linesize);
    sws_freeContext(converter);

    // Set frame info
    m_frame->width  = m_videoStream->codec->width;
    m_frame->height = m_videoStream->codec->height;
    m_frame->format = m_videoStream->codec->pix_fmt;

    // Set timestamp
    m_frame->pts = m_frameCounter++;

    // Create output packet
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    // Encode video frame
    int res = 0;
    if (m_context->oformat->flags & AVFMT_RAWPICTURE) {
        // Raw image format
        packet.flags |= AV_PKT_FLAG_KEY;
        packet.stream_index = m_videoStream->index;
        packet.data = (uint8_t *)m_frame;
        packet.size = sizeof(AVPicture);

        // Write frame
        res = av_write_frame(m_context, &packet);
    } else {
        // Encode image frame
        int got_output;
        avcodec_encode_video2(m_videoStream->codec, &packet, m_frame, &got_output);
        if (got_output) {
            // Rescale time stamps
            if (packet.pts != AV_NOPTS_VALUE) {
                packet.pts = av_rescale_q(packet.pts, m_videoStream->codec->time_base, m_videoStream->time_base);
            }
            if (packet.dts != AV_NOPTS_VALUE) {
                packet.dts = av_rescale_q(packet.dts, m_videoStream->codec->time_base, m_videoStream->time_base);
            }
            packet.stream_index = m_videoStream->index;

            // Write frame
            res = av_write_frame(m_context, &packet);
        }
    }

    // Check for errors
    if (res != 0) {
        critical() << "Error while writing video frame";
        return;
    }

    // Give back buffer
    /*
    auto freeBuffer = [] (char * data, void * hint)
    {
        // Log::log(Log::Message, "Free video frame buffer.");
        // QByteArray * byteArray = reinterpret_cast<QByteArray *>(hint);
        // delete byteArray;
    };

    v8::Local<v8::Object> bufferObj = node::Buffer::New(isolate, (char*)packet.data, packet.size, freeBuffer, &packet);
    args.GetReturnValue().Set(scope.Escape(bufferObj));
    */

    // Destroy packet
    av_free_packet(&packet);
}

void VideoEncoder::closeVideoEncoder()
{
    // Write end of video file
    av_write_trailer(m_context);

    // Close video codec
    if (m_videoStream) {
        avcodec_close(m_videoStream->codec);
    }

    // Release frame
    if (m_frame) {
        av_free(m_frame->data[0]);
        av_free(m_frame);
    }

    // Release video streams
    for (unsigned int i = 0; i < m_context->nb_streams; i++) {
        av_freep(&m_context->streams[i]->codec);
        av_freep(&m_context->streams[i]);
    }

    // Close output file
    if (!(m_context->oformat->flags & AVFMT_NOFILE)) {
        avio_close(m_context->pb);
    }

    // Release context
    av_free(m_context);
}


} // namespace gloperate_ffmpeg
