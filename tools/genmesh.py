#!/usr/bin/python3

# Default mesh generator
# Input from wavefront obj file

# Usage: ./genmesh.py mesh.obj > mesh.cpp

import sys

HEADER = """
static float CUBE_VERTICES[] = {
    // positions                     // normals                  // texture coords
"""

FOOTER = """
};"""

file = open(sys.argv[1], "r")

vert_coords = []
vert_norms = []
vert_uvs = []

out_vertices = []

# greatest_coord = 0
# least_coord = 0

def normalize(x, y, z):
    return (x/2, y/2, z/2)

for line in file:
    if line.startswith('v '):
        coords = line.split(' ')[1:]
        vert_coords.append((float(coords[0]), float(coords[1]), float(coords[2])))

    if line.startswith('vn '):
        coords = line.split(' ')[1:]
        vert_norms.append((float(coords[0]), float(coords[1]), float(coords[2])))

    if line.startswith('vt '):
        coords = line.split(' ')[1:]
        vert_uvs.append((float(coords[0]), float(coords[1])))

    if line.startswith('f '):
        verts = line.split(' ')[1:]
        for vert in verts:
            coords, uv, normal = vert.split('/')
            coords, uv, normal = int(coords), int(uv), int(normal)
            coords, uv, normal = vert_coords[coords-1], vert_uvs[uv-1], vert_norms[normal-1]

            coords = normalize(*coords)
            # for coord in [*normal]:
            #     if coord > greatest_coord: greatest_coord = coord
            #     if coord < least_coord: least_coord = coord

            out_vertices.append((coords, normal, uv))

print(HEADER)

for coords, normal, uv in out_vertices:
    print(f"\t{coords[0]}, {coords[1]}, {coords[2]},\t{normal[0]}, {normal[1]}, {normal[2]},\t{uv[0]}, {uv[1]},")

print(FOOTER)