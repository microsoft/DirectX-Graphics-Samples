#*********************************************************
#
# Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# This code is licensed under the MIT License (MIT).
# THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
# IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
# PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
#
#*********************************************************

"""Compare scene or physics dumps from two runs of the sample.

Usage: python tools/compare_dumps.py scene baseline_scene.bin candidate_scene.bin
       python tools/compare_dumps.py state baseline_state.bin candidate_state.bin --frame 119
"""

import argparse
import mmap
import struct


def compare_scene(baseline, candidate):
    header = struct.Struct("<II")
    record = struct.Struct("<I12f")
    counts_a = header.unpack_from(baseline)
    counts_b = header.unpack_from(candidate)
    if counts_a != counts_b or len(baseline) != len(candidate):
        raise ValueError(f"scene counts/length differ: {counts_a}, {counts_b}")
    count = (len(baseline) - header.size) // record.size
    mesh_mismatches = 0
    transform_mismatches = 0
    max_error = 0.0
    for index in range(count):
        offset = header.size + index * record.size
        a = record.unpack_from(baseline, offset)
        b = record.unpack_from(candidate, offset)
        mesh_mismatches += a[0] != b[0]
        error = max(abs(x - y) for x, y in zip(a[1:], b[1:]))
        transform_mismatches += error > 0.001
        max_error = max(max_error, error)
    print(f"scene: static={counts_a[0]}, dynamic={counts_a[1]}, records={count}")
    print(f"mesh mismatches={mesh_mismatches}; transforms over 0.001={transform_mismatches}; "
          f"maximum component error={max_error:.9g}")


def compare_state(baseline, candidate, frame):
    state = struct.Struct("<12f3fI3fI4I")
    if len(baseline) != len(candidate) or len(baseline) % state.size:
        raise ValueError("state lengths differ or are not multiples of 96 bytes")
    counts = {"transform>0.0001": 0, "transform>0.01": 0,
              "transform>0.1": 0, "flags": 0, "history": 0}
    maxima = [0.0, 0.0, 0.0]
    active = [0, 0]
    for index in range(len(baseline) // state.size):
        offset = index * state.size
        a = state.unpack_from(baseline, offset)
        b = state.unpack_from(candidate, offset)
        errors = [max(abs(a[j] - b[j]) for j in positions)
                  for positions in (range(12), range(12, 15), range(16, 19))]
        maxima = [max(old, new) for old, new in zip(maxima, errors)]
        for threshold in (0.0001, 0.01, 0.1):
            counts[f"transform>{threshold}"] += errors[0] > threshold
        counts["flags"] += a[15] != b[15]
        counts["history"] += a[19:] != b[19:]
        active[0] += a[22] == frame
        active[1] += b[22] == frame
    print(f"state: dominoes={len(baseline) // state.size}, frame={frame}")
    print(f"max transform/linear/angular error={maxima}")
    print(f"mismatch counts={counts}; changed dominoes baseline/candidate={active}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("kind", choices=("scene", "state"))
    parser.add_argument("baseline")
    parser.add_argument("candidate")
    parser.add_argument("--frame", type=int, default=0)
    args = parser.parse_args()
    with open(args.baseline, "rb") as baseline_file, open(args.candidate, "rb") as candidate_file:
        with mmap.mmap(baseline_file.fileno(), 0, access=mmap.ACCESS_READ) as baseline:
            with mmap.mmap(candidate_file.fileno(), 0, access=mmap.ACCESS_READ) as candidate:
                if args.kind == "scene":
                    compare_scene(baseline, candidate)
                else:
                    compare_state(baseline, candidate, args.frame)


if __name__ == "__main__":
    main()
