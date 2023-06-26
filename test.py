from pathlib import Path

globs = ["*.ino"]
p = Path("src/test/Examples/ExampleSketch")

for g in globs:
    for file in p.glob(g):
        print(file) 