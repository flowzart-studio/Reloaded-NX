"""Generate the original 256 × 256 Reloaded-NX homebrew launcher icon."""

# SPDX-License-Identifier: MIT
# Artwork and geometry for Reloaded-NX by Flowzart Studio.
from pathlib import Path

from PIL import Image, ImageDraw


def main():
    scale = 3
    canvas = Image.new("RGB", (256 * scale, 256 * scale), "#101c30")
    draw = ImageDraw.Draw(canvas)

    def polygon(points, color):
        draw.polygon([(x * scale, y * scale) for x, y in points], fill=color)

    draw.rounded_rectangle(
        (22 * scale, 22 * scale, 234 * scale, 234 * scale),
        44 * scale,
        fill="#162741",
        outline="#314863",
        width=2 * scale,
    )
    # R is drawn from coordinates. No font or third-party artwork is used.
    polygon(
        [(56, 77), (108, 77), (128, 96), (128, 125), (111, 142),
         (137, 179), (110, 179), (84, 142), (78, 142), (78, 179), (56, 179)],
        "#eef5fc",
    )
    polygon([(78, 99), (103, 99), (107, 103), (107, 118), (102, 122), (78, 122)], "#162741")
    polygon(
        [(138, 77), (162, 77), (177, 105), (193, 77), (217, 77), (190, 126),
         (219, 179), (194, 179), (177, 147), (160, 179), (136, 179), (165, 126)],
        "#5ee9cc",
    )
    icon = Path(__file__).resolve().parents[1] / "switch-port/branding/reloaded-nx.jpg"
    icon.parent.mkdir(parents=True, exist_ok=True)
    canvas.resize((256, 256), Image.Resampling.LANCZOS).save(
        icon, "JPEG", quality=92, subsampling=0, optimize=True
    )


if __name__ == "__main__":
    main()
