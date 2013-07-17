#!/usr/bin/env python
import sys
import argparse

BASIC_INDENT = 4
MAX_CHANNELS = 32

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
    output.write('#include "engine/channelmixer.h"\n')
    output.write('#include "util/timer.h"\n')
    output.write('#include "sampleutil.h"\n')
    output.write('////////////////////////////////////////////////////////\n')
    output.write('// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //\n')
    output.write('////////////////////////////////////////////////////////\n')
    output.write('\n')
    output.write('// static\n')

    header = 'void ChannelMixer::mixChannels('
    args = ['const QList<EngineMaster::ChannelInfo*>& channels',
            'const EngineMaster::GainCalculator& gainCalculator',
            'unsigned int channelBitvector',
            'unsigned int maxChannels',
            'QList<CSAMPLE>* channelGainCache',
            'CSAMPLE* pOutput',
            'unsigned int iBufferSize']

    separator = ',\n' + ' ' * len(header)
    output.write(header + separator.join(args) + ') { \n')

    def write(data, depth=0, indent=0):
        output.write(' ' * (BASIC_INDENT * depth + indent) + data)

    write('int activeChannels[%d] = {\n' % num_channels, depth=1)
    for i in xrange(num_channels):
        if i == num_channels - 1:
            write('-1};\n', depth=2)
        else:
            write('-1,\n', depth=2)

    write('unsigned int totalActive = 0;\n', depth=1)
    write('for (unsigned int i = 0; i < maxChannels; ++i) {\n', depth=1)
    write('if ((channelBitvector & (1 << i)) == 0) {\n', depth=2)
    write('continue;\n', depth=3)
    write('}\n', depth=2)
    write('if (totalActive < %d) {\n' % num_channels, depth=2)
    write('activeChannels[totalActive] = i;\n', depth=3)
    write('}\n', depth=2)
    write('++totalActive;\n', depth=2)
    write('}\n', depth=1)

    write('if (totalActive == 0) {\n', depth=1)
    write('ScopedTimer t("EngineMaster::mixChannels_0active");\n', depth=2)
    write('SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);\n', depth=2)
    for i in xrange(1, num_channels+1):
        write('} else if (totalActive == %d) {\n' % i, depth=1)
        write('ScopedTimer t("EngineMaster::mixChannels_%(i)dactive");\n' % {'i': i}, depth=2)
        for j in xrange(i):
            write('const int pChannelIndex%(j)d = activeChannels[%(j)d];\n' % {'j': j}, depth=2)
            write('EngineMaster::ChannelInfo* pChannel%(j)d = channels[pChannelIndex%(j)d];\n' % {'j': j}, depth=2)
            write('CSAMPLE oldGain%(j)d = (*channelGainCache)[pChannelIndex%(j)d];\n' % {'j': j}, depth=2)
            write('CSAMPLE newGain%(j)d = gainCalculator.getGain(pChannel%(j)d);\n' % {'j': j}, depth=2)
            write('(*channelGainCache)[pChannelIndex%(j)d] = newGain%(j)d;\n' % {'j': j}, depth=2)
            write('CSAMPLE* pBuffer%(j)d = pChannel%(j)d->m_pBuffer;\n' % {'j': j}, depth=2)

        arg_groups = ['pOutput'] + ['pBuffer%(j)d, oldGain%(j)d, newGain%(j)d' % {'j': j} for j in xrange(i)] + ['iBufferSize']
        call_prefix = "SampleUtil::" + copy_with_ramping_gain_method_name(i) + '('
        separator = ',\n' + len(call_prefix) * ' ' + ' ' * BASIC_INDENT * 2
        write(call_prefix + separator.join(arg_groups) + ');\n', depth=2)

    write('} else {\n', depth=1)
    write('// Set pOutput to all 0s\n', depth=2)
    write('SampleUtil::applyGain(pOutput, 0.0f, iBufferSize);\n', depth=2)
    write('for (unsigned int i = 0; i < maxChannels; ++i) {\n', depth=2)
    write('if (channelBitvector & (1 << i)) {\n', depth=3)
    write('EngineMaster::ChannelInfo* pChannelInfo = channels[i];\n', depth=4)
    write('CSAMPLE* pBuffer = pChannelInfo->m_pBuffer;\n', depth=4)
    write('CSAMPLE gain = gainCalculator.getGain(pChannelInfo);\n', depth=4)
    write('SampleUtil::addWithGain(pOutput, pBuffer, gain, iBufferSize);\n', depth=4)
    write('}\n', depth=3)
    write('}\n', depth=2)
    write('}\n', depth=1)
    output.write('}\n')

def write_sampleutil_autogen(output, num_channels):
    output.write('#ifndef SAMPLEUTILAUTOGEN_H\n')
    output.write('#define SAMPLEUTILAUTOGEN_H\n')
    output.write('////////////////////////////////////////////////////////\n')
    output.write('// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //\n')
    output.write('////////////////////////////////////////////////////////\n')

    for i in xrange(1, num_channels + 1):
        copy_with_gain(output, 0, i)
        copy_with_ramping_gain(output, 0, i)

    output.write('#endif /* SAMPLEUTILAUTOGEN_H */\n')

