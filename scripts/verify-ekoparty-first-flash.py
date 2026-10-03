#!/usr/bin/env python3

"""Validate the address layout of the badge v1 first-flash Intel HEX."""

from __future__ import annotations

import argparse
import hashlib
import sys
from pathlib import Path


APP_START = 0x27000
BOOTLOADER_START = 0xF4000
MBR_PARAMS_PAGE = 0xFE000
BOOTLOADER_SETTINGS = 0xFF000
FLASH_END = 0x100000
UICR_BOOTLOADER = 0x10001014
UICR_MBR_PARAMS = 0x10001018
UICR_PSELRESET = range(0x10001200, 0x10001208)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path, help="EKO_FIRST_FLASH.hex to validate")
    parser.add_argument(
        "--softdevice",
        type=Path,
        help="Optional reference S140 HEX; every byte must match the first-flash image",
    )
    parser.add_argument(
        "--flash-readback",
        type=Path,
        help="Optional 1 MiB flash dump read back from the programmed nRF52840",
    )
    parser.add_argument(
        "--uicr-readback",
        type=Path,
        help="Optional 4 KiB dump beginning at UICR address 0x10001000",
    )
    return parser.parse_args()


def load_hex(path: Path) -> dict[int, int]:
    memory: dict[int, int] = {}
    base = 0
    saw_eof = False

    with path.open("r", encoding="ascii") as handle:
        for line_number, raw_line in enumerate(handle, 1):
            line = raw_line.strip()
            if not line:
                continue
            if not line.startswith(":"):
                raise ValueError(f"{path}:{line_number}: Intel HEX record must start with ':'")

            try:
                record = bytes.fromhex(line[1:])
            except ValueError as exc:
                raise ValueError(f"{path}:{line_number}: invalid hexadecimal data") from exc

            if len(record) < 5 or len(record) != record[0] + 5:
                raise ValueError(f"{path}:{line_number}: invalid record length")
            if sum(record) & 0xFF:
                raise ValueError(f"{path}:{line_number}: checksum mismatch")

            count = record[0]
            address = (record[1] << 8) | record[2]
            record_type = record[3]
            data = record[4 : 4 + count]

            if record_type == 0x00:
                absolute = base + address
                for offset, value in enumerate(data):
                    target = absolute + offset
                    previous = memory.get(target)
                    if previous is not None and previous != value:
                        raise ValueError(f"{path}:{line_number}: conflicting data at 0x{target:08X}")
                    memory[target] = value
            elif record_type == 0x01:
                saw_eof = True
                break
            elif record_type == 0x02:
                if count != 2:
                    raise ValueError(f"{path}:{line_number}: invalid segment-address record")
                base = int.from_bytes(data, "big") << 4
            elif record_type == 0x04:
                if count != 2:
                    raise ValueError(f"{path}:{line_number}: invalid linear-address record")
                base = int.from_bytes(data, "big") << 16
            elif record_type not in {0x03, 0x05}:
                raise ValueError(f"{path}:{line_number}: unsupported record type 0x{record_type:02X}")

    if not saw_eof:
        raise ValueError(f"{path}: missing end-of-file record")
    if not memory:
        raise ValueError(f"{path}: contains no data")
    return memory


def read_u32(memory: dict[int, int], address: int) -> int | None:
    values = [memory.get(address + offset) for offset in range(4)]
    if any(value is None for value in values):
        return None
    return int.from_bytes(bytes(values), "little")  # type: ignore[arg-type]


def segments(memory: dict[int, int]) -> list[tuple[int, int]]:
    ordered = sorted(memory)
    result: list[tuple[int, int]] = []
    start = previous = ordered[0]
    for address in ordered[1:]:
        if address != previous + 1:
            result.append((start, previous + 1))
            start = address
        previous = address
    result.append((start, previous + 1))
    return result


