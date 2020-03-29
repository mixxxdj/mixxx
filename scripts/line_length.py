#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
from typing import Optional
from typing import Sequence
from subprocess import call


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("filenames", nargs="*", help="Filenames to check")
    args = parser.parse_args(argv)

    for filename in args.filenames:
        with open(filename) as fd:
            for lineno, line in enumerate(fd):
                if len(line) > 100:
                    continue
                humanlineno = lineno + 1
                print(f"{filename}:{humanlineno} Line is too long.")
                call(
                    [
                        "clang-format",
                        "-i",
                        f"-lines={humanlineno}:{humanlineno}",
                        "-style={"
                        "BasedOnStyle: Google, "
                        "IndentWidth: 4, "
                        "TabWidth: 8, "
                        "UseTab: Never, "
                        "ColumnLimit: 80, "
                        # clang-format normally unbreaks all short lines,
                        # which is undesired.
                        # This does not happen here because line is too long
                        "AccessModifierOffset: -2, "
                        "AlignAfterOpenBracket: DontAlign, "
                        "AlignOperands: false, "
                        "AllowShortFunctionsOnASingleLine: None, "
                        "AllowShortIfStatementsOnASingleLine: false, "
                        "AllowShortLoopsOnASingleLine: false, "
                        "BinPackArguments: false, "
                        "BinPackParameters: false, "
                        "ConstructorInitializerIndentWidth: 8, "
                        "ContinuationIndentWidth: 8, "
                        "IndentCaseLabels: false, "
                        "DerivePointerAlignment: false, "
                        "ReflowComments: false, "
                        "SpaceAfterTemplateKeyword: false, "
                        "SpacesBeforeTrailingComments: 1}",
                        f"{filename}",
                    ]
                )
    return 0


if __name__ == "__main__":
    exit(main())