def copy_with_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0, indent=0):
        output.write(' ' * (BASIC_INDENT * (depth + base_indent_depth) + indent) + data)

    header = "static inline void %s(" % copy_with_gain_method_name(num_channels)
    dest_arg = "CSAMPLE* pDest,"
    write(header + dest_arg + '\n')

    for i in xrange(num_channels):
        write("const CSAMPLE* pSrc%(i)d, CSAMPLE gain%(i)d,\n" % {'i': i}, indent=len(header))
    write('int iNumSamples) {\n', indent=len(header))

    if (num_channels == 1):
        write('copyWithGain(pDest, pSrc0, gain0, iNumSamples);\n', depth=1)
        write('return;\n', depth=1)
        write('}\n')
        return

    for i in xrange(num_channels):
        write('if (gain%(i)d == 0.0) {\n' % {'i': i}, depth=1)
        args = ['pDest',] + ['pSrc%(i)d, gain%(i)d' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
        write('%s;\n' %method_call(copy_with_gain_method_name(num_channels - 1), args), depth=2)
        write('return;\n', depth=2)
        write('}\n', depth=1)

    write('for (int i = 0; i < iNumSamples; ++i) {\n', depth=1)
    terms = ['pSrc%(i)d[i] * gain%(i)d' % {'i': i} for i in xrange(num_channels)]
    assign = 'pDest[i] = '
    write(assign + indent_separator(base_indent_depth + 2, assign).join(terms) + ';\n', depth=2)
    write('}\n', depth=1)
    write('}\n')


def copy_with_ramping_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0, indent=0):
        output.write(' ' * (BASIC_INDENT * (depth + base_indent_depth) + indent) + data)

    header = "static inline void %s(" % copy_with_ramping_gain_method_name(num_channels)
    dest_arg = "CSAMPLE* pDest,"
    write(header + dest_arg + '\n')
    for i in range(num_channels):
        write("const CSAMPLE* pSrc%(i)d, CSAMPLE gain%(i)din, CSAMPLE gain%(i)dout,\n" % {'i': i},
              indent=len(header))
    write('int iNumSamples) {\n', indent=len(header))

    if (num_channels == 1):
        write('copyWithRampingGain(pDest, pSrc0, gain0in, gain0out, iNumSamples);\n', depth=1)
        write('return;\n', depth=1)
        write('}\n')
        return


    for i in range(num_channels):
        write('if (gain%(i)din == 0.0 && gain%(i)dout == 0.0) {\n' % {'i': i}, depth=1)
        args = ['pDest',] + ['pSrc%(i)d, gain%(i)din, gain%(i)dout' % {'i': j} for j in xrange(num_channels) if i != j] + ['iNumSamples',]
        write('%s;\n' % method_call(copy_with_ramping_gain_method_name(num_channels - 1), args),
              depth=2)
        write('return;\n', depth=2)
        write('}\n', depth=1)

    for i in range(num_channels):
        write('const CSAMPLE delta%(i)d = 2.0 * (gain%(i)dout - gain%(i)din) / iNumSamples;\n' % {'i': i}, depth=1)
        write('CSAMPLE gain%(i)d = gain%(i)din;\n' % {'i': i}, depth=1)

    write('for (int i = 0; i < iNumSamples; i += 2', depth=1)
    for i in range(num_channels):
        output.write(', gain%(i)d += delta%(i)d' % {'i': i})
    output.write(') {\n')

    dest1 = []
    dest2 = []
    for i in range(num_channels):
        dest1.append('pSrc%(i)d[i] * gain%(i)d' % {'i': i})
        dest2.append('pSrc%(i)d[i + 1] * gain%(i)d' % {'i': i})

    assign1 = 'pDest[i] = '
    assign2 = 'pDest[i + 1] = '

    write(assign1 + indent_separator(base_indent_depth + 2, assign1).join(dest1) + ';\n', depth=2)
    write(assign2 + indent_separator(base_indent_depth + 2, assign2).join(dest2) + ';\n', depth=2)
    write('}\n', depth=1)
    write('}\n')

def main(args):
    output = sys.stdout
    if args.sampleutil_autogen_h:
        output = open(args.sampleutil_autogen_h, 'w')
    write_sampleutil_autogen(output, MAX_CHANNELS)

    output = sys.stdout
    if args.channelmixer_autogen_cpp:
        output = open(args.channelmixer_autogen_cpp, 'w')
    write_channelmixer_autogen(output, MAX_CHANNELS)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Auto-generate sample processing and mixing functions.')
    parser.add_argument('--sampleutil_autogen_h')
    parser.add_argument('--channelmixer_autogen_cpp')
    args = parser.parse_args()
    main(args)
