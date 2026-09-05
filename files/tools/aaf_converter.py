#!/usr/bin/env python3
"""
Fallout 2 AAF Font Converter – Extract/rebuild AAF fonts to/from a BMP grid.
Supports preserving original spacing or creating a new uniform icon font.

Usage:
  Extract:   python aaf_converter.py to_bmp font.aaf grid.bmp [--padding P] [--border]
  Rebuild:   python aaf_converter.py from_bmp grid.bmp new_font.aaf [options]

Rebuild options (choose one mode):
  --original F       Original AAF to copy metrics from (spacing overrides can be added)
  (or without --original, create a new uniform font using the options below)

  --glyph-width W    Force every glyph to this width (default: auto max)
  --glyph-height H   Force every glyph to this height (default: auto max)
  --max-height H     Set the maxHeight header field (default: glyph-height)
  --letter-spacing S Pixels between glyphs (default: 0)
  --word-spacing S   Width of space character (default: letter-spacing)
  --line-spacing S   Extra vertical pixels between lines (default: 0)
  --v-align {top,center,bottom} Vertical alignment (default: bottom)
  --h-align {left,center} Horizontal alignment (default: left)
  --mono             Shorthand for --glyph-width X --glyph-height X (square)

  --padding P        Padding used when generating the grid (must match extraction)
  --border           Expect a 1‑pixel border around each glyph cell (must match extraction)
  --no-trim          Preserve the exact pixel positions in the BMP (no trimming or repositioning)
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

# ----------------------------------------------------------------------
# Helpers
# ----------------------------------------------------------------------
def value_to_grey(v):
    return int(round(v * 255 / 9)) if v <= 9 else 255

def grey_to_value(g):
    v = int(round(g * 9 / 255))
    if v < 0: v = 0
    if v > 9: v = 9
    return v

# ----------------------------------------------------------------------
# AAF I/O
# ----------------------------------------------------------------------
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

# ----------------------------------------------------------------------
# Extraction (to_bmp)
# ----------------------------------------------------------------------
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

# ----------------------------------------------------------------------
# Rebuild (from_bmp)
# ----------------------------------------------------------------------
def grid_to_aaf(
    input_bmp,
    output_aaf,
    padding=2,
    border=False,
    original_aaf=None,
    glyph_width=None,
    glyph_height=None,
    max_height=None,
    letter_spacing=0,
    word_spacing=None,
    line_spacing=0,
    v_align='bottom',
    h_align='left',
    no_trim=False,
):
    # Load the grid image
    grid = Image.open(input_bmp).convert('L')
    grid_w, grid_h = grid.size
    if grid_w % 16 != 0 or grid_h % 16 != 0:
        print(f"Warning: grid dimensions ({grid_w}x{grid_h}) not divisible by 16.")
    cell_w = grid_w // 16
    cell_h = grid_h // 16
    print(f"Cell size: {cell_w}x{cell_h}")

    # ------------------------------------------------------------------
    # Step 1: Extract glyphs from the grid
    # ------------------------------------------------------------------
    trimmed_glyphs = []
    for idx in range(AAF_GLYPH_COUNT):
        row = idx // 16
        col = idx % 16
        x0 = col * cell_w
        y0 = row * cell_h

        # Crop the cell, removing the border if present
        if border:
            cell = grid.crop((x0 + 1, y0 + 1, x0 + cell_w - 1, y0 + cell_h - 1))
        else:
            cell = grid.crop((x0, y0, x0 + cell_w, y0 + cell_h))

        if no_trim:
            w = cell.width
            h = cell.height
            pixels = []
            for y in range(h):
                for x in range(w):
                    grey = cell.getpixel((x, y))
                    pixels.append(grey_to_value(grey))
            trimmed_glyphs.append({
                'width': w,
                'height': h,
                'pixels': pixels,
            })
        else:
            bbox = cell.getbbox()
            if bbox is None:
                trimmed_glyphs.append({'width': 0, 'height': 0, 'pixels': []})
                continue
            left, top, right, bottom = bbox
            glyph_img = cell.crop((left, top, right, bottom))
            pixels = []
            for y in range(bottom - top):
                for x in range(right - left):
                    grey = glyph_img.getpixel((x, y))
                    pixels.append(grey_to_value(grey))
            trimmed_glyphs.append({
                'width': right - left,
                'height': bottom - top,
                'pixels': pixels,
            })

    # Pad to 256
    while len(trimmed_glyphs) < AAF_GLYPH_COUNT:
        trimmed_glyphs.append({'width': 0, 'height': 0, 'pixels': []})

    # ------------------------------------------------------------------
    # Step 2: Build final glyphs
    # ------------------------------------------------------------------
    if original_aaf is not None and os.path.exists(original_aaf):
        # --- Mode: preserve original metrics and alignment ---
        orig_header, orig_glyphs = read_aaf(original_aaf)
        print(f"Loaded original AAF with {len(orig_glyphs)} glyphs for reference.")

        final_glyphs = []
        for idx in range(AAF_GLYPH_COUNT):
            orig = orig_glyphs[idx]
            width = orig['width']
            height = orig['height']
            if width == 0:
                width = 1
            if height == 0:
                height = 1

            # Compute original top bearing (first non‑zero row)
            orig_top = 0
            if orig['pixels']:
                for y in range(height):
                    for x in range(width):
                        if orig['pixels'][y * width + x] != 0:
                            orig_top = y
                            break
                    else:
                        continue
                    break

            # Start with a blank canvas of original size
            final_pixels = [0] * (width * height)
            trimmed = trimmed_glyphs[idx]

            if trimmed['width'] > 0 and trimmed['height'] > 0:
                # Paste trimmed glyph at the original top bearing
                paste_y = orig_top
                # Clip if the trimmed glyph is taller than remaining space
                max_paste_h = height - paste_y
                if max_paste_h > 0:
                    copy_h = min(trimmed['height'], max_paste_h)
                    copy_w = min(trimmed['width'], width)  # horizontal fits because we keep original width
                    for y in range(copy_h):
                        for x in range(copy_w):
                            val = trimmed['pixels'][y * trimmed['width'] + x]
                            if val != 0:
                                dest_x = x  # left‑aligned
                                dest_y = paste_y + y
                                if dest_x < width and dest_y < height:
                                    final_pixels[dest_y * width + dest_x] = val

            final_glyphs.append({
                'width': width,
                'height': height,
                'pixels': final_pixels,
            })

        # Build header, allow spacing overrides
        header = bytearray(orig_header)
        if letter_spacing != 0:
            struct.pack_into('>H', header, 6, letter_spacing)
        if word_spacing is not None:
            struct.pack_into('>H', header, 8, word_spacing)
        else:
            # Read original word_spacing as integer
            orig_word_spacing = struct.unpack_from('>H', orig_header, 8)[0]
            struct.pack_into('>H', header, 8, letter_spacing if letter_spacing else orig_word_spacing)
        if line_spacing != 0:
            struct.pack_into('>H', header, 10, line_spacing)
        if max_height is not None:
            struct.pack_into('>H', header, 4, max_height)

        save_aaf(output_aaf, bytes(header), final_glyphs)
        print(f"Rebuilt AAF saved to {output_aaf} (using original metrics, alignment preserved)")
        return  # early exit

    # ------------------------------------------------------------------
    # Mode: create a new uniform font from scratch (no --original)
    # ------------------------------------------------------------------
    if no_trim:
        if glyph_width is None:
            glyph_width = cell_w - (2 if border else 0)
        if glyph_height is None:
            glyph_height = cell_h - (2 if border else 0)
        if max_height is None:
            max_height = glyph_height
    else:
        non_empty = [g for g in trimmed_glyphs if g['width'] > 0 and g['height'] > 0]
        if glyph_width is None:
            glyph_width = max(g['width'] for g in non_empty) if non_empty else 8
        if glyph_height is None:
            glyph_height = max(g['height'] for g in non_empty) if non_empty else 12
        if max_height is None:
            max_height = glyph_height

    if word_spacing is None:
        word_spacing = letter_spacing if letter_spacing != 0 else 0

    final_glyphs = []
    for idx in range(AAF_GLYPH_COUNT):
        trimmed = trimmed_glyphs[idx]
        final_pixels = [0] * (glyph_width * glyph_height)

        if trimmed['width'] > 0 and trimmed['height'] > 0:
            if no_trim:
                copy_w = min(trimmed['width'], glyph_width)
                copy_h = min(trimmed['height'], glyph_height)
                for y in range(copy_h):
                    for x in range(copy_w):
                        val = trimmed['pixels'][y * trimmed['width'] + x]
                        if val != 0:
                            final_pixels[y * glyph_width + x] = val
            else:
                if h_align == 'center':
                    paste_x = (glyph_width - trimmed['width']) // 2
                else:
                    paste_x = 0

                if v_align == 'center':
                    paste_y = (glyph_height - trimmed['height']) // 2
                elif v_align == 'top':
                    paste_y = 0
                else:  # bottom
                    paste_y = glyph_height - trimmed['height']

                if paste_x < 0:
                    paste_x = 0
                if paste_y < 0:
                    paste_y = 0

                for y in range(trimmed['height']):
                    for x in range(trimmed['width']):
                        val = trimmed['pixels'][y * trimmed['width'] + x]
                        if val != 0:
                            dest_x = paste_x + x
                            dest_y = paste_y + y
                            if dest_x < glyph_width and dest_y < glyph_height:
                                final_pixels[dest_y * glyph_width + dest_x] = val

        final_glyphs.append({
            'width': glyph_width,
            'height': glyph_height,
            'pixels': final_pixels,
        })

    # Build header
    header = b'AAFF'
    header += struct.pack('>H', max_height)
    header += struct.pack('>H', letter_spacing)
    header += struct.pack('>H', word_spacing)
    header += struct.pack('>H', line_spacing)

    save_aaf(output_aaf, header, final_glyphs)
    print(f"Rebuilt AAF saved to {output_aaf} (uniform size: {glyph_width}x{glyph_height}, maxHeight={max_height})")

# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------
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
    from_bmp.add_argument('--border', action='store_true', help='Expect a 1‑pixel border around each glyph cell (must match extraction)')
    from_bmp.add_argument('--no-trim', action='store_true', help='Preserve exact pixel positions (no trimming or repositioning)')

    from_bmp.add_argument('--original', help='Original .AAF file (optional) – copy metrics from this font')

    from_bmp.add_argument('--glyph-width', type=int, help='Force every glyph to this width (auto if not set)')
    from_bmp.add_argument('--glyph-height', type=int, help='Force every glyph to this height (auto if not set)')
    from_bmp.add_argument('--max-height', type=int, help='Set maxHeight header field (default = glyph-height)')
    from_bmp.add_argument('--letter-spacing', type=int, default=0, help='Pixels between glyphs (default 0)')
    from_bmp.add_argument('--word-spacing', type=int, help='Width of space character (default = letter-spacing)')
    from_bmp.add_argument('--line-spacing', type=int, default=0, help='Extra vertical pixels between lines (default 0)')
    from_bmp.add_argument('--v-align', choices=['top', 'center', 'bottom'], default='bottom', help='Vertical alignment (default bottom)')
    from_bmp.add_argument('--h-align', choices=['left', 'center'], default='left', help='Horizontal alignment (default left)')
    from_bmp.add_argument('--mono', action='store_true', help='Shorthand for --glyph-width X --glyph-height X (square)')

    args = parser.parse_args()

    if args.command == 'to_bmp':
        header, glyphs = read_aaf(args.input_aaf)
        aaf_to_grid(header, glyphs, args.output_bmp, args.padding, args.border)

    elif args.command == 'from_bmp':
        if args.mono:
            if args.glyph_width is not None and args.glyph_height is None:
                args.glyph_height = args.glyph_width
            elif args.glyph_height is not None and args.glyph_width is None:
                args.glyph_width = args.glyph_height

        grid_to_aaf(
            input_bmp=args.input_bmp,
            output_aaf=args.output_aaf,
            padding=args.padding,
            border=args.border,
            original_aaf=args.original,
            glyph_width=args.glyph_width,
            glyph_height=args.glyph_height,
            max_height=args.max_height,
            letter_spacing=args.letter_spacing,
            word_spacing=args.word_spacing,
            line_spacing=args.line_spacing,
            v_align=args.v_align,
            h_align=args.h_align,
            no_trim=args.no_trim,
        )

if __name__ == '__main__':
    main()