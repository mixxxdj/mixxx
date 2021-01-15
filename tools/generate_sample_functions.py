#!/usr/bin/env python
import argparse
import sys

# To use, run this from the top level of the Git repository tree:
# scripts/generate_sample_functions.py
#     --sample_autogen_h src/util/sample_autogen.h

BASIC_INDENT = 4

COPY_WITH_GAIN_METHOD_PATTERN = "copy%(i)dWithGain"


def copy_with_gain_method_name(i):
    return COPY_WITH_GAIN_METHOD_PATTERN % {"i": i}


RAMPING_GAIN_METHOD_PATTERN = "copy%(i)dWithRampingGain"


def copy_with_ramping_gain_method_name(i):
    return RAMPING_GAIN_METHOD_PATTERN % {"i": i}


def method_call(method_name, args):
    return "%(method_name)s(%(args)s)" % {
        "method_name": method_name,
        "args": ", ".join(args),
    }


def hanging_indent(base, groups, hanging_suffix, terminator, depth=0):
    return map(
        lambda line: " " * BASIC_INDENT * depth + line,
        map(
            "".join,
            zip(
                [base] + [" " * len(base)] * (len(groups) - 1),
                groups,
                [hanging_suffix] * (len(groups) - 1) + [terminator],
            ),
        ),
    )


def write_sample_autogen(output, num_channels):
    output.append("#ifndef MIXXX_UTIL_SAMPLEAUTOGEN_H")
    output.append("#define MIXXX_UTIL_SAMPLEAUTOGEN_H")
    output.append("////////////////////////////////////////////////////////")
    output.append("// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //")
    output.append("// SEE scripts/generate_sample_functions.py           //")
    output.append("////////////////////////////////////////////////////////")

    for i in range(1, num_channels + 1):
        copy_with_gain(output, 0, i)
        copy_with_ramping_gain(output, 0, i)

    output.append("#endif /* MIXXX_UTIL_SAMPLEAUTOGEN_H */")


def copy_with_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0):
        output.append(
            " " * (BASIC_INDENT * (depth + base_indent_depth)) + data
        )

    header = "static inline void %s(" % copy_with_gain_method_name(
        num_channels
    )
    arg_groups = (
        ["CSAMPLE* M_RESTRICT pDest"]
        + [
            "const CSAMPLE* M_RESTRICT pSrc%(i)d, CSAMPLE_GAIN gain%(i)d"
            % {"i": i}
            for i in range(num_channels)
        ]
        + ["int iNumSamples"]
    )

    output.extend(
        hanging_indent(header, arg_groups, ",", ") {", depth=base_indent_depth)
    )

    for i in range(num_channels):
        write("if (gain%(i)d == CSAMPLE_GAIN_ZERO) {" % {"i": i}, depth=1)
        if num_channels > 1:
            args = (
                ["pDest"]
                + [
                    "pSrc%(i)d, gain%(i)d" % {"i": j}
                    for j in range(num_channels)
                    if i != j
                ]
                + ["iNumSamples"]
            )
            write(
                "%s;"
                % method_call(
                    copy_with_gain_method_name(num_channels - 1), args
                ),
                depth=2,
            )
        else:
            write("clear(pDest, iNumSamples);", depth=2)
        write("return;", depth=2)
        write("}", depth=1)

    write("// note: LOOP VECTORIZED.", depth=1)
    write("for (int i = 0; i < iNumSamples; ++i) {", depth=1)
    terms = [
        "pSrc%(i)d[i] * gain%(i)d" % {"i": i} for i in range(num_channels)
    ]
    assign = "pDest[i] = "
    output.extend(hanging_indent(assign, terms, " +", ";", depth=2))

    write("}", depth=1)
    write("}")


def copy_with_ramping_gain(output, base_indent_depth, num_channels):
    def write(data, depth=0):
        output.append(
            " " * (BASIC_INDENT * (depth + base_indent_depth)) + data
        )

    header = "static inline void %s(" % copy_with_ramping_gain_method_name(
        num_channels
    )
    arg_groups = (
        ["CSAMPLE* M_RESTRICT pDest"]
        + [
            (
                "const CSAMPLE* M_RESTRICT pSrc%(i)d, "
                "CSAMPLE_GAIN gain%(i)din, CSAMPLE_GAIN gain%(i)dout"
            )
            % {"i": i}
            for i in range(num_channels)
        ]
        + ["int iNumSamples"]
    )

    output.extend(
        hanging_indent(header, arg_groups, ",", ") {", depth=base_indent_depth)
    )

    for i in range(num_channels):
        write(
            (
                "if (gain%(i)din == CSAMPLE_GAIN_ZERO && "
                "gain%(i)dout == CSAMPLE_GAIN_ZERO) {"
            )
            % {"i": i},
            depth=1,
        )
        if num_channels > 1:
            args = (
                ["pDest"]
                + [
                    "pSrc%(i)d, gain%(i)din, gain%(i)dout" % {"i": j}
                    for j in range(num_channels)
                    if i != j
                ]
                + ["iNumSamples"]
            )
            write(
                "%s;"
                % method_call(
                    copy_with_ramping_gain_method_name(num_channels - 1), args
                ),
                depth=2,
            )
        else:
            write("clear(pDest, iNumSamples);", depth=2)
        write("return;", depth=2)
        write("}", depth=1)

    for i in range(num_channels):
        write(
            (
                "const CSAMPLE_GAIN gain_delta%(i)d = "
                "(gain%(i)dout - gain%(i)din) / (iNumSamples / 2);"
            )
            % {"i": i},
            depth=1,
        )
        write(
            (
                "const CSAMPLE_GAIN start_gain%(i)d = "
                "gain%(i)din + gain_delta%(i)d;"
            )
            % {"i": i},
            depth=1,
        )

    write("// note: LOOP VECTORIZED.", depth=1)
    write("for (int i = 0; i < iNumSamples / 2; ++i) {", depth=1)

    for i in range(num_channels):
        write(
            (
                "const CSAMPLE_GAIN gain%(i)d = "
                "start_gain%(i)d + gain_delta%(i)d * i;"
            )
            % {"i": i},
            depth=2,
        )

    terms1 = []
    terms2 = []
    for i in range(num_channels):
        terms1.append("pSrc%(i)d[i * 2] * gain%(i)d" % {"i": i})
        terms2.append("pSrc%(i)d[i * 2 + 1] * gain%(i)d" % {"i": i})

    assign1 = "pDest[i * 2] = "
    assign2 = "pDest[i * 2 + 1] = "

    output.extend(hanging_indent(assign1, terms1, " +", ";", depth=2))
    output.extend(hanging_indent(assign2, terms2, " +", ";", depth=2))

    write("}", depth=1)
    write("}")


def main(args):
    sampleutil_output_lines = []
    write_sample_autogen(sampleutil_output_lines, args.max_channels)

    output = (
        open(args.sample_autogen_h, "w")
        if args.sample_autogen_h
        else sys.stdout
    )
    output.write("\n".join(sampleutil_output_lines) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Auto-generate sample processing and mixing functions.",
        epilog=(
            "Example Call:"
            "./generate_sample_functions.py --sample_autogen_h "
        ),
    )
    parser.add_argument("--sample_autogen_h")
    parser.add_argument("--max_channels", type=int, default=32)
    args = parser.parse_args()
    main(args)
