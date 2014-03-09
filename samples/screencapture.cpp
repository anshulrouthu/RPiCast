// tutorial01.c
// Code based on a tutorial by Martin Bohme
// (boehme@inb.uni-luebeckREMOVETHIS.de)
// and another tutorial by Stephen Dranger (dranger at gmail dot com)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1

// A small sample program that shows how to use
// libavformat and libavcodec to read video from a file.
//
// Use
//
// gcc -o tutorial01 tutorial01.c -lavformat -lavcodec -lswscale -lz
//
// to build (assuming libavformat and libavcodec and libswscale
// are correctly installed your system).
//

extern "C"
{
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#include <assert.h>
#include <stdio.h>

static void video_encode_example(AVFrame* frame, FILE* fp);

FILE *pFile;

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    printf("SaveFrame\n");
    char szFilename[32];
    int y;

    // Write header
    //if(iFrame == 1)
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);

}

int main(int argc, char *argv[])
{

    AVFormatContext *pFormatCtx = NULL;
    int i, videoStream;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame;
    AVFrame *pFrameCropped;
    AVFrame *pFrameRGB;
    struct SwsContext * pSwsCtx;
    AVPacket packet;
    int frameFinished;
    int numBytes;
    int numBytesCroped;
    uint8_t *buffer;

    AVDictionary * p_options = NULL;
    AVInputFormat * p_in_fmt = NULL;

    pFile = fopen("screencap.out", "wb");
    if (pFile == NULL)
        return 0;

    // Register all formats and codecs
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();

    av_dict_set(&p_options, "framerate", "60", 0);
    av_dict_set(&p_options, "video_size", "1920x1080", 0);
    av_dict_set(&p_options, "qscale", "1", 0);
    p_in_fmt = av_find_input_format("x11grab");

    // Open video file
    if (avformat_open_input(&pFormatCtx, ":0.0", p_in_fmt, &p_options) != 0)
    {
        printf("cannot open input file!\n");
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        return -1; // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);

    // Find the first video stream
    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    if (videoStream == -1)
        return -1; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, 0) < 0)
        return -1; // Could not open codec

    // Allocate video frame
    pFrame = avcodec_alloc_frame();

    int crop_x = 0, crop_y = 0, crop_h = 1080, crop_w = 1920;
    pFrameCropped = avcodec_alloc_frame();

    if (pFrameCropped == NULL)
        return -1;

    // Allocate an AVFrame structure
    pFrameRGB = avcodec_alloc_frame();
    if (pFrameRGB == NULL)
        return -1;

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, crop_w, crop_h);
    buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *) pFrameRGB, buffer, AV_PIX_FMT_YUV420P, crop_w, crop_h);

    pSwsCtx = sws_getContext(crop_w, crop_h, pCodecCtx->pix_fmt, crop_w, crop_h, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR,
        NULL, NULL, NULL);

    if (pSwsCtx == NULL)
    {
        fprintf(stderr, "Cannot initialize the sws context\n");
        return -1;
    }

    // Read frames and save first five frames to disk
    i = 0;
    FILE* fp = fopen("encodec.mpg", "wb");
    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream)
        { // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // Did we get a video frame?
            if (frameFinished)
            {
                sws_scale(pSwsCtx, (const uint8_t * const *) pFrame->data, pFrame->linesize, 0, crop_h, pFrameRGB->data,
                    pFrameRGB->linesize);
                int y, x;
                /* Y */
                for (y = 0; y < crop_h; y++)
                {
                    for (x = 0; x < crop_w; x++)
                    {
                        //fwrite(pFrameRGB->data[0] + y * pFrameRGB->linesize[0] + x, sizeof(uint8_t), 1, fp);
                    }
                }
                /* Cb and Cr */
                for (y = 0; y < crop_h / 2; y++)
                {
                    for (x = 0; x < crop_w / 2; x++)
                    {
                        //fwrite(pFrameRGB->data[1] + y * pFrameRGB->linesize[1] + x, sizeof(uint8_t), 1, fp);
                        //fwrite(pFrameRGB->data[2] + y * pFrameRGB->linesize[2] + x, sizeof(uint8_t), 1, fp);
                    }
                }

                video_encode_example(pFrameRGB, fp);

                // Save the frame to disk
                if (++i >= 100)
                    break;
            }
        }

        av_free_packet(&packet);
    }

    fclose(fp);
    printf("Frames read %d\n", i);
    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    // Close file
    fclose(pFile);
    return 0;
}

static void video_encode_example(AVFrame* in_frame, FILE* fp)

{
    AVCodec *codec;
    AVCodecContext *c = NULL;
    int i, ret, got_output;
    FILE *f;
    AVFrame * frame = in_frame;
    AVPacket pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
    if (!codec)
    {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 1920;
    c->height = 1080;
    /* frames per second */
    c->time_base = (AVRational ) { 1, 60 };
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    /* open it */
    if (avcodec_open2(c, codec, 0) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fp;
    if (!f)
    {
        fprintf(stderr, "Could not open\n");
        exit(1);
    }

    {
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;

        fflush(stdout);

        frame->pts = i;

        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
        if (ret < 0)
        {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output)
        {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* get the delayed frames */
    for (got_output = 1; got_output; i++)
    {
        fflush(stdout);

        ret = avcodec_encode_video2(c, &pkt, 0, &got_output);
        if (ret < 0)
        {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output)
        {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* add sequence end code to have a real mpeg file */
    fwrite(endcode, 1, sizeof(endcode), f);

    avcodec_close(c);
    av_free(c);

    printf("\n");

}
