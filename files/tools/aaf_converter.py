#!/usr/bin/env python3
"""
Convert Fallout 2 .AAF fonts to/from a single BMP grid.
If --original is omitted, the script will auto‑detect uniform glyph sizes from the grid.
Usage:
    Extract:   python3 aaf_converter.py to_bmp font.aaf grid.bmp [--border]
    Rebuild:   python3 aaf_converter.py from_bmp grid.bmp new_font.aaf [--original original.aaf] [--border]
Options:
    --padding P          Pixels between glyphs in the grid (default 2)
    --border             Draw/expect a 1‑pixel white border around each glyph cell
    --original F         Original .AAF file (optional) – if omitted, uniform spacing is auto‑detected
"""

import os
import sys
import struct
import argparse
from PIL import Image, ImageDraw

AAF_HEADER_SIZE = 12
AAF_GLYPH_COUNT = 256
AAF_GLYPH_ENTRY_SIZE = 8
AAF_BITMAP_BASE = 0x080C

def value_to_grey(v):
    return int(round(v * 255 / 9)) if v <= 9 else 255

def grey_to_value(g):
    v = int(round(g * 9 / 255))
    if v < 0: v = 0
    if v > 9: v = 9
    return v

def read_aaf(filepath):
    with open(filepath, 'rb') as f:
        data = f.read()
    if len(data) < AAF_BITMAP_BASE:
        raise ValueError("File too small.")
    header = data[:AAF_HEADER_SIZE]
    if header[:4] != b'AAFF':
        raise ValueError("Invalid AAF signature.")
    glyphs = []
    for i in range(AAF_GLYPH_COUNT):
        offset = AAF_HEADER_SIZE + i * AAF_GLYPH_ENTRY_SIZE
        if offset + 8 > len(data):
            raise ValueError(f"Glyph {i} entry out of range.")
        width = struct.unpack('>H', data[offset:offset+2])[0]
        height = struct.unpack('>H', data[offset+2:offset+4])[0]
        data_offset = struct.unpack('>I', data[offset+4:offset+8])[0]
        pixel_start = AAF_BITMAP_BASE + data_offset
        pixel_count = width * height
        if pixel_count > 0:
            if pixel_start + pixel_count > len(data):
                raise ValueError(f"Glyph {i} pixel data out of range.")
            pixels = list(data[pixel_start:pixel_start + pixel_count])
        else:
            pixels = []
        glyphs.append({
            'width': width,
            'height': height,
            'data_offset': data_offset,
            'pixels': pixels,
        })
    return header, glyphs

def save_aaf(filepath, header, glyphs):
    entries = bytearray()
    all_pixels = bytearray()
    current_offset = 0
    for g in glyphs:
        width = g['width']
        height = g['height']
        pixels = g['pixels']
        expected = width * height
        if len(pixels) != expected:
            raise ValueError(f"Glyph pixel count mismatch: expected {expected}, got {len(pixels)}")
        entries += struct.pack('>H', width)
        entries += struct.pack('>H', height)
        entries += struct.pack('>I', current_offset)
        all_pixels.extend(pixels)
        current_offset += len(pixels)
    with open(filepath, 'wb') as f:
        f.write(header)
        f.write(entries)
        f.write(all_pixels)

def glyph_to_image(glyph, max_width, max_height, padding):
    cell_w = max_width + padding
    cell_h = max_height + padding
    img = Image.new('L', (cell_w, cell_h), 0)
    x_offset = 0
    y_offset = cell_h - glyph['height']
    if y_offset < 0:
        y_offset = 0
    for y in range(glyph['height']):
        for x in range(glyph['width']):
            idx = y * glyph['width'] + x
            if idx < len(glyph['pixels']):
                val = glyph['pixels'][idx]
                if val != 0:
                    grey = value_to_grey(val)
                    img.putpixel((x_offset + x, y_offset + y), grey)
    return img

