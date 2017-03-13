#!/usr/bin/env python
import argparse
import sys

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
    output.append('#include "util/timer.h"')
    output.append('#include "sampleutil.h"')
    output.append('////////////////////////////////////////////////////////')
    output.append('// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //')
    output.append('// SEE scripts/generate_sample_functions.py           //')
    output.append('////////////////////////////////////////////////////////')
    output.append('')
    output.append('// static')

    def write_mixchannels(ramping, output):
        if ramping:
            header = 'void ChannelMixer::mixChannelsRamping('
        else:
            header = 'void ChannelMixer::mixChannels('
        args = ['const QList<EngineMaster::ChannelInfo*>& channels',
                'const EngineMaster::GainCalculator& gainCalculator',
                'unsigned int channelBitvector',
                'unsigned int maxChannels',
                'QList<CSAMPLE>* channelGainCache',
                'CSAMPLE* pOutput',
                'unsigned int iBufferSize']
        output.extend(hanging_indent(header, args, ',', ') {'))

        def write(data, depth=0):
            output.append(' ' * (BASIC_INDENT * depth) + data)

        write('int activeChannels[%d] = {' % num_channels, depth=1)
        for i in xrange(num_channels):
            if i == num_channels - 1:
                write('-1};', depth=2)
            else:
                write('-1,', depth=2)

        write('unsigned int totalActive = 0;', depth=1)
        write('for (unsigned int i = 0; i < maxChannels; ++i) {', depth=1)
        write('if ((channelBitvector & (1 << i)) == 0) {', depth=2)
        write('continue;', depth=3)
        write('}', depth=2)
        write('if (totalActive < %d) {' % num_channels, depth=2)
        write('activeChannels[totalActive] = i;', depth=3)
        write('}', depth=2)
        write('++totalActive;', depth=2)
        write('}', depth=1)

        write('if (totalActive == 0) {', depth=1)
        write('ScopedTimer t("EngineMaster::mixChannels_0active");', depth=2)
        write('SampleUtil::clear(pOutput, iBufferSize);', depth=2)
        for i in xrange(1, num_channels+1):
            write('} else if (totalActive == %d) {' % i, depth=1)
            write('ScopedTimer t("EngineMaster::mixChannels_%(i)dactive");' % {'i': i}, depth=2)
            for j in xrange(i):
                write('const int pChannelIndex%(j)d = activeChannels[%(j)d];' % {'j': j}, depth=2)
                write('EngineMaster::ChannelInfo* pChannel%(j)d = channels[pChannelIndex%(j)d];' % {'j': j}, depth=2)
                if ramping:
                    write('CSAMPLE_GAIN oldGain%(j)d = (*channelGainCache)[pChannelIndex%(j)d];' % {'j': j}, depth=2)
                write('CSAMPLE_GAIN newGain%(j)d = gainCalculator.getGain(pChannel%(j)d);' % {'j': j}, depth=2)
                write('(*channelGainCache)[pChannelIndex%(j)d] = newGain%(j)d;' % {'j': j}, depth=2)
                write('CSAMPLE* pBuffer%(j)d = pChannel%(j)d->m_pBuffer;' % {'j': j}, depth=2)

            if ramping:
                arg_groups = ['pOutput'] + ['pBuffer%(j)d, oldGain%(j)d, newGain%(j)d' % {'j': j} for j in xrange(i)] + ['iBufferSize']
                call_prefix = "SampleUtil::" + copy_with_ramping_gain_method_name(i) + '('
            else:
                arg_groups = ['pOutput'] + ['pBuffer%(j)d, newGain%(j)d' % {'j': j} for j in xrange(i)] + ['iBufferSize']
                call_prefix = "SampleUtil::" + copy_with_gain_method_name(i) + '('
            output.extend(hanging_indent(call_prefix, arg_groups, ',', ');', depth=2))


        write('} else {', depth=1)
        write('// Set pOutput to all 0s', depth=2)
        write('SampleUtil::clear(pOutput, iBufferSize);', depth=2)
        write('for (unsigned int i = 0; i < maxChannels; ++i) {', depth=2)
        write('if (channelBitvector & (1 << i)) {', depth=3)
        write('EngineMaster::ChannelInfo* pChannelInfo = channels[i];', depth=4)
        write('CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;', depth=4)
        write('CSAMPLE gain = gainCalculator.getGain(pChannelInfo);', depth=4)
        write('SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);', depth=4)
        write('}', depth=3)
        write('}', depth=2)
        write('}', depth=1)
        output.append('}')
    write_mixchannels(False, output)
    write_mixchannels(True, output)

def write_sampleutil_autogen(output, num_channels):
    output.append('#ifndef SAMPLEUTILAUTOGEN_H')
    output.append('#define SAMPLEUTILAUTOGEN_H')
    output.append('////////////////////////////////////////////////////////')
    output.append('// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //')
    output.append('// SEE scripts/generate_sample_functions.py           //')
    output.append('////////////////////////////////////////////////////////')

    for i in xrange(1, num_channels + 1):
        copy_with_gain(output, 0, i)
        copy_with_ramping_gain(output, 0, i)

    output.append('#endif /* SAMPLEUTILAUTOGEN_H */')

