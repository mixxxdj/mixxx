#!/usr/bin/env python
import argparse
import sys

# To use, run this from the top level of the Git repository tree:
# scripts/generate_sample_functions.py --sample_autogen_h src/util/sample_autogen.h --channelmixer_autogen_cpp src/engine/channelmixer_autogen.cpp

BASIC_INDENT = 4

COPY_WITH_GAIN_METHOD_PATTERN = 'copy%(i)dWithGain'
def copy_with_gain_method_name(i):
    return COPY_WITH_GAIN_METHOD_PATTERN % {'i' : i}

RAMPING_GAIN_METHOD_PATTERN = 'copy%(i)dWithRampingGain'
def copy_with_ramping_gain_method_name(i):
    return RAMPING_GAIN_METHOD_PATTERN % {'i': i}

def method_call(method_name, args):
    return '%(method_name)s(%(args)s)' % {'method_name': method_name,
                                          'args': ', '.join(args)}

def hanging_indent(base, groups, hanging_suffix, terminator, depth=0):
    return map(lambda line: ' ' * BASIC_INDENT * depth + line, map(''.join, zip(
        [base] + [' ' * len(base)] * (len(groups) - 1),
        groups,
        [hanging_suffix] * (len(groups) - 1) + [terminator])))

def write_channelmixer_autogen(output, num_channels):
    output.append('#include "engine/channelmixer.h"')
    output.append('#include "util/sample.h"')
    output.append('#include "util/timer.h"')
    output.append('////////////////////////////////////////////////////////')
    output.append('// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //')
    output.append('// SEE scripts/generate_sample_functions.py           //')
    output.append('////////////////////////////////////////////////////////')
    output.append('')
    output.append('// static')

    def write_applyeffectsandmixchannels(inplace, output):
        header = 'void ChannelMixer::applyEffects%(inplace)sAndMixChannels(' % {'inplace': 'InPlace' if inplace else ''}
        args = ['const EngineMaster::GainCalculator& gainCalculator',
                'QVarLengthArray<EngineMaster::ChannelInfo*, kPreallocatedChannels>* activeChannels',
                'QVarLengthArray<EngineMaster::GainCache, kPreallocatedChannels>* channelGainCache',
                'CSAMPLE* pOutput', 'const ChannelHandle& outputHandle',
                'unsigned int iBufferSize', 'unsigned int iSampleRate',
                'EngineEffectsManager* pEngineEffectsManager']
        output.extend(hanging_indent(header, args, ',', ') {'))

        def write(data, depth=0):
            output.append(' ' * (BASIC_INDENT * depth) + data)

        if inplace:
            write('// Signal flow overview:', depth=1)
            write('// 1. Calculate gains for each channel', depth=1)
            write('// 2. Pass each channel\'s calculated gain and input buffer to pEngineEffectsManager, which then:', depth=1)
            write('//    A) Applies the calculated gain to the channel buffer, modifying the original input buffer', depth=1)
            write('//    B) Applies effects to the buffer, modifying the original input buffer', depth=1)
            write('// 4. Mix the channel buffers together to make pOutput, overwriting the pOutput buffer from the last engine callback', depth=1)
        else:
            write('// Signal flow overview:', depth=1)
            write('// 1. Clear pOutput buffer', depth=1)
            write('// 2. Calculate gains for each channel', depth=1)
            write('// 3. Pass each channel\'s calculated gain and input buffer to pEngineEffectsManager, which then:', depth=1)
            write('//     A) Copies each channel input buffer to a temporary buffer', depth=1)
            write('//     B) Applies gain to the temporary buffer', depth=1)
            write('//     C) Processes effects on the temporary buffer', depth=1)
            write('//     D) Mixes the temporary buffer into pOutput', depth=1)
            write('// The original channel input buffers are not modified.', depth=1)

        write('int totalActive = activeChannels->size();', depth=1)
        if not inplace:
            write('SampleUtil::clear(pOutput, iBufferSize);', depth=1)
        write('if (totalActive == 0) {', depth=1)
        write('//ScopedTimer t("EngineMaster::applyEffects%(inplace)sAndMixChannels_0active");' %
              {'inplace': 'InPlace' if inplace else ''}, depth=2)
        if inplace:
            write('SampleUtil::clear(pOutput, iBufferSize);', depth=2)
        for i in xrange(1, num_channels+1):
            write('} else if (totalActive == %d) {' % i, depth=1)
            write('//ScopedTimer t("EngineMaster::applyEffects%(inplace)sAndMixChannels_%(i)dactive");' %
                  {'inplace': 'InPlace' if inplace else '', 'i': i}, depth=2)
            write('CSAMPLE_GAIN oldGain[%(i)d];' % {'i': i}, depth=2)
            write('CSAMPLE_GAIN newGain[%(i)d];' % {'i': i}, depth=2)
            for j in xrange(i):
                write('EngineMaster::ChannelInfo* pChannel%(j)d = activeChannels->at(%(j)d);' % {'j': j}, depth=2)
                write('const int channelIndex%(j)d = pChannel%(j)d->m_index;' % {'j': j}, depth=2)
                write('EngineMaster::GainCache& gainCache%(j)d = (*channelGainCache)[channelIndex%(j)d];' % {'j': j}, depth=2)
                write('oldGain[%(j)d] = gainCache%(j)d.m_gain;' % {'j': j}, depth=2)
                write('if (gainCache%(j)d.m_fadeout) {' % {'j': j}, depth=2)
                write('newGain[%(j)d] = 0;' % {'j': j}, depth=3)
                write('gainCache%(j)d.m_fadeout = false;' % {'j': j}, depth=3)
                write('} else {', depth=2)
                write('newGain[%(j)d] = gainCalculator.getGain(pChannel%(j)d);' % {'j': j}, depth=3)
                write('}', depth=2)
                write('gainCache%(j)d.m_gain = newGain[%(j)d];' % {'j': j}, depth=2)
                write('CSAMPLE* pBuffer%(j)d = pChannel%(j)d->m_pBuffer;' % {'j': j}, depth=2)

            if inplace:
                write('// Process effects for each channel in place', depth=2)
            else:
                write('// Process effects for each channel and mix the processed signal into pOutput', depth=2)
            for j in xrange(i):
              if inplace:
                  write('pEngineEffectsManager->processPostFaderInPlace(pChannel%(j)d->m_handle, outputHandle, pBuffer%(j)d, iBufferSize, iSampleRate, pChannel%(j)d->m_features, oldGain[%(j)d], newGain[%(j)d]);' % {'j': j}, depth=2)
              else:
                  write('pEngineEffectsManager->processPostFaderAndMix(pChannel%(j)d->m_handle, outputHandle, pBuffer%(j)d, pOutput, iBufferSize, iSampleRate, pChannel%(j)d->m_features, oldGain[%(j)d], newGain[%(j)d]);' % {'j': j}, depth=2)

            if inplace:
                write('// Mix the effected channel buffers together to replace the old pOutput from the last engine callback', depth=2)
                write('for (unsigned int i = 0; i < iBufferSize; ++i) {', depth=2)
                line = 'pOutput[i] = pBuffer0[i]'
                for k in xrange(1, i):
                    line += ' + pBuffer%(k)d[i]' % {'k': k}
                line += ';'
                write(line, depth=3)
                write('}', depth=2)

        write('} else {', depth=1)
        write('//ScopedTimer t("EngineMaster::applyEffects%(inplace)sAndMixChannels_Over32active");' %
            {'inplace': 'InPlace' if inplace else ''}, depth=2)
        if inplace:
            write('SampleUtil::clear(pOutput, iBufferSize);', depth=2)
        write('for (int i = 0; i < activeChannels->size(); ++i) {', depth=2)
        write('EngineMaster::ChannelInfo* pChannelInfo = activeChannels->at(i);', depth=3)

        write('const int channelIndex = pChannelInfo->m_index;', depth=3)
        write('EngineMaster::GainCache& gainCache = (*channelGainCache)[channelIndex];', depth=3)
        write('CSAMPLE_GAIN oldGain = gainCache.m_gain;', depth=3)
        write('CSAMPLE_GAIN newGain;', depth=3)
        write('if (gainCache.m_fadeout) {', depth=3)
        write('newGain = 0;', depth=4)
        write('gainCache.m_fadeout = false;', depth=4)
        write('} else {', depth=3)
        write('newGain = gainCalculator.getGain(pChannelInfo);', depth=4)
        write('}', depth=3)
        write('gainCache.m_gain = newGain;', depth=3)

        write('CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;', depth=3)
        if inplace:
            write('pEngineEffectsManager->processPostFaderInPlace(pChannelInfo->m_handle, outputHandle, pBuffer, iBufferSize, iSampleRate, pChannelInfo->m_features, oldGain, newGain);' % {'j': j}, depth=3)
            write('SampleUtil::add(pOutput, pBuffer, iBufferSize);', depth=3)
        else:
            write('pEngineEffectsManager->processPostFaderAndMix(pChannelInfo->m_handle, outputHandle, pBuffer, pOutput, iBufferSize, iSampleRate, pChannelInfo->m_features, oldGain, newGain);' % {'j': j}, depth=3)

        write('}', depth=2)
        write('}', depth=1)
        output.append('}')

    write_applyeffectsandmixchannels(False, output)
    write_applyeffectsandmixchannels(True, output)

