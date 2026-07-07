#!/usr/bin/env python3
"""ffmpeg2xaf.py - Convert any video to XAF format via ffmpeg's Cinepak encoder.

Generates a XAF container file with Cinepak-encoded frames.
Encoding pipeline: input → ffmpeg (Cinepak) → temp AVI → parse RIFF → XAF.
All encoding is delegated to ffmpeg; no Cinepak encoding logic in this script.
Depends on: ffmpeg (command-line tool).
"""

import argparse
import os
import struct
import subprocess
import sys
import tempfile


def _read_le32(data, offset):
    if offset + 4 > len(data):
        return 0
    return struct.unpack_from("<I", data, offset)[0]


def _read_le16(data, offset):
    if offset + 2 > len(data):
        return 0
    return struct.unpack_from("<H", data, offset)[0]


def _parse_avi_riff(data):
    """Parse AVI RIFF data. Returns (width, height, fps, frame_list)."""
    if len(data) < 12:
        raise ValueError("File too small for RIFF header")
    if data[0:4] != b"RIFF":
        raise ValueError(f"Not a RIFF file: {data[0:4]!r}")
    riff_size = _read_le32(data, 4)
    if data[8:12] != b"AVI ":
        raise ValueError(f"Not an AVI file: {data[8:12]!r}")

    width = None
    height = None
    fps = 30
    frames = []

    # Stack: (start, end, list_type_or_None)
    # list_type is None for the root RIFF (after 'AVI ')
    stack = [(12, 12 + riff_size)]

    while stack:
        start, end = stack.pop()
        pos = start
        while pos + 8 <= end:
            ckid = data[pos : pos + 4]
            cksize = _read_le32(data, pos + 4)
            pos += 8
            data_start = pos

            if pos + cksize > end:
                break

            if ckid == b"LIST" and pos + 4 <= pos + cksize:
                list_type = data[pos : pos + 4]
                stack.append((pos + 4, pos + cksize))
                pos += cksize
            elif ckid == b"avih" and cksize >= 56:
                uspf = _read_le32(data, data_start)
                if uspf > 0:
                    fps = 1000000.0 / uspf
                w = _read_le32(data, data_start + 32)
                h = _read_le32(data, data_start + 36)
                if w and h:
                    width, height = w, h
                pos += cksize
            elif len(ckid) == 4 and ckid[2:4] == b"dc":
                frame_data = data[data_start : data_start + cksize]
                frames.append(frame_data)
                pos += cksize
            else:
                pos += cksize

            if cksize & 1:
                pos += 1

    return width, height, int(round(fps)), frames


def create_xaf(input_video, output_xaf, fps=None, width=None, height=None):
    """Create an XAF file from an input video using ffmpeg Cinepak encoder."""
    vf_parts = []
    if fps:
        vf_parts.append(f"fps={fps}")
    if width and height:
        vf_parts.append(f"scale={width}:{height}")
    elif width:
        vf_parts.append(f"scale={width}:-1")
    elif height:
        vf_parts.append(f"scale=-1:{height}")

    fd, temp_avi = tempfile.mkstemp(suffix=".avi")
    os.close(fd)

    cmd = ["ffmpeg", "-y", "-i", input_video]
    if vf_parts:
        cmd.extend(["-vf", ",".join(vf_parts)])
    cmd.extend(["-c:v", "cinepak", "-an", temp_avi])

    print(f"Encoding with ffmpeg...")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"ffmpeg error:\n{result.stderr}", file=sys.stderr)
        if os.path.exists(temp_avi):
            os.unlink(temp_avi)
        sys.exit(1)

    print("Parsing AVI and extracting Cinepak frames...")
    with open(temp_avi, "rb") as f:
        avi_data = f.read()
    os.unlink(temp_avi)

    avi_width, avi_height, avi_fps, frames = _parse_avi_riff(avi_data)

    if avi_width is None or avi_height is None:
        print("Error: could not determine video dimensions from AVI", file=sys.stderr)
        sys.exit(1)

    if not frames:
        print("Error: no video frames found in AVI", file=sys.stderr)
        sys.exit(1)

    print(
        f"Video: {avi_width}x{avi_height}, "
        f"{len(frames)} frames, {avi_fps} fps"
    )

    with open(output_xaf, "wb") as f:
        # 16-byte header, LE, packed
        f.write(b"XAF\x00")
        f.write(struct.pack("<H", avi_width))
        f.write(struct.pack("<H", avi_height))
        f.write(struct.pack("<I", len(frames)))
        f.write(struct.pack("<B", avi_fps))
        f.write(b"\x00\x00\x00")   # reserved

        for frame_data in frames:
            f.write(struct.pack("<I", len(frame_data)))
            f.write(frame_data)

    print(f"Done: {output_xaf} ({os.path.getsize(output_xaf)} bytes)")


def main():
    parser = argparse.ArgumentParser(
        description="Convert any video to XAF format using Cinepak encoding",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s video.mp4
  %(prog)s video.mp4 -o output.xaf -f 24 -w 320 -H 240
  %(prog)s video.mp4 -f 15
  %(prog)s video.mp4 -- all extra ffmpeg args""",
    )
    parser.add_argument("input", help="Input video file")
    parser.add_argument("-o", "--output", help="Output XAF file (default: <input>.xaf)")
    parser.add_argument("-f", "--fps", type=float, help="Frame rate")
    parser.add_argument("-w", "--width", type=int, help="Output width")
    parser.add_argument("-H", "--height", type=int, help="Output height")

    args, extra = parser.parse_known_args()

    if not os.path.isfile(args.input):
        print(f"Error: input file not found: {args.input}", file=sys.stderr)
        sys.exit(1)

    output = args.output
    if not output:
        base = os.path.splitext(args.input)[0]
        output = base + ".xaf"

    create_xaf(args.input, output, args.fps, args.width, args.height)


if __name__ == "__main__":
    main()
