# Pre-requisite packages

From AUR: `mingw-w64-qt6-base`, `mingw-w64-rust-bin` (needed for qt6)

NOTE: There are a few circular/cyclic dependencies in the build chain. In order
to resolve them, you must first install the following packages first:

`mingw-w64-freetype2-bootstrap mingw-w64-cairo-bootstrap`

Then install the full versions:

`mingw-w64-freetype2 mingw-w64-cairo`

When prompted to remove the `-bootstrap` variants to be replaced with the normal variants, answer `y`

# Configuration and build

Configure using `just configure-mingw`

Build using `just build`