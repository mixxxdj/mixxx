#!/usr/bin/env python
import sys
import argparse

BASIC_INDENT = 4

def indent_separator(depth, offset):
    indent = BASIC_INDENT * depth + len(offset)
    return ' +\n' + ' ' * indent

COPY_WITH_GAIN_METHOD_PATTERN = 'copy%(i)dWithGain'
def copy_with_gain_method_name(i):
    return COPY_WITH_GAIN_METHOD_PATTERN % {'i' : i}

RAMPING_GAIN_METHOD_PATTERN = 'copy%(i)dWithRampingGain'
def copy_with_ramping_gain_method_name(i):
    return RAMPING_GAIN_METHOD_PATTERN % {'i': i}

def method_call(method_name, args):
    return '%(method_name)s(%(args)s)' % {'method_name': method_name,
                                          'args': ', '.join(args)}

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

    header = 'void ChannelMixer::mixChannels('
    args = ['const QList<EngineMaster::ChannelInfo*>& channels',
            'const EngineMaster::GainCalculator& gainCalculator',
            'unsigned int channelBitvector',
            'unsigned int maxChannels',
            'QList<CSAMPLE>* channelGainCache',
            'CSAMPLE* pOutput',
            'unsigned int iBufferSize']

    separator = ',\n' + ' ' * len(header)
    output.append(header + separator.join(args) + ') {')

    def write(data, depth=0, indent=0):
        output.append(' ' * (BASIC_INDENT * depth + indent) + data)

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
    write('SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);', depth=2)
    for i in xrange(1, num_channels+1):
        write('} else if (totalActive == %d) {' % i, depth=1)
        write('ScopedTimer t("EngineMaster::mixChannels_%(i)dactive");' % {'i': i}, depth=2)
        for j in xrange(i):
            write('const int pChannelIndex%(j)d = activeChannels[%(j)d];' % {'j': j}, depth=2)
            write('EngineMaster::ChannelInfo* pChannel%(j)d = channels[pChannelIndex%(j)d];' % {'j': j}, depth=2)
            write('CSAMPLE oldGain%(j)d = (*channelGainCache)[pChannelIndex%(j)d];' % {'j': j}, depth=2)
            write('CSAMPLE newGain%(j)d = gainCalculator.getGain(pChannel%(j)d);' % {'j': j}, depth=2)
            write('(*channelGainCache)[pChannelIndex%(j)d] = newGain%(j)d;' % {'j': j}, depth=2)
            write('CSAMPLE* pBuffer%(j)d = pChannel%(j)d->m_pBuffer;' % {'j': j}, depth=2)

        arg_groups = ['pOutput'] + ['pBuffer%(j)d, oldGain%(j)d, newGain%(j)d' % {'j': j} for j in xrange(i)] + ['iBufferSize']
        call_prefix = "SampleUtil::" + copy_with_ramping_gain_method_name(i) + '('
        separator = ',\n' + len(call_prefix) * ' ' + ' ' * BASIC_INDENT * 2
        write(call_prefix + separator.join(arg_groups) + ');', depth=2)

    write('} else {', depth=1)
    write('// Set pOutput to all 0s', depth=2)
    write('SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);', depth=2)
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
    def write(data, depth=0, indent=0):
        output.append(' ' * (BASIC_INDENT * (depth + base_indent_depth) + indent) + data)

    header = "static inline void %s(" % copy_with_gain_method_name(num_channels)
    dest_arg = "CSAMPLE* pDest,"
    write(header + dest_arg)

    for i in xrange(num_channels):
        write("const CSAMPLE* pSrc%(i)d, CSAMPLE gain%(i)d," % {'i': i}, indent=len(header))
    write('int iNumSamples) {', indent=len(header))

    if (num_channels == 1):
        write('copyWithGain(pDest, pSrc0, gain0, iNumSamples);', depth=1)
        write('return;', depth=1)
        write('}')
        return

    for i in xrange(num_channels):
        write('if (gain%(i)d == 0.0) {' % {'i': i}, depth=1)
        args = ['pDest',] + ['pSrc%(i)d, gain%(i)d' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
        write('%s;' %method_call(copy_with_gain_method_name(num_channels - 1), args), depth=2)
        write('return;', depth=2)
        write('}', depth=1)

    write('for (int i = 0; i < iNumSamples; ++i) {', depth=1)
    terms = ['pSrc%(i)d[i] * gain%(i)d' % {'i': i} for i in xrange(num_channels)]
    assign = 'pDest[i] = '
    write(assign + indent_separator(base_indent_depth + 2, assign).join(terms) + ';', depth=2)
    write('}', depth=1)
    write('}')


def copy_with_ramping_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0, indent=0):
        output.append(' ' * (BASIC_INDENT * (depth + base_indent_depth) + indent) + data)

    header = "static inline void %s(" % copy_with_ramping_gain_method_name(num_channels)
    dest_arg = "CSAMPLE* pDest,"
    write(header + dest_arg)
    for i in range(num_channels):
        write("const CSAMPLE* pSrc%(i)d, CSAMPLE gain%(i)din, CSAMPLE gain%(i)dout," % {'i': i},
              indent=len(header))
    write('int iNumSamples) {', indent=len(header))

    if (num_channels == 1):
        write('copyWithRampingGain(pDest, pSrc0, gain0in, gain0out, iNumSamples);', depth=1)
        write('return;', depth=1)
        write('}')
        return


    for i in range(num_channels):
        write('if (gain%(i)din == 0.0 && gain%(i)dout == 0.0) {' % {'i': i}, depth=1)
        args = ['pDest',] + ['pSrc%(i)d, gain%(i)din, gain%(i)dout' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
        write('%s;' % method_call(copy_with_ramping_gain_method_name(num_channels - 1), args),
              depth=2)
        write('return;', depth=2)
        write('}', depth=1)

    for i in range(num_channels):
        write('const CSAMPLE delta%(i)d = 2.0 * (gain%(i)dout - gain%(i)din) / iNumSamples;' % {'i': i}, depth=1)
        write('CSAMPLE gain%(i)d = gain%(i)din;' % {'i': i}, depth=1)

    increments = ['i += 2',] + ['gain%(i)d += delta%(i)d' % {'i': i} for i in xrange(num_channels)]

    write('for (int i = 0; i < iNumSamples; %s) {' % ', '.join(increments), depth=1)

    dest1 = []
    dest2 = []
    for i in range(num_channels):
        dest1.append('pSrc%(i)d[i] * gain%(i)d' % {'i': i})
        dest2.append('pSrc%(i)d[i + 1] * gain%(i)d' % {'i': i})

    assign1 = 'pDest[i] = '
    assign2 = 'pDest[i + 1] = '

    write(assign1 + indent_separator(base_indent_depth + 2, assign1).join(dest1) + ';', depth=2)
    write(assign2 + indent_separator(base_indent_depth + 2, assign2).join(dest2) + ';', depth=2)
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
