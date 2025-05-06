#!/usr/bin/bash

(cd core/src; cinclude2dot --paths > /tmp/tree.dot)
dot -Tsvg /tmp/tree.dot -o /tmp/tree.svg
inkscape /tmp/tree.svg &