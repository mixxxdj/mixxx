// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include <bungee/Bungee.h>

#define CXXOPTS_NO_EXCEPTIONS
#include "cxxopts.hpp"
#undef CXXOPTS_NO_EXCEPTIONS

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace Bungee::CommandLine {

static void fail(const char *message)
{
	std::cerr << "Fatal error: " << message << "\n";
	exit(1);
}

struct Options :
	cxxopts::Options
{
	std::vector<std::string> helpGroups;

	Options(std::string program_name, std::string help_string) :
		cxxopts::Options(program_name, help_string)
	{
		add_options() //
			("input", "input WAV filename", cxxopts::value<std::string>()) //
			("output", "output WAV filename", cxxopts::value<std::string>()) //
			("start", "start time in seconds", cxxopts::value<double>()->default_value("+0")) //
			("stop", "stop time in seconds", cxxopts::value<double>()->default_value("-0")) //
			;
		add_options(helpGroups.emplace_back("Sample rate")) //
			("output-rate", "output sample rate, Hz, or 0 to match input sample rate", cxxopts::value<int>()->default_value("0")) //
			;
		add_options(helpGroups.emplace_back("Stretch")) //
			("s,speed", "output speed as multiple of input speed", cxxopts::value<double>()->default_value("1")) //
			("p,pitch", "output pitch shift in semitones", cxxopts::value<double>()->default_value("0")) //
			;
		auto optionAdder = add_options(helpGroups.emplace_back("Processing"));

#define X_BEGIN(Type, type) \
		{ \
			std::string names, d, a = "[";
#define X_END(Type, type) \
			optionAdder(#type, names + "]", cxxopts::value<std::string>()->default_value(d)); \
		}
#define X_ITEM(Type, type, mode, description) \
		names += a + #mode; \
		a = "|"; \
		if (Request{}.type##Mode == type##Mode_##mode) \
			d = #mode;

		BUNGEE_MODES

#undef X_BEGIN
#undef X_END
#undef X_ITEM
			//
			;
		add_options(helpGroups.emplace_back("Developer")) //
			("grain", "increases [+1] or decreases [-1] grain duration by a factor of two", cxxopts::value<int>()->default_value("0")) //
			("push", "input chunk size (0 for pull operation, negative for random push chunk size)", cxxopts::value<int>()->default_value("0")) //
			("instrumentation", "report useful diagnostic information to system log") //
			;
		add_options(helpGroups.emplace_back("Help")) //
			("h,help", "display this message") //
			;
		custom_help("[options...]");
		parse_positional({"input", "output"});
		positional_help("input.wav output.wav");
	}
};

struct Parameters :
	cxxopts::ParseResult
{
	Parameters(Options &options, int argc, const char *argv[], Request &request) :
		cxxopts::ParseResult(options.parse(argc, argv))
	{
		if (count("help"))
		{
			std::cout << options.help(options.helpGroups) << std::endl;
			exit(0);
		}

		if (!unmatched().empty())
			fail("unrecognised command parameter(s)");

		if (!count("input"))
			fail("no input file specified");

		if (!count("output"))
			fail("no output file specified");

		const auto semitones = (*this)["pitch"].as<double>();
		if (semitones < -48. || semitones > +48.)
			fail("pitch is outside of the range -48 to +48");
		request.pitch = std::pow(2., semitones / 12);

		request.speed = (*this)["speed"].as<double>();
		if (std::abs(request.speed) > 100.)
			fail("speed is outside of the range -100 to +100");

		if (std::abs((*this)["grain"].as<int>()) > 1)
			fail("grain is outside of the range -1 to +1");

		if ((*this)["push"].as<int>() && request.speed < 0.)
			fail("speed not greater than zero in 'push' mode");

#define X_BEGIN(Type, type) \
		{ \
			const auto s = (*this)[#type].as<std::string>(); \
			if (false) \
			{ \
			}

#define X_ITEM(Type, type, mode, description) \
			else if (s == #mode) \
			{ \
				request.type##Mode = type##Mode_##mode; \
			}

#define X_END(Type, type) \
			else \
			{ \
				Bungee::CommandLine::fail("Unrecognised value for --" #type); \
			} \
		}

		BUNGEE_MODES

#undef X_BEGIN
#undef X_ITEM
#undef X_END
	}
};

struct Processor
{
	struct OutputChunkBuffer :
		OutputChunk
	{
		std::vector<float> audio;
		std::vector<float *> channelPointers;
		Request request[2]{};

		OutputChunkBuffer(int frameCount, int channelCount) :
			OutputChunk{},
			audio(frameCount * channelCount),
			channelPointers(channelCount)
		{
			OutputChunk::request[0] = &request[0];
			OutputChunk::request[1] = &request[1];

			OutputChunk::data = audio.data();
			OutputChunk::channelStride = frameCount;

			for (int c = 0; c < channelCount; ++c)
				channelPointers[c] = audio.data() + c * OutputChunk::channelStride;
		}

		OutputChunkBuffer(const OutputChunkBuffer &) = delete;
		OutputChunkBuffer &operator=(const OutputChunkBuffer &) = delete;

		const OutputChunk &outputChunk(int frameCount, double positionBegin, double positionEnd)
		{
			OutputChunk::frameCount = frameCount;
			request[0].position = positionBegin;
			request[1].position = positionEnd;
			request[0].speed = 1.;
			request[1].speed = 1.;
			return *this;
		}
	};

	std::vector<char> wavHeader;
	std::vector<char> wavData;
	decltype(wavData.begin()) o;
	SampleRates sampleRates;
	int inputFrameCount;
	int inputChannelStride;
	int sampleFormat;
	int channelCount;
	int bitsPerSample;
	std::vector<float> inputBuffer;
	std::ifstream inputFile;
	std::ofstream outputFile;

	Processor(const cxxopts::ParseResult &parameters, Request &request) :
		inputFile(parameters["input"].as<std::string>(), std::ios::binary)
	{
		if (!inputFile)
			fail("Please check your input file: could not open it");

		wavHeader.resize(20);
		inputFile.read(wavHeader.data(), wavHeader.size());

		if (read<uint32_t>(&wavHeader[0]) != read<uint32_t>("RIFF"))
			fail("Please check your input file: it seems not to be a compatible WAV file (no 'RIFF')");
		if (read<uint32_t>(&wavHeader[8]) != read<uint32_t>("WAVE"))
			fail("Please check your input file: it seems not to be a compatible WAV file (no 'WAVE')");
		if (read<uint32_t>(&wavHeader[12]) != read<uint32_t>("fmt "))
			fail("Please check your input file: it seems not to be a compatible WAV file (no 'fmt ')");
		if (read<uint32_t>(&wavHeader[16]) < 16)
			fail("Please check your input file: it seems not to be a compatible WAV file (format length less than 16)'");

		int subchunkCount = 0;
		while (read<uint32_t>(&wavHeader[wavHeader.size() - 8]) != read<uint32_t>("data"))
		{
			const auto len = read<uint32_t>(&wavHeader[wavHeader.size() - 4]) + 8;
			wavHeader.resize(wavHeader.size() + len);
			if (!inputFile.read(wavHeader.data() + wavHeader.size() - len, len))
				fail("Please check your input file: there was a problem reading one of its chunks");

			if (subchunkCount++ == 0)
			{
				sampleRates.input = read<uint32_t>(&wavHeader[24]);
				if (sampleRates.input < 8000 || sampleRates.input > 192000)
					fail("Please check your input file: it seems not to be a compatible WAV file (unexpected sample rate)");

				if (parameters["output-rate"].has_default())
					sampleRates.output = sampleRates.input;
				else
					sampleRates.output = parameters["output-rate"].as<int>();

				if (sampleRates.output < 8000 || sampleRates.output > 192000)
					fail("Output sample rate must be in the range [8000, 192000] kHz");

				sampleFormat = read<uint16_t>(&wavHeader[20]);
				channelCount = read<uint16_t>(&wavHeader[22]);
				bitsPerSample = read<uint16_t>(&wavHeader[34]);
				if (!channelCount)
					fail("Please check your input file: it seems not to be a compatible WAV file (zero channels)");
				if (read<int32_t>(&wavHeader[28]) != sampleRates.input * channelCount * bitsPerSample / 8)
					fail("Please check your input file: it seems not to be a compatible WAV file (inconsistent at position 28)");
				if (read<uint16_t>(&wavHeader[32]) != channelCount * bitsPerSample / 8)
					fail("Please check your input file: it seems not to be a compatible WAV file (inconsistent at position 32)'");
			}
		}

		{
			const auto frameStart = std::round(parameters["start"].as<double>() * sampleRates.input);
			const auto frameStop = std::round(parameters["stop"].as<double>() * sampleRates.input);

			if (sampleFormat == 1 && bitsPerSample == 16)
				readInputAudio<int16_t>(frameStart, frameStop);
			else if (sampleFormat == 1 && bitsPerSample == 32)
				readInputAudio<int32_t>(frameStart, frameStop);
			else if (sampleFormat == 3 && bitsPerSample == 32)
				readInputAudio<float>(frameStart, frameStop);
			else
				fail("Please check your input file: its sample format is not supported");
		}

		std::fill(wavData.begin(), wavData.end(), 0);

		outputFile.open(parameters["output"].as<std::string>(), std::ios::binary);
		if (!outputFile)
			fail("Please check your output path: there was a problem opening the output file");

		{
			constexpr size_t maximumOutputDataBytes = 1ll << 30; // 1G
			const size_t bytesPerFrame = channelCount * bitsPerSample / 8;
			const size_t maximumOutputFrameCount = maximumOutputDataBytes / bytesPerFrame;

			size_t outputFrameCount = std::floor(inputFrameCount / std::fabs(request.speed) * sampleRates.output / sampleRates.input);
			if (outputFrameCount > maximumOutputFrameCount)
			{
				outputFrameCount = maximumOutputFrameCount;
				std::cerr << "Warning: output audio will be truncated to 1GB\n";
			}

			const size_t wavDataBytes = outputFrameCount * bytesPerFrame;
			wavData.resize(wavDataBytes);
		}

		restart(request);
	}

	void restart(Request &request)
	{
		o = wavData.begin();

		if (request.speed < 0)
			request.position = inputFrameCount - 1;
		else
			request.position = 0.;
	}

	bool write(OutputChunk outputChunk)
	{
		const auto positionBegin = outputChunk.request[0]->position;
		const auto positionEnd = outputChunk.request[1]->position;

		if (!std::isnan(positionBegin) && positionBegin != positionEnd)
		{
			double nPrerollInput = outputChunk.request[0]->speed < 0. ? positionBegin - inputFrameCount + 1 : -positionBegin;
			nPrerollInput = std::max(0, (int)std::round(nPrerollInput));

			const int nPrerollOutput = (int)std::round(nPrerollInput * (outputChunk.frameCount / std::abs(positionEnd - positionBegin)));

			if (outputChunk.frameCount > nPrerollOutput)
			{
				outputChunk.frameCount -= nPrerollOutput;
				outputChunk.data += nPrerollOutput;
				return writeChunk(outputChunk);
			}
		}

		return false;
	}

	const float *getInputAudio(InputChunk inputChunk) const
	{
		return inputBuffer.data() + inputChunk.begin;
	}

	void getInputAudio(float *p, int stride, int position, int length) const
	{
		for (int c = 0; c < channelCount; ++c)
			for (int i = 0; i < length; ++i)
			{
				if (position + i >= 0 && position + i < inputFrameCount)
					p[c * stride + i] = inputBuffer[c * inputChannelStride + position + i];
				else
					p[c * stride + i] = 0.f;
			}
	}

	template <typename Sample>
	bool writeSamples(Bungee::OutputChunk chunk)
	{
		const int count = std::min<int>(chunk.frameCount * channelCount, int((wavData.end() - o) / sizeof(Sample)));

		for (int f = 0; f < count / channelCount; ++f)
			for (int c = 0; c < channelCount; ++c)
			{
				write<Sample>(&*o, fromFloat<Sample>(chunk.data[f + c * chunk.channelStride]));
				o += sizeof(Sample);
			}

		return o == wavData.end();
	}

	bool writeChunk(Bungee::OutputChunk chunk)
	{
		if (sampleFormat == 3)
			return writeSamples<float>(chunk);
		if (bitsPerSample == 32)
			return writeSamples<int32_t>(chunk);
		else
			return writeSamples<int16_t>(chunk);
	}

	void writeOutputFile()
	{
		write<uint32_t>(&wavHeader[4], uint32_t(wavHeader.size() + wavData.size() - 8));
		write<uint32_t>(&wavHeader[24], uint32_t(sampleRates.output));
		write<uint32_t>(&wavHeader[28], uint32_t(sampleRates.output * channelCount * bitsPerSample / 8));
		write<uint32_t>(&wavHeader[wavHeader.size() - 4], uint32_t(wavData.size()));

		outputFile.write(wavHeader.data(), wavHeader.size());
		outputFile.write(wavData.data(), wavData.size());
	}

	template <typename Sample>
	void readInputAudio(double frameStart, double frameStop)
	{
		wavData.resize(read<uint32_t>(&wavHeader[wavHeader.size() - 4]));
		if (!inputFile.read(wavData.data(), wavData.size()))
			fail("Please check your input file: there was a problem reading its audio data");

		inputFrameCount = int(8 * wavData.size() / bitsPerSample / channelCount);

		if (std::signbit(frameStart))
			frameStart += inputFrameCount;
		if (std::signbit(frameStop))
			frameStop += inputFrameCount;

		if (frameStart < 0 || frameStart >= inputFrameCount || frameStop < 0 || frameStop > inputFrameCount || frameStart >= frameStop)
			fail("Please check your start/stop times: they are outside the range of the input audio");

		inputChannelStride = inputFrameCount = int(frameStop - frameStart);
		inputBuffer.resize(channelCount * inputChannelStride);

		for (int i = 0; i < inputFrameCount; ++i)
			for (int c = 0; c < channelCount; ++c)
				inputBuffer[c * inputChannelStride + i] = toFloat(read<Sample>(&wavData[((i + frameStart) * channelCount + c) * sizeof(Sample)]));
	}

	template <typename Type>
	static inline Type read(const char *data)
	{
		if constexpr (std::is_same_v<Type, float>)
			return std::bit_cast<float>(read<uint32_t>(data));
		else
		{
			Type value = 0;
			for (unsigned i = 0; i < sizeof(Type); ++i)
				value |= (Type(data[i]) & 0xff) << 8 * i;
			return value;
		}
	}

	template <typename Type>
	static inline void write(char *data, Type value)
	{
		if constexpr (std::is_same_v<Type, float>)
			write(data, std::bit_cast<uint32_t>(value));
		else
			for (unsigned i = 0; i < sizeof(Type); ++i)
				data[i] = value >> 8 * i;
	}

	template <typename Sample>
	static inline float toFloat(Sample x)
	{
		if (std::is_same_v<Sample, float>)
			return x;
		constexpr float k = -1.f / std::numeric_limits<Sample>::min();
		return k * x;
	}

	template <typename Sample>
	static inline Sample fromFloat(float x)
	{
		if constexpr (std::is_same_v<Sample, float>)
			return x;
		x = std::ldexp(x, 8 * sizeof(Sample) - 1);
		x = std::round(x);
		if (x < std::numeric_limits<Sample>::min())
			return std::numeric_limits<Sample>::min();
		if (x >= -(float)std::numeric_limits<Sample>::min())
			return std::numeric_limits<Sample>::max();
		return static_cast<Sample>(x);
	}
};

} // namespace Bungee::CommandLine
