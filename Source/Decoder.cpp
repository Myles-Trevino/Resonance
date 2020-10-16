/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	https://www.apache.org/licenses/LICENSE-2.0
*/


#include "Decoder.hpp"

#include <stdexcept>
#include <cmath>

extern "C"
{
	#include <ffmpeg/libavformat/avformat.h>
	#include <ffmpeg/libavcodec/avcodec.h>
	#include <ffmpeg/libswresample/swresample.h>
	#include <libavutil/opt.h>
}


namespace
{
	constexpr int channel_count{1};
	constexpr AVSampleFormat sample_format{AV_SAMPLE_FMT_FLTP};

	AVFormatContext* format_context;
	AVCodec* codec;
	AVCodecContext* codec_context;
	struct SwrContext* resampler_context;
	AVFrame* frame;
	AVPacket packet;
	uint64_t sample;
	float** resample_buffer;
	uint64_t channel_layout;

	int stream_index;
	uint64_t clipped_samples;
	float peak;
	unsigned original_channel_count;
	unsigned original_sample_rate;
	unsigned original_bit_depth;

	std::vector<float> data;


	// Processes and saves the decoded frames to the data vector.
	int process_decoded_frames()
	{
		while(true)
		{
			// Retrieve a frame from the decoder.
			int error{avcodec_receive_frame(codec_context, frame)};
			if(error < 0) return error;

			// Allocate the resample buffer.
			int buffer_size{swr_get_out_samples(resampler_context, frame->nb_samples)};

			if(av_samples_alloc_array_and_samples(
				reinterpret_cast<uint8_t***>(&resample_buffer), nullptr,
				channel_count, buffer_size, sample_format, 0) < 0)
				throw std::runtime_error{"Could not allocate a resample buffer."};

			// Resample the frame.
			int samples_written{swr_convert(resampler_context,
				reinterpret_cast<uint8_t**>(resample_buffer), buffer_size,
				const_cast<const uint8_t**>(frame->extended_data), frame->nb_samples)};

			if(samples_written < 0) throw std::runtime_error{"Could not resample a frame."};

			// For each processed sample...
			peak = 0;
			for(int j{}; j < samples_written; ++j)
			{
				// Make sure the sample is in range.
				float sample{resample_buffer[0][j]};
				float absolute_value_sample{std::abs(sample)};
				if(absolute_value_sample > peak) peak = absolute_value_sample;
				if(sample > 1){ ++clipped_samples; sample = 1; }
				if(sample < -1){ ++clipped_samples; sample = -1; }

				// Store the sample.
				data.emplace_back(sample);
			}

			// Destroy.
			av_freep(&resample_buffer[0]);
			av_freep(&resample_buffer);
			av_frame_unref(frame);
		}
	}


	// Decodes the recieved audio packet.
	void decode_packet()
	{
		// If the packet is not from the desired stream, destroy it and return.
		if(packet.stream_index != stream_index)
		{
			av_packet_unref(&packet);
			return;
		}

		// Otherwise, send the packet to the decoder.
		if(avcodec_send_packet(codec_context, &packet))
			throw std::runtime_error{"Could not send a packet to the decoder."};

		av_packet_unref(&packet);

		// Retrieve and process the decoded frames.
		if(process_decoded_frames() != AVERROR(EAGAIN))
			throw std::runtime_error{"Could not process a decoded frame."};
	}
}


