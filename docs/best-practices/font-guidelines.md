## Chinese bitmap fonts

For bitmap fonts containing many Chinese characters, ASTC 6x6 is the
recommended initial format when supported by the selected EVE device.

ASTC 6x6 normally provides a useful balance between visual quality and
resource size. Use ASTC 4x4 when small characters, thin strokes, or
complex glyphs are not sufficiently clear.

Always validate representative Chinese characters on the target LCD.

## Character subsets and whitespace

Build a converted-font subset from every localized string and every value that can be formatted at runtime. Include spaces, punctuation, units, currency symbols, replacement or fallback characters, and all supported language variants. Preserve whitespace during extraction rather than assuming the converter supplies it.

For extended fonts, validate the metadata and glyph data as a pair both before and after packing into external flash. Every UI code point must resolve inside the matching glyph asset. In particular, U+0020 must have a positive advance, render no visible pixels, and use storage that does not alias a visible character.

When using an EVE Asset Builder workflow whose font converter provides `--add-space` (`-S`), enable it explicitly. Verify the installed tool's syntax and defaults rather than relying on omission or on a literal space in the subset input.

Generate previews for every produced size and inspect representative mixed strings containing repeated spaces, digits with units, currency and punctuation, and each supported script. Preview validation does not replace checking the final packed flash image because relocated or sparse glyph pointers can still be wrong.

If a legacy font must be repaired, preserve the original asset, give the repaired space independent width and glyph storage, and verify that unrelated widths and glyph bytes remain unchanged. Prefer a clean conversion for the next asset release.
