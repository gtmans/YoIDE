from PIL import Image
import glob
import os

# Doelresolutie
WIDTH = 240
HEIGHT = 120

# Alle JPG-bestanden in de huidige map
for jpg_file in glob.glob("*.jpg"):
    print(f"Converting {jpg_file}...")

    # Openen en voorbereiden
    img = Image.open(jpg_file)
    img = img.resize((WIDTH, HEIGHT))
    img = img.convert("RGB")
    pix = img.load()

    # Uitvoerbestand
    raw_file = os.path.splitext(jpg_file)[0] + ".raw"

    with open(raw_file, "wb") as f:
        for y in range(HEIGHT):
            for x in range(WIDTH):
                r, g, b = pix[x, y]

                rgb565 = (
                    ((r & 0xF8) << 8) |
                    ((g & 0xFC) << 3) |
                    (b >> 3)
                )

                f.write(rgb565.to_bytes(2, "little"))

    print(f" -> {raw_file} klaar!")

print("Alle JPG-bestanden zijn geconverteerd.")