// Loads the information from the given audio file.
void LV::Decoder::load_track_information(const std::string& file)
{
	av_log_set_level(AV_LOG_QUIET);

	// Allocate the format context.
	format_context = avformat_alloc_context();
	if(!format_context) throw std::runtime_error{"Could not allocate the format context."};

	// Open the file.
	if(avformat_open_input(&format_context, file.c_str(), nullptr, nullptr))
		throw std::runtime_error{"Could not open the file \""+file+"\"."};

	// Retrieve the file's stream information.
	if(avformat_find_stream_info(format_context, nullptr) < 0)
		throw std::runtime_error{"Could not retrieve the stream information."};

	// Find the audio stream and codec.
	stream_index = av_find_best_stream(format_context,
		AVMEDIA_TYPE_AUDIO, -1, -1, &codec, NULL);
	if(stream_index < 0) throw std::runtime_error{"Could not find a supported audio stream."};
	AVStream *stream{format_context->streams[stream_index]};

	// Initialize the codec context.
	codec_context = avcodec_alloc_context3(codec);
	if(!codec_context) throw std::runtime_error{"Could not initialize the codec context."};

	// Fill the codec context with the parameters of the stream's codec.
	if(avcodec_parameters_to_context(codec_context, stream->codecpar) < 0)
		throw std::runtime_error{"Could not set the codec context's parameters."};

	// Retrieve the channel count, sample rate, and bit depth.
	original_channel_count = static_cast<unsigned>(codec_context->channels);
	original_sample_rate = static_cast<unsigned>(codec_context->sample_rate);
	original_bit_depth = static_cast<unsigned>(codec_context->bits_per_raw_sample);

	// Determine the channel layout.
	channel_layout = codec_context->channel_layout;
	if(!channel_layout) channel_layout = static_cast<uint64_t>(
		av_get_default_channel_layout(codec_context->channels));
}


// Initializes the resampler and decoder with the given settings.
void LV::Decoder::initialize_resampler_and_decoder()
{
	// Initialize the resampler.
	resampler_context = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_MONO,
		sample_format, original_sample_rate, channel_layout,
		codec_context->sample_fmt, original_sample_rate, 0, nullptr);

	if(av_opt_set_int(resampler_context, "resampler", SWR_ENGINE_SOXR, NULL))
		throw std::runtime_error{"Could not enable the SOX resampler."};

	if(av_opt_set_int(resampler_context, "precision", 33, NULL))
		throw std::runtime_error{"Could not set the SOX resampler precision."};

	if(av_opt_set_int(resampler_context, "cheby", 1, NULL))
		throw std::runtime_error{"Could not enable Chebyshev passband rolloff."};

	if(av_opt_set_int(resampler_context, "dither_method", SWR_DITHER_NS_SHIBATA, NULL))
		throw std::runtime_error{"Could not enable Shibata noise shaping dithering."};

	swr_init(resampler_context);
	if(!swr_is_initialized(resampler_context))
		throw std::runtime_error{"Could not initialize the resampler."};

	// Initialize the decoder.
	if(avcodec_open2(codec_context, codec, nullptr) < 0)
		throw std::runtime_error{"Could not initialize the decoder."};

	// Create the frame.
	frame = av_frame_alloc();
	if(!frame) throw std::runtime_error{"Could not create the frame."};

	// Initialize the packet.
	av_init_packet(&packet);
	if(!&packet) throw std::runtime_error{"Could not initialize the packet."};

	// Allocate the channels.
	data.resize(channel_count);
}


// Loads the entire audio file into the data vector.
void LV::Decoder::load_samples()
{
	// Read packets until there are none left.
	while(av_read_frame(format_context, &packet) != AVERROR_EOF) decode_packet();
}


// Resets values and deallocates any resources.
void LV::Decoder::destroy()
{
	data.clear();

	if(resample_buffer && resample_buffer[0]) av_freep(&resample_buffer[0]);
	if(resample_buffer) av_freep(&resample_buffer);

	if(&packet) av_packet_unref(&packet);
	if(frame) av_frame_free(&frame);

	if(resampler_context) swr_free(&resampler_context);
	if(codec_context){ avcodec_close(codec_context); avcodec_free_context(&codec_context); }
	if(format_context) avformat_close_input(&format_context);
	if(format_context) avformat_free_context(format_context);
}


const std::vector<float>& LV::Decoder::get_data(){ return data; }

const int LV::Decoder::get_sample_rate(){ return original_sample_rate; }
