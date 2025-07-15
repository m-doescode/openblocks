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

min_coords: tuple[float, float, float] | None = None
max_coords: tuple[float, float, float] | None = None

def normalize(x, y, z):
    assert min_coords
    assert max_coords
    return ((x-max_coords[0])/(max_coords[0]-min_coords[0])+0.5, (y-max_coords[1])/(max_coords[1]-min_coords[1])+0.5, (z-max_coords[2])/(max_coords[2]-min_coords[2])+0.5)

for line in file:
    if line.startswith('v '):
        coords = line.split(' ')[1:]
        coords = (float(coords[0]), float(coords[1]), float(coords[2]))
        vert_coords.append(coords)

        if not min_coords: min_coords = coords
        if not max_coords: max_coords = coords

        if coords[0] > max_coords[0]: max_coords = (coords[0], max_coords[1], max_coords[2])
        if coords[1] > max_coords[1]: max_coords = (max_coords[0], coords[1], max_coords[2])
        if coords[2] > max_coords[2]: max_coords = (max_coords[0], max_coords[1], coords[2])

        if coords[0] < min_coords[0]: min_coords = (coords[0], min_coords[1], min_coords[2])
        if coords[1] < min_coords[1]: min_coords = (min_coords[0], coords[1], min_coords[2])
        if coords[2] < min_coords[2]: min_coords = (min_coords[0], min_coords[1], coords[2])

    if line.startswith('vn '):
        coords = line.split(' ')[1:]
        vert_norms.append((float(coords[0]), float(coords[1]), float(coords[2])))

    if line.startswith('vt '):
        coords = line.split(' ')[1:]
        vert_uvs.append((float(coords[0]), float(coords[1])))

    if line.startswith('f '):
        verts = line.split(' ')[1:]
        for vert in reversed(verts):
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