def aaf_to_grid(header, glyphs, output_bmp, padding=2, border=False):
    actual_padding = max(padding, 2) if border else padding
    max_w = max(g['width'] for g in glyphs) if glyphs else 1
    max_h = max(g['height'] for g in glyphs) if glyphs else 1
    cell_w = max_w + actual_padding
    cell_h = max_h + actual_padding
    cols = 16
    rows = 16
    grid_w = cols * cell_w
    grid_h = rows * cell_h
    grid_img = Image.new('L', (grid_w, grid_h), 0)

    for idx, glyph in enumerate(glyphs):
        row = idx // cols
        col = idx % cols
        x = col * cell_w
        y = row * cell_h
        paste_x = 1 if border else 0
        paste_y = (cell_h - glyph['height'] - 1) if border else (cell_h - glyph['height'])
        if paste_y < 0:
            paste_y = 0
        cell_canvas = Image.new('L', (cell_w, cell_h), 0)
        for gy in range(glyph['height']):
            for gx in range(glyph['width']):
                val = glyph['pixels'][gy * glyph['width'] + gx] if glyph['pixels'] else 0
                if val != 0:
                    cell_canvas.putpixel((paste_x + gx, paste_y + gy), value_to_grey(val))
        if border:
            draw = ImageDraw.Draw(cell_canvas)
            draw.rectangle([0, 0, cell_w - 1, cell_h - 1], outline=255, width=1)
        grid_img.paste(cell_canvas, (x, y))

    grid_img.save(output_bmp, format='BMP')
    print(f"Saved grid to {output_bmp} (cell size: {cell_w}x{cell_h}, padding: {actual_padding}, border: {border})")

def grid_to_aaf(input_bmp, output_aaf, padding=2, original_aaf=None, border=False):
    grid = Image.open(input_bmp).convert('L')
    grid_w, grid_h = grid.size
    if grid_w % 16 != 0 or grid_h % 16 != 0:
        print(f"Warning: grid dimensions not divisible by 16.")
    cell_w = grid_w // 16
    cell_h = grid_h // 16
    print(f"Cell size: {cell_w}x{cell_h}")

    # If original is provided, load it for dimensions/header
    orig_glyphs = None
    orig_header = None
    if original_aaf is not None and os.path.exists(original_aaf):
        orig_header, orig_glyphs = read_aaf(original_aaf)
        print(f"Loaded original AAF with {len(orig_glyphs)} glyphs for reference.")

    # Extract glyphs from grid (as trimmed bounding boxes)
    extracted_glyphs = []
    for idx in range(AAF_GLYPH_COUNT):
        row = idx // 16
        col = idx % 16
        x0 = col * cell_w
        y0 = row * cell_h

        # Crop the cell
        cell = grid.crop((x0, y0, x0 + cell_w, y0 + cell_h))

        # Remove border if present (1 pixel from each side)
        if border:
            cell = cell.crop((1, 1, cell_w - 1, cell_h - 1))

        # Now cell is the interior (glyph + padding, without border)
        bbox = cell.getbbox()
        if bbox is None:
            extracted_glyphs.append({'width': 0, 'height': 0, 'pixels': []})
            continue
        left, top, right, bottom = bbox
        glyph_img = cell.crop((left, top, right, bottom))
        pixels = []
        for y in range(bottom - top):
            for x in range(right - left):
                grey = glyph_img.getpixel((x, y))
                val = grey_to_value(grey)
                pixels.append(val)
        extracted_glyphs.append({
            'width': right - left,
            'height': bottom - top,
            'pixels': pixels,
        })

    # Pad to 256
    while len(extracted_glyphs) < AAF_GLYPH_COUNT:
        extracted_glyphs.append({'width': 0, 'height': 0, 'pixels': []})

    # Determine final glyph dimensions
    if orig_glyphs is not None:
        # Use original dimensions
        final_glyphs = []
        for idx in range(AAF_GLYPH_COUNT):
            orig = orig_glyphs[idx]
            width = orig['width']
            height = orig['height']
            if width == 0:
                width = 1
            if height == 0:
                height = 1
            final_pixels = [0] * (width * height)
            trimmed = extracted_glyphs[idx]
            if trimmed['width'] > 0 and trimmed['height'] > 0:
                paste_x = 0
                paste_y = height - trimmed['height']
                if paste_y < 0:
                    paste_y = 0
                for y in range(trimmed['height']):
                    for x in range(trimmed['width']):
                        val = trimmed['pixels'][y * trimmed['width'] + x]
                        if val != 0:
                            dest_x = paste_x + x
                            dest_y = paste_y + y
                            if dest_x < width and dest_y < height:
                                final_pixels[dest_y * width + dest_x] = val
            final_glyphs.append({
                'width': width,
                'height': height,
                'pixels': final_pixels,
            })
        max_height = max(g['height'] for g in final_glyphs)
        header = orig_header
        if header is None:
            header = b'AAFF' + struct.pack('>H', max_height) + b'\x00' * 6
    else:
        # No original – compute uniform size from the extracted glyphs
        non_empty = [g for g in extracted_glyphs if g['width'] > 0 and g['height'] > 0]
        if non_empty:
            max_w = max(g['width'] for g in non_empty)
            max_h = max(g['height'] for g in non_empty)
        else:
            max_w = 8
            max_h = 12

        # Build final glyphs with uniform dimensions
        final_glyphs = []
        for idx in range(AAF_GLYPH_COUNT):
            trimmed = extracted_glyphs[idx]
            width = max_w
            height = max_h
            final_pixels = [0] * (width * height)
            if trimmed['width'] > 0 and trimmed['height'] > 0:
                paste_x = 0
                paste_y = height - trimmed['height']
                if paste_y < 0:
                    paste_y = 0
                for y in range(trimmed['height']):
                    for x in range(trimmed['width']):
                        val = trimmed['pixels'][y * trimmed['width'] + x]
                        if val != 0:
                            dest_x = paste_x + x
                            dest_y = paste_y + y
                            if dest_x < width and dest_y < height:
                                final_pixels[dest_y * width + dest_x] = val
            final_glyphs.append({
                'width': width,
                'height': height,
                'pixels': final_pixels,
            })
        max_height = max_h
        header = b'AAFF' + struct.pack('>H', max_height) + b'\x00' * 6
        print(f"Auto‑detected uniform glyph size: {max_w}x{max_h}")

    save_aaf(output_aaf, header, final_glyphs)
    print(f"Rebuilt AAF saved to {output_aaf}")