def write_sample_autogen(output, num_channels):
    output.append('#ifndef MIXXX_UTIL_SAMPLEAUTOGEN_H')
    output.append('#define MIXXX_UTIL_SAMPLEAUTOGEN_H')
    output.append('////////////////////////////////////////////////////////')
    output.append('// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //')
    output.append('// SEE scripts/generate_sample_functions.py           //')
    output.append('////////////////////////////////////////////////////////')

    for i in xrange(1, num_channels + 1):
        copy_with_gain(output, 0, i)
        copy_with_ramping_gain(output, 0, i)

    output.append('#endif /* MIXXX_UTIL_SAMPLEAUTOGEN_H */')

def copy_with_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0):
        output.append(' ' * (BASIC_INDENT * (depth + base_indent_depth)) + data)

    header = "static inline void %s(" % copy_with_gain_method_name(num_channels)
    arg_groups = ['CSAMPLE* M_RESTRICT pDest'] + [
        "const CSAMPLE* M_RESTRICT pSrc%(i)d, CSAMPLE_GAIN gain%(i)d" % {'i': i}
        for i in xrange(num_channels)] + ['int iNumSamples']

    output.extend(hanging_indent(header, arg_groups, ',', ') {',
                                 depth=base_indent_depth))


    for i in xrange(num_channels):
        write('if (gain%(i)d == CSAMPLE_GAIN_ZERO) {' % {'i': i}, depth=1)
        if (num_channels > 1):
            args = ['pDest',] + ['pSrc%(i)d, gain%(i)d' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
            write('%s;' %method_call(copy_with_gain_method_name(num_channels - 1), args), depth=2)
        else:
            write('clear(pDest, iNumSamples);', depth=2)
        write('return;', depth=2)
        write('}', depth=1)

    write('// note: LOOP VECTORIZED.', depth=1)
    write('for (int i = 0; i < iNumSamples; ++i) {', depth=1)
    terms = ['pSrc%(i)d[i] * gain%(i)d' % {'i': i} for i in xrange(num_channels)]
    assign = 'pDest[i] = '
    output.extend(hanging_indent(assign, terms, ' +', ';', depth=2))

    write('}', depth=1)
    write('}')


def copy_with_ramping_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0):
        output.append(' ' * (BASIC_INDENT * (depth + base_indent_depth)) + data)

    header = "static inline void %s(" % copy_with_ramping_gain_method_name(num_channels)
    arg_groups = ['CSAMPLE* M_RESTRICT pDest'] + [
        "const CSAMPLE* M_RESTRICT pSrc%(i)d, CSAMPLE_GAIN gain%(i)din, CSAMPLE_GAIN gain%(i)dout" % {'i': i}
        for i in xrange(num_channels)] + ['int iNumSamples']

    output.extend(hanging_indent(header, arg_groups, ',', ') {',
                                 depth=base_indent_depth))


    for i in xrange(num_channels):
        write('if (gain%(i)din == CSAMPLE_GAIN_ZERO && gain%(i)dout == CSAMPLE_GAIN_ZERO) {' % {'i': i}, depth=1)
        if (num_channels > 1):
            args = ['pDest',] + ['pSrc%(i)d, gain%(i)din, gain%(i)dout' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
            write('%s;' % method_call(copy_with_ramping_gain_method_name(num_channels - 1), args),
                  depth=2)
        else:
           write('clear(pDest, iNumSamples);', depth=2)
        write('return;', depth=2)
        write('}', depth=1)

    for i in xrange(num_channels):
        write('const CSAMPLE_GAIN gain_delta%(i)d = (gain%(i)dout - gain%(i)din) / (iNumSamples / 2);' % {'i': i}, depth=1)
        write('const CSAMPLE_GAIN start_gain%(i)d = gain%(i)din + gain_delta%(i)d;' % {'i': i}, depth=1)

    write('// note: LOOP VECTORIZED.', depth=1)
    write('for (int i = 0; i < iNumSamples / 2; ++i) {', depth=1)

    increments = ['const CSAMPLE_GAIN gain%(i)d = start_gain%(i)d + gain_delta%(i)d * i;' % {'i': i} for i in xrange(num_channels)]
    for i in xrange(num_channels):
        write('const CSAMPLE_GAIN gain%(i)d = start_gain%(i)d + gain_delta%(i)d * i;' % {'i': i}, depth=2)

    terms1 = []
    terms2 = []
    for i in xrange(num_channels):
        terms1.append('pSrc%(i)d[i * 2] * gain%(i)d' % {'i': i})
        terms2.append('pSrc%(i)d[i * 2 + 1] * gain%(i)d' % {'i': i})

    assign1 = 'pDest[i * 2] = '
    assign2 = 'pDest[i * 2 + 1] = '

    output.extend(hanging_indent(assign1, terms1, ' +', ';', depth=2))
    output.extend(hanging_indent(assign2, terms2, ' +', ';', depth=2))

    write('}', depth=1)
    write('}')

def main(args):
    sampleutil_output_lines = []
    write_sample_autogen(sampleutil_output_lines, args.max_channels)

    output = (open(args.sample_autogen_h, 'w')
              if args.sample_autogen_h else sys.stdout)
    output.write('\n'.join(sampleutil_output_lines) + '\n')

    channelmixer_output_lines = []
    write_channelmixer_autogen(channelmixer_output_lines, args.max_channels)

    output = (open(args.channelmixer_autogen_cpp, 'w')
              if args.channelmixer_autogen_cpp else sys.stdout)
    output.write('\n'.join(channelmixer_output_lines) + '\n')



if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Auto-generate sample processing and mixing functions.' +
        'Example Call:' +
        './generate_sample_functions.py --sample_autogen_h ../src/util/sample_autogen.h --channelmixer_autogen_cpp ../src/engine/channelmixer_autogen.cpp')
    parser.add_argument('--sample_autogen_h')
    parser.add_argument('--channelmixer_autogen_cpp')
    parser.add_argument('--max_channels', type=int, default=32)
    args = parser.parse_args()
    main(args)
