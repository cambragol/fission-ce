# Fallout 2 AAF Font Converter

A Python tool to extract, edit, and rebuild Fallout 2 `.AAF` bitmap font
files using a single BMP grid image.

## Features

-   **Extract** any `.AAF` font to a 16×16 BMP grid for easy editing in
    any image editor.
-   **Rebuild** an `.AAF` from an edited grid, preserving:
    -   Original spacing, glyph widths, heights, and vertical alignment.
    -   Custom uniform sizes and spacing for icon fonts.
-   **Visual border** mode --- add a 1-pixel white border around each
    glyph cell to clearly see cell boundaries.
-   **Preserve exact pixel positions** --- disable automatic trimming to
    keep blank margins.
-   **Full control** over letter spacing, line spacing, word spacing,
    and maximum height.
-   **Greyscale support** --- the grid uses greyscale values (0--255)
    representing the original 0--9 palette indices.

## Requirements

-   Python 3.6+
-   [Pillow](https://python-pillow.org/) (PIL)

Install Pillow with:

``` bash
pip install Pillow
```

## Usage

The script provides two main commands: `to_bmp` (extract) and `from_bmp`
(rebuild).

### 1. Extract a font to a BMP grid

``` bash
python aaf_converter.py to_bmp input.aaf grid.bmp [--padding P] [--border]
```

-   `input.aaf` --- the font file to extract.
-   `grid.bmp` --- output BMP file (will be created).
-   `--padding P` --- extra pixels between glyphs (default 2). Use a
    larger value to create visual separation.
-   `--border` --- draw a 1-pixel white border around each glyph cell.

**Example** (recommended for editing):

``` bash
python aaf_converter.py to_bmp font1.aaf grid.bmp --padding 2 --border
```

### 2. Rebuild an AAF from a BMP grid

``` bash
python aaf_converter.py from_bmp grid.bmp output.aaf [options]
```

#### Preserve original font metrics (spacing, dimensions, alignment)

Use the `--original` flag to copy all metrics from the original font.
Your edits will overlay onto the original glyphs while keeping
everything else identical.

``` bash
python aaf_converter.py from_bmp grid.bmp output.aaf --original font1.aaf --border
```

**Important:** Use `--border` if the grid was extracted with `--border`
--- this tells the script to ignore the border pixels.

You can also override specific spacing values while preserving the rest:

``` bash
python aaf_converter.py from_bmp grid.bmp output.aaf --original font1.aaf --letter-spacing 2 --line-spacing 1 --border
```

#### Create a new uniform icon font (no original)

When you don't provide `--original`, the script creates a new font where
every glyph has the same width and height (useful for icons or
monospaced symbols).

``` bash
python aaf_converter.py from_bmp grid.bmp output.aaf \
    --glyph-width 12 --glyph-height 12 \
    --letter-spacing 1 --line-spacing 1 \
    --v-align center --h-align center
```

Options:

-   `--glyph-width W` --- force every glyph to this width.
-   `--glyph-height H` --- force every glyph to this height.
-   `--max-height H` --- set the `maxHeight` header field.
-   `--letter-spacing S` --- extra horizontal pixels between glyphs.
-   `--word-spacing S` --- width of the space character.
-   `--line-spacing S` --- extra vertical pixels between lines.
-   `--v-align {top,center,bottom}` --- vertical alignment.
-   `--h-align {left,center}` --- horizontal alignment.
-   `--mono` --- shorthand to set width and height equally.

#### Preserve exact pixel positions (no trimming)

``` bash
python aaf_converter.py from_bmp grid.bmp output.aaf --no-trim --border
```

## Examples

### Edit an existing font while preserving spacing

1.  Extract the grid:

    ``` bash
    python aaf_converter.py to_bmp font1.aaf grid.bmp --padding 2 --border
    ```

2.  Edit `grid.bmp` in Photoshop, GIMP, or Paint.NET.

3.  Rebuild:

    ``` bash
    python aaf_converter.py from_bmp grid.bmp new_font.aaf --original font1.aaf --border
    ```

### Create a 12×12 icon font

``` bash
python aaf_converter.py from_bmp icons.bmp icons.aaf \
    --glyph-width 12 --glyph-height 12 \
    --letter-spacing 1 --line-spacing 1 \
    --v-align center --h-align center
```

### Use a pre-aligned grid

``` bash
python aaf_converter.py from_bmp grid.bmp output.aaf --no-trim --border
```

## Notes on Spacing and Alignment

-   **letterSpacing** --- horizontal advance after each glyph.
-   **wordSpacing** --- width of the space character.
-   **lineSpacing** --- vertical advance between lines.
-   **maxHeight** --- maximum glyph height used for line height.
-   **Vertical alignment** affects glyph placement within uniform cells.
-   `--original` preserves original spacing and top bearing for
    descenders.

## Border Mode

-   Extraction draws a 1-pixel white outline around each cell.
-   Glyphs are shifted inward by 1 pixel.
-   Rebuilding with `--border` removes the border automatically.
-   Always use `--border` for both extraction and rebuild.

## File Format

Supports the **Fallout 2 AAF** format:

-   **Header:** 12 bytes (`AAFF` + spacing values, big-endian)
-   **Glyph table:** 256 entries × 8 bytes
-   **Glyph data:** raw 0--9 pixel values beginning at offset `0x080C`

## Troubleshooting

  -----------------------------------------------------------------------
  Problem                             Solution
  ----------------------------------- -----------------------------------
  Glyph pixel count mismatch          Ensure the BMP remains a 16×16 grid
                                      and wasn't resized.

  Spacing looks wrong                 Override with `--letter-spacing`,
                                      `--word-spacing`, or
                                      `--line-spacing`.

  Glyphs are vertically offset        Taller edited glyphs may clip when
                                      using `--original`.

  Border appears in game              Rebuild with the `--border` flag.

  Division by zero crash              Ensure spacing values and
                                      `maxHeight` are greater than zero.
  -----------------------------------------------------------------------

## License

Released under the **MIT License**. You are free to use, modify, and
distribute it.

## Credits

Created for the Fallout 2 modding community.