def main():
    parser = argparse.ArgumentParser(description='Fallout 2 AAF font converter (greyscale)')
    subparsers = parser.add_subparsers(dest='command', required=True)

    to_bmp = subparsers.add_parser('to_bmp', help='Extract AAF to BMP grid')
    to_bmp.add_argument('input_aaf', help='Input .AAF file')
    to_bmp.add_argument('output_bmp', help='Output BMP file')
    to_bmp.add_argument('--padding', type=int, default=2, help='Padding between glyphs (default 2)')
    to_bmp.add_argument('--border', action='store_true', help='Draw a 1‑pixel white border around each glyph cell')

    from_bmp = subparsers.add_parser('from_bmp', help='Rebuild AAF from BMP grid')
    from_bmp.add_argument('input_bmp', help='Input BMP grid file')
    from_bmp.add_argument('output_aaf', help='Output .AAF file')
    from_bmp.add_argument('--padding', type=int, default=2, help='Padding used when generating the grid (default 2)')
    from_bmp.add_argument('--original', help='Original .AAF file (optional) – if omitted, auto‑detect uniform spacing')
    from_bmp.add_argument('--border', action='store_true', help='Expect a 1‑pixel border around each glyph cell (must match extraction)')

    args = parser.parse_args()

    if args.command == 'to_bmp':
        header, glyphs = read_aaf(args.input_aaf)
        aaf_to_grid(header, glyphs, args.output_bmp, args.padding, args.border)
    elif args.command == 'from_bmp':
        grid_to_aaf(args.input_bmp, args.output_aaf, args.padding, args.original, args.border)

if __name__ == '__main__':
    main()