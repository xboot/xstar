#!/usr/bin/env python3
"""pyav2xaf.py - Convert any video to XAF format via PyAV's Cinepak encoder.

Generates a XAF container file with Cinepak-encoded frames.
Encoding pipeline: decode with PyAV → scale/crop → encode with Cinepak → XAF.
Pure Python in-memory pipeline, no intermediate files, no ffmpeg subprocess.
Depends on: PyAV (`pip install av`).
"""

import argparse
import os
import struct
import sys
import av
from fractions import Fraction


def create_xaf(input_video, output_xaf, fps=None, width=None, height=None):
    input_container = av.open(input_video)
    in_stream = input_container.streams.video[0]

    in_width = in_stream.width
    in_height = in_stream.height
    if in_stream.average_rate and in_stream.average_rate > 0:
        in_fps = float(in_stream.average_rate)
    else:
        in_fps = 30.0

    if width is not None and height is not None:
        target_width, target_height = width, height
    elif width is not None:
        target_width = width
        target_height = int(round(in_height * width / in_width))
    elif height is not None:
        target_height = height
        target_width = int(round(in_width * height / in_height))
    else:
        target_width, target_height = in_width, in_height
    target_fps = int(round(fps if fps is not None else in_fps))

    print(f"  Input:  {in_width}x{in_height}, {in_fps:.2f} fps")
    print(f"  Output: {target_width}x{target_height}, {target_fps} fps")

    codec = av.CodecContext.create("cinepak", "w")
    codec.width = target_width
    codec.height = target_height
    codec.pix_fmt = "rgb24"
    codec.time_base = Fraction(1, target_fps)

    need_scale = target_width != in_width or target_height != in_height
    if fps is not None and target_fps < in_fps:
        frame_step = in_fps / target_fps
    else:
        frame_step = 1.0

    frames = []
    frame_accum = 0.0
    total = int(in_stream.frames) if in_stream.frames else None

    for idx, frame in enumerate(input_container.decode(video=0)):
        frame_accum += 1.0
        if frame_accum < frame_step:
            continue
        frame_accum -= frame_step

        if need_scale:
            frame = frame.reformat(width=target_width, height=target_height)
        frame = frame.reformat(format="rgb24")
        frame.pts = None

        for pkt in codec.encode(frame):
            frames.append(bytes(pkt))

        if total and idx % max(1, total // 100) == 0:
            print(f"\r  Encoding... {idx * 100 // total}%", end="", flush=True)

    for pkt in codec.encode(None):
        frames.append(bytes(pkt))

    input_container.close()

    if not frames:
        print("\nError: no video frames encoded", file=sys.stderr)
        sys.exit(1)

    if total:
        print(f"\r  Encoding... 100%")

    print(f"  Encoded: {len(frames)} frames")

    with open(output_xaf, "wb") as f:
        f.write(b"XAF\x00")
        f.write(struct.pack("<H", target_width))
        f.write(struct.pack("<H", target_height))
        f.write(struct.pack("<I", len(frames)))
        f.write(struct.pack("<B", target_fps))
        f.write(b"\x00\x00\x00")

        for fd in frames:
            f.write(struct.pack("<I", len(fd)))
            f.write(fd)

    print(f"Done: {output_xaf} ({os.path.getsize(output_xaf)} bytes)")


def main():
    parser = argparse.ArgumentParser(
        description="Convert any video to XAF format using Cinepak encoding (PyAV)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s video.mp4
  %(prog)s video.mp4 -o output.xaf -f 24 -w 320 -H 240
  %(prog)s video.mp4 -f 15""",
    )
    parser.add_argument("input", help="Input video file")
    parser.add_argument("-o", "--output", help="Output XAF file (default: <input>.xaf)")
    parser.add_argument("-f", "--fps", type=float, help="Frame rate")
    parser.add_argument("-w", "--width", type=int, help="Output width")
    parser.add_argument("-H", "--height", type=int, help="Output height")

    args = parser.parse_args()

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
