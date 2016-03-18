#include "player.h"

int read_thread_abord = 0;

int no_more_data_in_the_queue = 0; // if there are no more data in the queue, we think the audio is stopping

CPlayer *global_cplayer_ctx = NULL;
AVPacket flush_pkt;

static int g_ffmpeg_global_inited = false;

void packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
}

static void packet_queue_flush(PacketQueue *q) {
    AVPacketList *pkt, *pkt1;
    SDL_LockMutex(q->mutex);
    for (pkt = q->first_pkt; pkt != NULL; pkt=pkt1) {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_free(pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
    SDL_UnlockMutex(q->mutex);
}

static void packet_queue_destory(PacketQueue *q) {
    packet_queue_flush(q);
    if (q->mutex)
        SDL_DestroyMutex(q->mutex);
    if (q->cond)
        SDL_DestroyCond(q->cond);
}

void global_init() {
    if (g_ffmpeg_global_inited)
        return;
    // Register all formats and codecs
    av_register_all();

    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    av_init_packet(&flush_pkt);
    flush_pkt.data = (unsigned char *)"FLUSH";

    g_ffmpeg_global_inited = true;
}

static int audio_resampling(AVCodecContext *audio_decode_ctx,
                            AVFrame *audio_decode_frame,
                            enum AVSampleFormat out_sample_fmt,
                            int out_channels,
                            int out_sample_rate,
                            uint8_t *out_buf)
{
    SwrContext *swr_ctx = NULL;
    int ret = 0;
    int64_t in_channel_layout = audio_decode_ctx->channel_layout;
    int64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int out_nb_channels = 0;
    int out_linesize = 0;
    int in_nb_samples = 0;
    int out_nb_samples = 0;
    int max_out_nb_samples = 0;
    uint8_t **resampled_data = NULL;
    int resampled_data_size = 0;

    swr_ctx = swr_alloc();
    if (!swr_ctx) {
        printf("swr_alloc error\n");
        return -1;
    }

    in_channel_layout = (audio_decode_ctx->channels ==
                     av_get_channel_layout_nb_channels(audio_decode_ctx->channel_layout)) ?
                     audio_decode_ctx->channel_layout :
                     av_get_default_channel_layout(audio_decode_ctx->channels);
    if (in_channel_layout <=0) {
        printf("in_channel_layout error\n");
        return -1;
    }

    if (out_channels == 1) {
        out_channel_layout = AV_CH_LAYOUT_MONO;
    } else if (out_channels == 2) {
        out_channel_layout = AV_CH_LAYOUT_STEREO;
    } else {
        out_channel_layout = AV_CH_LAYOUT_SURROUND;
    }

    in_nb_samples = audio_decode_frame->nb_samples;
    if (in_nb_samples <=0) {
        printf("in_nb_samples error\n");
        return -1;
    }

    av_opt_set_int(swr_ctx, "in_channel_layout", in_channel_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", audio_decode_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_decode_ctx->sample_fmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout", out_channel_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", out_sample_fmt, 0);

    if ((ret = swr_init(swr_ctx)) < 0) {
        printf("Failed to initialize the resampling context\n");
        return -1;
    }

    max_out_nb_samples = out_nb_samples = av_rescale_rnd(in_nb_samples,
                                                         out_sample_rate,
                                                         audio_decode_ctx->sample_rate,
                                                         AV_ROUND_UP);

    if (max_out_nb_samples <= 0) {
        printf("av_rescale_rnd error\n");
        return -1;
    }

    out_nb_channels = av_get_channel_layout_nb_channels(out_channel_layout);

    ret = av_samples_alloc_array_and_samples(&resampled_data, &out_linesize, out_nb_channels, out_nb_samples, out_sample_fmt, 0);
    if (ret < 0) {
        printf("av_samples_alloc_array_and_samples error\n");
        return -1;
    }

    out_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, audio_decode_ctx->sample_rate) + in_nb_samples,
                                    out_sample_rate, audio_decode_ctx->sample_rate, AV_ROUND_UP);
    if (out_nb_samples <= 0) {
        printf("av_rescale_rnd error\n");
        return -1;
    }

    if (out_nb_samples > max_out_nb_samples) {
        av_free(resampled_data[0]);
        ret = av_samples_alloc(resampled_data, &out_linesize, out_nb_channels, out_nb_samples, out_sample_fmt, 1);
        max_out_nb_samples = out_nb_samples;
    }

    if (swr_ctx) {
        ret = swr_convert(swr_ctx, resampled_data, out_nb_samples,
                          (const uint8_t **)audio_decode_frame->data, audio_decode_frame->nb_samples);
        if (ret < 0) {
            printf("swr_convert_error\n");
            return -1;
        }

        resampled_data_size = av_samples_get_buffer_size(&out_linesize, out_nb_channels, ret, out_sample_fmt, 1);
        if (resampled_data_size < 0) {
            printf("av_samples_get_buffer_size error\n");
            return -1;
        }
    } else {
        printf("swr_ctx null error\n");
        return -1;
    }

    memcpy(out_buf, resampled_data[0], resampled_data_size);

    if (resampled_data) {
        av_freep(&resampled_data[0]);
    }
    av_freep(&resampled_data);
    resampled_data = NULL;

    if (swr_ctx) {
        swr_free(&swr_ctx);
    }
    return resampled_data_size;
}

int audio_decode_frame(CPlayer *cp, uint8_t *audio_buf, int buf_size){
    AudioState *is = cp->is;
    AVPacket *pkt = &is->audio_pkt;

    int len1, data_size = 0;

    for (;;) {
        while (is->audio_pkt_size > 0) {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(is->audio_codec_ctx, &is->audio_frame, &got_frame, pkt);
            if (len1 < 0) {
                // error, skip frame
                is->audio_pkt_size = 0;
                break;
            }
            is->audio_pkt_data += len1;
            is->audio_pkt_size -= len1;
            data_size = 0;
            if (got_frame) {
                // resamplling and copy buf to audio_buf
                data_size = audio_resampling(is->audio_codec_ctx, &is->audio_frame, AV_SAMPLE_FMT_S16, is->audio_frame.channels, is->audio_frame.sample_rate, audio_buf);
                assert(data_size <= buf_size);
            }
            if (data_size <= 0) {
                // No data yet, get more frames
                continue;
            }
            int n = 2 * is->audio_frame.channels;
            is->audio_clock += (double)data_size / (double)(n * is->audio_frame.sample_rate);
            // We have data, return it and come back for more later
            return data_size;
        }

        if (pkt->data) {
            av_packet_unref(pkt);
        }

        if (is->quit) {
            return -1;
        }

        if (packet_queue_get(&is->audio_queue, pkt, 1) < 0) {
            return -1;
        }

        if (pkt->data == flush_pkt.data) {
            avcodec_flush_buffers(is->audio_codec_ctx);
            continue;
        }

        is->audio_pkt_data = pkt->data;
        is->audio_pkt_size = pkt->size;
        // if update, update the audio clock w/pts
        if (pkt->pts != AV_NOPTS_VALUE) {
            is->audio_clock = av_q2d(is->audio_codec_ctx->pkt_timebase) * pkt->pts;
        }
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len){
    CPlayer *cp = (CPlayer *)userdata;
    AudioState *is = cp->is;
    int len1, audio_size;

    while (len > 0) {
        if (is->audio_buf_index >= is->audio_buf_size) {
            // We have already sent all our data; get more */
            audio_size = audio_decode_frame(cp, is->audio_buf, sizeof(is->audio_buf));
            if (audio_size < 0) {
                // If error, output silence
                is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE; // eh...
                memset(is->audio_buf, 0, is->audio_buf_size);
            } else {
                is->audio_buf_size = audio_size;
            }
            is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if(len1 > len)
            len1 = len;
        if (!is->muted && is->audio_volume == SDL_MIX_MAXVOLUME) {
            memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        }
        else {
            memset(stream, is->silence_buf[0], len1);
            if (!is->muted)
                SDL_MixAudio(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1, is->audio_volume);
        }
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}


static int audio_open(CPlayer *arg) {
    CPlayer *cp = arg;
    AudioState *is = cp->is;
    SDL_AudioSpec wanted_spec, spec;

    // Set audio settings from codec info
    wanted_spec.freq = is->audio_codec_ctx->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = is->audio_codec_ctx->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = cp;

    if (is->audio_opend) {
        SDL_CloseAudio();
    }
    if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
        fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
        return -1;
    }
    is->audio_opend = 1;

    SDL_PauseAudio(0);
    return 0;
}

static int audio_close() {
    SDL_PauseAudio(1);
    return 0;
}

static int read_thread(void *arg) {
    CPlayer *cp = (CPlayer *)arg;
    AudioState *is = cp->is;
    AVPacket packet;

start:
    // prepare clean stuff
    is->audio_buf_index = 0;
    is->audio_buf_size = 0;
    is->audio_pkt_size = 0;
    // clean work
    if (is->audio_codec_ctx_orig) {
        avcodec_close(is->audio_codec_ctx_orig);
        is->audio_codec_ctx_orig = NULL;
    }
    if (is->audio_codec_ctx) {
        avcodec_close(is->audio_codec_ctx);
        is->audio_codec_ctx = NULL;
    }
    if (is->format_ctx) {
        avformat_close_input(&is->format_ctx);
        is->format_ctx = NULL;
    }

    // Open audio file
    if (avformat_open_input(&is->format_ctx, cp->input_filename, NULL, NULL) != 0) {
        printf("avformat_open_input Failed: %s\n", cp->input_filename);
        return -1;  // Error
    }

    // Retrieve stream information
    if (avformat_find_stream_info(is->format_ctx, NULL) < 0) {
        printf("avformat find_stream Failed\n");
        return -1;
    }

    is->audio_stream_index = -1;
    int i;
    for (i = 0; i < is->format_ctx->nb_streams; i++) {
        if (is->format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && is->audio_stream_index < 0) {
            is->audio_stream_index = i;
        }
    }
    if (is->audio_stream_index == -1) {
        printf("audio_stream_index == -1, return -1;\n");
        return -1;
    }

    if (is->format_ctx->duration != AV_NOPTS_VALUE) {
        int secs;
        int64_t duration = is->format_ctx->duration + (is->format_ctx->duration <= INT64_MAX - 5000 ? 5000 : 0);
        secs  = duration / AV_TIME_BASE;
        is->duration = secs;
    }

    is->audio_codec_ctx_orig = is->format_ctx->streams[is->audio_stream_index]->codec;
    is->audio_codec = avcodec_find_decoder(is->audio_codec_ctx_orig->codec_id);
    if (!is->audio_codec) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    // Copy context
    is->audio_codec_ctx = avcodec_alloc_context3(is->audio_codec);
    if (avcodec_copy_context(is->audio_codec_ctx, is->audio_codec_ctx_orig) != 0) {
        fprintf(stderr, "Couldn't copy codec context\n");
        return -1;
    }


    // Open audio device
    audio_open(cp);

    if(avcodec_open2(is->audio_codec_ctx, is->audio_codec, NULL)<0) {
        printf("avcodec_open2 failed\n");
        return -1;
    }

    // Read frames and put to audio_queue
    for (;;) {
        if (is->quit) {
            break;
        }
        if (is->read_thread_abord) {
            is->read_thread_abord = 0;
            packet_queue_flush(&is->audio_queue);
            goto start;
        }
        // handle seek stuff
        if (is->seek_req) {
            int stream_index = -1;
            int64_t seek_target = is->seek_pos;

            if (is->audio_stream_index >= 0) {
                stream_index = is->audio_stream_index;
            }
            if (stream_index >= 0) {
                seek_target = av_rescale_q(seek_target, AV_TIME_BASE_Q,
                                           is->format_ctx->streams[stream_index]->time_base);
            }
            if (av_seek_frame(is->format_ctx, stream_index, seek_target,
                              is->seek_flags) < 0) {
                fprintf(stderr, "seek error\n");
            } else {
                if (is->audio_stream_index >= 0) {
                    packet_queue_flush(&is->audio_queue);
                    packet_queue_put(&is->audio_queue, &flush_pkt);
                }
            }
            is->seek_req = 0;
        }

        if (is->audio_queue.size > MAX_AUDIOQ_SIZE) {
            SDL_Delay(10);
            continue;
        }

        if (av_read_frame(is->format_ctx, &packet) < 0) {
            if (!is->read_thread_abord) {
                SDL_Delay(100); /* no error; wait for user input */
                continue;
            } else {
                is->read_thread_abord = 0;
                packet_queue_flush(&is->audio_queue);
                goto start;
            }
        }

        if (packet.stream_index == is->audio_stream_index) {
            packet_queue_put(&is->audio_queue, &packet);
        } else {
            av_packet_unref(&packet); // Free the packet
        }
    }
    return 0;
}

static void stream_close(CPlayer *cp) {
    AudioState *is = cp->is;
    is->quit = 1;
    av_log(NULL, AV_LOG_WARNING, "wait for read_tid\n");
    SDL_WaitThread(is->read_tid, NULL);
    audio_close();
    if (is->audio_codec_ctx_orig) {
        avcodec_close(is->audio_codec_ctx_orig);
        is->audio_codec_ctx_orig = NULL;
    }
    if (is->audio_codec_ctx) {
        avcodec_close(is->audio_codec_ctx);
        is->audio_codec_ctx = NULL;
    }
    if (is->format_ctx) {
        avformat_close_input(&is->format_ctx);
        is->format_ctx = NULL;
    }
    packet_queue_destory(&is->audio_queue);
    av_free(is);
}

AudioState *stream_open(CPlayer *cp, const char *filename) {
    assert(!cp->is);
    AudioState *is;
    is = (AudioState *) av_mallocz(sizeof(AudioState));
    if (!is)
        return NULL;
    cp->input_filename = av_strdup(filename);
    packet_queue_init(&is->audio_queue);
    is->audio_volume = SDL_MIX_MAXVOLUME;
    is->muted = 0;
    cp->is = is;

    is->read_tid = SDL_CreateThread(read_thread, "read_thread", cp);
    if (!is->read_tid) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateThread(): %s\n", SDL_GetError());
        stream_close(cp);
        return NULL;
    }
    return is;
}

void stream_seek(CPlayer *cp, int64_t pos, int flag) {
    AudioState *is = cp->is;
    if (!is->seek_req) {
        is->seek_pos = pos;
        is->seek_flags = flag < 0 ? AVSEEK_FLAG_BACKWARD : 0;
        is->seek_req = 1;
    }
}

CPlayer *player_create() {
    CPlayer *cp = (CPlayer *) av_mallocz(sizeof(CPlayer));
    if (!cp)
        return NULL;
    global_cplayer_ctx = cp;
    return cp;
}

void player_destory(CPlayer *cp) {
    if (!cp)
        return;
    if (cp->is) {
        stream_close(cp);
        av_log(NULL, AV_LOG_WARNING, "destroy_cplayer: force stream_close()\n");
        cp->is = NULL;
    }
    av_free(cp);
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
    pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;

    SDL_LockMutex(q->mutex);

    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    no_more_data_in_the_queue = 0;
    // Send signal to queue get function
    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
    return 0;
}

// if block set to True, this func will wait for SDL_CondSignal
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {

        if (global_cplayer_ctx->is->quit) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            no_more_data_in_the_queue = 0;
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            if (q->nb_packets <= 0) {
                no_more_data_in_the_queue = 1;
                ret = -1;
                break;
            }
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

void cp_pause_audio() {
    if (global_cplayer_ctx == NULL)
        return;
    AudioState *is = global_cplayer_ctx->is;
    is->paused = !is->paused;
    SDL_PauseAudio(is->paused);
}

void cp_stop_audio() {
    if (global_cplayer_ctx == NULL)
        return;
    AudioState *is = global_cplayer_ctx->is;
    if (!is->stopped) {
        is->stopped = !is->stopped;
        audio_close();
        packet_queue_flush(&is->audio_queue);
        is->duration = 0;
        is->audio_clock = 0;
    }
}

void cp_set_volume(int volume) {
    if (global_cplayer_ctx == NULL)
        return;
    if (volume < 0 || volume > 100)
        return;
    AudioState *is = global_cplayer_ctx->is;
    volume = (SDL_MIX_MAXVOLUME / 100) * volume;
    is->audio_volume = volume;
}

void cp_seek_audio(double percent) {
    if (global_cplayer_ctx == NULL)
        return;
    if (percent < 0 || percent > 100){
        return;
    }
    int incr = 0;
    AudioState *is = global_cplayer_ctx->is;
    double seek_target = (double)is->duration * percent / 100.0;
    double current_pos = is->audio_clock;
    incr = seek_target > current_pos ? 1 : -1;
    stream_seek(global_cplayer_ctx, (int64_t)(seek_target * AV_TIME_BASE), incr);
}

void cp_seek_audio_by_sec(int sec) {
    if (global_cplayer_ctx == NULL)
        return;
    int incr = 0;
    incr = sec > 0 ? 1 : -1;
    AudioState *is = global_cplayer_ctx->is;
    double pos = is->audio_clock;
    pos += sec;
    int duration = is->duration;
    if (duration < pos || pos < 0) {
        return;
    }
    stream_seek(global_cplayer_ctx, (int64_t)(pos * AV_TIME_BASE), incr);
}

// return total audio length
int cp_get_time_length() {
    if (global_cplayer_ctx == NULL)
        return 0;
    int duration = 0;
    if (global_cplayer_ctx != NULL) {
        duration = global_cplayer_ctx->is->duration;
    }
    return duration;
}

// return current time pos
double cp_get_current_time_pos() {
    if (global_cplayer_ctx == NULL)
        return 0;
    double pos = 0;
    if (global_cplayer_ctx != NULL) {
        pos = global_cplayer_ctx->is->audio_clock;
    }
    return pos;
}

int cp_is_stopping() {
    return no_more_data_in_the_queue;
}

void cp_free_player() {
    if (global_cplayer_ctx != NULL) {
        player_destory(global_cplayer_ctx);
    }
}

CPlayer *cp_load_file(const char *filename) {
    CPlayer *cp = NULL;
    if (global_cplayer_ctx == NULL) {
        global_init();
        cp = player_create();
        if (!cp) {
            printf("player_create error\n");
            return NULL;
        }
        AudioState *is = stream_open(cp, filename);
        if (!is) {
            printf("is is null\n");
            return NULL;
        }
    } else {
        audio_close();
        no_more_data_in_the_queue = 0;
        cp = global_cplayer_ctx;
        AudioState *is = cp->is;
        cp->input_filename = av_strdup(filename);
        is->read_thread_abord = 1;
    }
    return cp;
}
