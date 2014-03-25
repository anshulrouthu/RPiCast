/*
 * muxing.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: anshul
 */

#include "utils.h"
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
FILE *FileOut;

static int WritePacket(void *opaque, uint8_t *pBuffer, int pBufferSize)
{
    return fwrite(pBuffer, pBufferSize, 1, FileOut);
}

void run()
{
    printf("begin\n");
    int Width = 320, Height = 240;
    const char *FileName = "muxedstream.m2ts";
    FileOut = fopen(FileName, "w");
    int BUFFERSIZE = 32768;
    //int BUFFERSIZE= 512;
    AVIOContext *pAVIOContext = NULL;
    unsigned char *pBufferCallBack = (unsigned char*) av_malloc(BUFFERSIZE * sizeof(uint8_t));
    {
        pAVIOContext = avio_alloc_context(pBufferCallBack, BUFFERSIZE, 1, NULL, NULL, WritePacket, NULL);
        if (pAVIOContext == NULL)
            printf("Errore avio_alloc_context !!!\n");
        else
        {
            AVOutputFormat *pAVOutputFormat = av_guess_format("h264", FileName, NULL); // CODEC_ID_H264 -> mp4, mov; CODEC_ID_THEORA -> ogg; CODEC_ID_MPEG4 -> mpegts, avi; CODEC_ID_VP8 -> webm
            if (!pAVOutputFormat)
            {
                printf("Could not set output format, using MPEG.\n");
                pAVOutputFormat = av_guess_format("mp4", NULL, NULL);
            }
            if (!pAVOutputFormat)
                printf("av_guess_format Error!\n");
            else
            {
                AVFormatContext *pAVFormatContext;
                pAVFormatContext = avformat_alloc_context();
                if (!pAVFormatContext)
                    printf("avformat_alloc_context Error!\n");
                else
                {
                    pAVFormatContext->pb = pAVIOContext;
                    pAVFormatContext->oformat = pAVOutputFormat;
                    AVStream *pAVStream = avformat_new_stream(pAVFormatContext, NULL);
                    if (!pAVStream)
                        printf("av_new_stream Error!");
                    else
                    {
                        AVCodecContext *pAVCodecContext = pAVStream->codec;
                        pAVCodecContext->codec_id = /*(CodecID)*/pAVOutputFormat->video_codec;
                        pAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
                        pAVCodecContext->bit_rate = 40000;
                        pAVCodecContext->width = Width;
                        pAVCodecContext->height = Height;
                        pAVCodecContext->time_base.den = 25;
                        pAVCodecContext->time_base.num = 1;
                        pAVCodecContext->gop_size = 10;
                        pAVCodecContext->pix_fmt = PIX_FMT_YUV420P;
                        if (pAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
                            pAVCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
                        if (pAVCodecContext->codec_id == CODEC_ID_H264)
                        {
                            printf("CODEC_ID_H264\n");
                            pAVCodecContext->bit_rate = 500 * 1000;
                            pAVCodecContext->bit_rate_tolerance = 0;
                            pAVCodecContext->rc_max_rate = 0;
                            pAVCodecContext->rc_buffer_size = 0;
                            pAVCodecContext->gop_size = 40;
                            pAVCodecContext->max_b_frames = 3;
                            pAVCodecContext->b_frame_strategy = 1;
                            pAVCodecContext->coder_type = 1;
                            pAVCodecContext->me_cmp = 1;
                            pAVCodecContext->me_range = 16;
                            pAVCodecContext->qmin = 10;
                            pAVCodecContext->qmax = 51;
                            pAVCodecContext->scenechange_threshold = 40;
                            pAVCodecContext->flags |= CODEC_FLAG_LOOP_FILTER;
                            pAVCodecContext->me_method = ME_HEX;
                            pAVCodecContext->me_subpel_quality = 5;
                            pAVCodecContext->i_quant_factor = 0.71;
                            pAVCodecContext->qcompress = 0.6;
                            pAVCodecContext->max_qdiff = 4;
                            pAVFormatContext->oformat->flags |= AVFMT_TS_NONSTRICT;
                        }
                        else if (pAVCodecContext->codec_id == CODEC_ID_THEORA)
                        {
                            printf("CODEC_ID_THEORA\n");
                            //pAVCodecContext->extradata= av_malloc((void*)FF_INPUT_BUFFER_PADDING_SIZE);
                        }
                        av_dict_set(&pAVFormatContext->metadata, "title", "Titolo", 0);
                        av_dump_format(pAVFormatContext, 0, NULL, 1);
                        if (avformat_write_header(pAVFormatContext, NULL) != 0)
                            printf("av_write_header Error!\n");
                        AVCodec *pCodec = avcodec_find_encoder(pAVCodecContext->codec_id);
                        if (!pCodec)
                            printf("avcodec_find_encoder Error!");
                        else
                        {
                            if (avcodec_open2(pAVCodecContext, pCodec, NULL) < 0)
                                printf("avcodec_open Error!\n");
                            else
                            {
                                int BYTEPIC = Width * Height * 3;
                                uint8_t *pOutBuffer = (uint8_t*) malloc(BYTEPIC);
                                {
                                    int Frames = 0;
                                    while (Frames < 250)
                                    {
                                        Frames++;
                                        AVFrame *pAVFrame = avcodec_alloc_frame();
                                        uint8_t *pBuffer = (uint8_t*) malloc(
                                            avpicture_get_size(PIX_FMT_YUV420P, Width, Height));
                                        {
                                            avpicture_fill((AVPicture*) pAVFrame, pBuffer, PIX_FMT_YUV420P, Width,
                                                Height);
                                            pAVFrame->pts = Frames;
                                            int OutSize = avcodec_encode_video(pAVCodecContext, pOutBuffer, BYTEPIC,
                                                pAVFrame);
                                            if (OutSize > 0)
                                            {
                                                AVPacket Packet;
                                                av_init_packet(&Packet);
                                                //Packet.pts= Frames;
                                                //if (pAVCodecContext->coded_frame->pts!= unsigned(AV_NOPTS_VALUE)) {
                                                Packet.pts = av_rescale_q(pAVCodecContext->coded_frame->pts,
                                                    pAVCodecContext->time_base, pAVStream->time_base);
                                                //}
                                                if (pAVCodecContext->coded_frame->key_frame)
                                                    Packet.flags |= AV_PKT_FLAG_KEY;
                                                Packet.stream_index = pAVStream->index;
                                                Packet.data = pOutBuffer;
                                                Packet.size = OutSize;
                                                if (av_interleaved_write_frame(pAVFormatContext, &Packet) != 0)
                                                    printf("av_interleaved_write_frame Error!\n");
                                            }
                                        }
                                        {
                                            av_free(pAVFrame);
                                            free(pBuffer);
                                        }
                                        usleep(1000 * 1000 / 25);
                                    }
                                }
                                {
                                    free(pOutBuffer);
                                }
                                avcodec_close(pAVCodecContext);
                            }
                        }
                        if (av_write_trailer(pAVFormatContext) != 0)
                            printf("av_write_trailer Error!\n");
                        av_free(pAVStream);
                    }
                    av_free(pAVFormatContext);
                }
                //av_free(pAVOutputFormat);
            }
            av_free(pAVIOContext);
        }
    }
    {
        free(pBufferCallBack);
    }
    fclose(FileOut);
    printf("end\n");
}

int main(void)
{
    printf("Hello World!\n");
    av_register_all();
    run();
    return 0;
}