def main() -> int:
    args = parse_args()
    failures: list[str] = []

    try:
        image = load_hex(args.image)
    except (OSError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 2

    flash_addresses = [address for address in image if address < FLASH_END]
    if not flash_addresses or min(flash_addresses) != 0:
        failures.append("MBR/SoftDevice must begin at flash address 0x00000000")
    if any(APP_START <= address < BOOTLOADER_START for address in flash_addresses):
        failures.append("image writes into the application or Meshtastic data region 0x27000..0xF3FFF")
    if not any(BOOTLOADER_START <= address < MBR_PARAMS_PAGE for address in flash_addresses):
        failures.append("bootloader data is missing from 0xF4000..0xFDFFF")
    if any(BOOTLOADER_SETTINGS <= address < FLASH_END for address in flash_addresses):
        failures.append("first-flash image must leave the bootloader settings page at 0xFF000 erased")

    bootloader_address = read_u32(image, UICR_BOOTLOADER)
    if bootloader_address != BOOTLOADER_START:
        failures.append(
            f"UICR.NRFFW[0] is {bootloader_address!r}, expected bootloader address 0x{BOOTLOADER_START:08X}"
        )
    mbr_params_address = read_u32(image, UICR_MBR_PARAMS)
    if mbr_params_address != MBR_PARAMS_PAGE:
        failures.append(
            f"UICR.NRFFW[1] is {mbr_params_address!r}, expected MBR params page 0x{MBR_PARAMS_PAGE:08X}"
        )
    if any(address in image for address in UICR_PSELRESET):
        failures.append("image writes UICR.PSELRESET even though badge v1 leaves P0.18/nRESET unconnected")

    if args.softdevice:
        try:
            softdevice = load_hex(args.softdevice)
        except (OSError, ValueError) as exc:
            failures.append(str(exc))
        else:
            mismatches = [
                address
                for address, value in softdevice.items()
                if image.get(address) != value
            ]
            if mismatches:
                failures.append(
                    f"bundled SoftDevice differs from {args.softdevice} at 0x{mismatches[0]:08X}"
                )

    if args.flash_readback:
        try:
            flash_readback = args.flash_readback.read_bytes()
        except OSError as exc:
            failures.append(str(exc))
        else:
            if len(flash_readback) != FLASH_END:
                failures.append(
                    f"flash readback has {len(flash_readback)} bytes; expected exactly {FLASH_END}"
                )
            else:
                mismatch = next(
                    (
                        address
                        for address, actual in enumerate(flash_readback)
                        if actual != image.get(address, 0xFF)
                    ),
                    None,
                )
                if mismatch is not None:
                    failures.append(
                        f"flash readback differs at 0x{mismatch:08X}: "
                        f"actual 0x{flash_readback[mismatch]:02X}, expected 0x{image.get(mismatch, 0xFF):02X}"
                    )

    if args.uicr_readback:
        try:
            uicr_readback = args.uicr_readback.read_bytes()
        except OSError as exc:
            failures.append(str(exc))
        else:
            if len(uicr_readback) != 0x1000:
                failures.append(
                    f"UICR readback has {len(uicr_readback)} bytes; expected exactly 4096"
                )
            else:
                uicr_base = 0x10001000
                expected_addresses = [address for address in image if uicr_base <= address < uicr_base + 0x1000]
                mismatch = next(
                    (
                        address
                        for address in expected_addresses
                        if uicr_readback[address - uicr_base] != image[address]
                    ),
                    None,
                )
                if mismatch is not None:
                    failures.append(
                        f"UICR readback differs at 0x{mismatch:08X}: "
                        f"actual 0x{uicr_readback[mismatch - uicr_base]:02X}, expected 0x{image[mismatch]:02X}"
                    )
                reset_mismatch = next(
                    (
                        address
                        for address in UICR_PSELRESET
                        if uicr_readback[address - uicr_base] != 0xFF
                    ),
                    None,
                )
                if reset_mismatch is not None:
                    failures.append(
                        f"UICR.PSELRESET readback is programmed at 0x{reset_mismatch:08X}; expected erased 0xFF"
                    )

    digest = hashlib.sha256(args.image.read_bytes()).hexdigest()
    print(f"Image: {args.image}")
    print(f"SHA-256: {digest}")
    print("Segments:")
    for start, end in segments(image):
        print(f"  0x{start:08X}..0x{end - 1:08X} ({end - start} bytes)")

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        return 2

    checks = "layout, S140, UICR and P0.18 policy"
    if args.flash_readback or args.uicr_readback:
        checks += ", including supplied readback"
    print(f"PASS: first-flash {checks} are consistent")
    return 0


if __name__ == "__main__":
    sys.exit(main())