def copy_with_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0):
        output.append(' ' * (BASIC_INDENT * (depth + base_indent_depth)) + data)

    header = "static inline void %s(" % copy_with_gain_method_name(num_channels)
    arg_groups = ['CSAMPLE* pDest'] + [
        "const CSAMPLE* pSrc%(i)d, CSAMPLE_GAIN gain%(i)d" % {'i': i}
        for i in xrange(num_channels)] + ['unsigned int iNumSamples']

    output.extend(hanging_indent(header, arg_groups, ',', ') {',
                                 depth=base_indent_depth))

    if (num_channels == 1):
        write('copyWithGain(pDest, pSrc0, gain0, iNumSamples);', depth=1)
        write('return;', depth=1)
        write('}')
        return

    for i in xrange(num_channels):
        write('if (gain%(i)d == CSAMPLE_GAIN_ZERO) {' % {'i': i}, depth=1)
        args = ['pDest',] + ['pSrc%(i)d, gain%(i)d' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
        write('%s;' %method_call(copy_with_gain_method_name(num_channels - 1), args), depth=2)
        write('return;', depth=2)
        write('}', depth=1)

    write('for (unsigned int i = 0; i < iNumSamples; ++i) {', depth=1)
    terms = ['pSrc%(i)d[i] * gain%(i)d' % {'i': i} for i in xrange(num_channels)]
    assign = 'pDest[i] = '
    output.extend(hanging_indent(assign, terms, ' +', ';', depth=2))

    write('}', depth=1)
    write('}')


def copy_with_ramping_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0):
        output.append(' ' * (BASIC_INDENT * (depth + base_indent_depth)) + data)

    header = "static inline void %s(" % copy_with_ramping_gain_method_name(num_channels)
    arg_groups = ['CSAMPLE* pDest'] + [
        "const CSAMPLE* pSrc%(i)d, CSAMPLE_GAIN gain%(i)din, CSAMPLE_GAIN gain%(i)dout" % {'i': i}
        for i in xrange(num_channels)] + ['unsigned int iNumSamples']

    output.extend(hanging_indent(header, arg_groups, ',', ') {',
                                 depth=base_indent_depth))

    if (num_channels == 1):
        write('copyWithRampingGain(pDest, pSrc0, gain0in, gain0out, iNumSamples);', depth=1)
        write('return;', depth=1)
        write('}')
        return


    for i in xrange(num_channels):
        write('if (gain%(i)din == CSAMPLE_GAIN_ZERO && gain%(i)dout == CSAMPLE_GAIN_ZERO) {' % {'i': i}, depth=1)
        args = ['pDest',] + ['pSrc%(i)d, gain%(i)din, gain%(i)dout' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
        write('%s;' % method_call(copy_with_ramping_gain_method_name(num_channels - 1), args),
              depth=2)
        write('return;', depth=2)
        write('}', depth=1)

    for i in xrange(num_channels):
        write('const CSAMPLE_GAIN gain_delta%(i)d = (gain%(i)dout - gain%(i)din) / (iNumSamples / 2);' % {'i': i}, depth=1)
        write('CSAMPLE_GAIN gain%(i)d = gain%(i)din;' % {'i': i}, depth=1)

    increments = ['i += 2',] + ['gain%(i)d += gain_delta%(i)d' % {'i': i} for i in xrange(num_channels)]

    write('for (unsigned int i = 0; i < iNumSamples; %s) {' % ', '.join(increments), depth=1)

    terms1 = []
    terms2 = []
    for i in xrange(num_channels):
        terms1.append('pSrc%(i)d[i] * gain%(i)d' % {'i': i})
        terms2.append('pSrc%(i)d[i + 1] * gain%(i)d' % {'i': i})

    assign1 = 'pDest[i] = '
    assign2 = 'pDest[i + 1] = '

    output.extend(hanging_indent(assign1, terms1, ' +', ';', depth=2))
    output.extend(hanging_indent(assign2, terms2, ' +', ';', depth=2))

    write('}', depth=1)
    write('}')

def main(args):
    sampleutil_output_lines = []
    write_sampleutil_autogen(sampleutil_output_lines, args.max_channels)

    output = (open(args.sampleutil_autogen_h, 'w')
              if args.sampleutil_autogen_h else sys.stdout)
    output.write('\n'.join(sampleutil_output_lines) + '\n')

    channelmixer_output_lines = []
    write_channelmixer_autogen(channelmixer_output_lines, args.max_channels)

    output = (open(args.channelmixer_autogen_cpp, 'w')
              if args.channelmixer_autogen_cpp else sys.stdout)
    output.write('\n'.join(channelmixer_output_lines) + '\n')



if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Auto-generate sample processing and mixing functions.')
    parser.add_argument('--sampleutil_autogen_h')
    parser.add_argument('--channelmixer_autogen_cpp')
    parser.add_argument('--max_channels', type=int, default=32)
    args = parser.parse_args()
    main(args)
