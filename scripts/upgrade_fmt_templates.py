import argparse
import json
import os
from typing import Any, Dict, List


replacements = {
    "macro_px_w": "subsampling_mode.macro_px_w",
    "macro_px_h": "subsampling_mode.macro_px_h",
    "plane[": "planes["
}

def perform_replacements(value: str) -> str:
    for src, replacement, in replacements:
        value = value.replace(src, replacement)
    return value

def fix_planes(planes: List[Dict[str, Any]]) -> None:
    for plane in planes:
        for key, value in plane:
            if type(value) == str:
                plane[key] = perform_replacements(value)

def fix_format(format: str) -> None:
    with open(format, 'r') as f:
        fmt_dict = json.load(f)

        if "planes" in fmt_dict:
            fix_planes(fmt_dict["planes"])


def main(argv: List[str]) -> None:
    parser = argparse.ArgumentParser(description="Update the format templates to the version required for xyuv 2.x.")
    parser.add_argument("px_fmt_folder", nargs=1, help="Path to a px_fmt folder containing format templates to update.")

    args = parser.parse_args(argv[1:])

    for files in os.listdir(args.px_fmt_folder):

