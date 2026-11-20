#!/usr/bin/env python3
"""Check actual HID descriptor length and 7-byte report packing."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
config = (root / "libraries/EasyHID/usbconfig.h").read_text()
source = (root / "libraries/EasyHID/HIDPrivate.c").read_text()
begin = source.index("usbHidReportDescriptor[]")
values = source[begin:source.index("};", begin)]
values = re.sub(r"//[^\n]*", "", values)
descriptor = bytes(int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{2})", values))
m = re.search(r"(?m)^#define\s+USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH\s+(\d+)", config)
assert m, "USB HID report descriptor length definition missing"
assert len(descriptor) == int(m.group(1)), (len(descriptor), m.group(1))

report_bits = 0
report_ids = []
fields = []
size = count = 0
i = 0
while i < len(descriptor):
    header = descriptor[i]
    i += 1
    assert header != 0xfe, "Unexpected HID long item"
    width = (0, 1, 2, 4)[header & 3]
    item = descriptor[i:i + width]
    assert len(item) == width
    i += width
    typ, tag = (header >> 2) & 3, header >> 4
    value = int.from_bytes(item, "little")
    if typ == 1 and tag == 7:
        size = value
    elif typ == 1 and tag == 9:
        count = value
    elif typ == 1 and tag == 8:
        report_ids.append(value)
    elif typ == 0 and tag == 8:
        fields.append((size, count, value))
        report_bits += size * count

assert not report_ids, report_ids
assert report_bits == 56, report_bits
assert fields == [
    (8, 4, 2),   # 4 axes
    (4, 1, 66),  # Hat Switch with Null State
    (4, 1, 3),   # 4 bits padding
    (1, 10, 2),  # 10 buttons
    (6, 1, 3),   # 6 bits padding
], fields
print(f"HID descriptor OK: {len(descriptor)} bytes, input report = {report_bits // 8} bytes")
