#!/usr/bin/env python3
"""Genere un icosaedre placeholder (gltf, flat-shaded, gris neutre "grey-box") en pur Python,
sans dependance externe -- juste pour avoir un premier .gltf a brancher dans <model-viewer>
avant d'avoir de vrais exports (Unreal/Blender) par sort. A supprimer une fois les vrais assets
disponibles ; regenerer avec `python generate-placeholder.py` si besoin d'ajuster la couleur/taille.
"""
import base64
import json
import struct

PHI = (1 + 5 ** 0.5) / 2

BASE_VERTICES = [
    (-1, PHI, 0), (1, PHI, 0), (-1, -PHI, 0), (1, -PHI, 0),
    (0, -1, PHI), (0, 1, PHI), (0, -1, -PHI), (0, 1, -PHI),
    (PHI, 0, -1), (PHI, 0, 1), (-PHI, 0, -1), (-PHI, 0, 1),
]

FACES = [
    (0, 11, 5), (0, 5, 1), (0, 1, 7), (0, 7, 10), (0, 10, 11),
    (1, 5, 9), (5, 11, 4), (11, 10, 2), (10, 7, 6), (7, 1, 8),
    (3, 9, 4), (3, 4, 2), (3, 2, 6), (3, 6, 8), (3, 8, 9),
    (4, 9, 5), (2, 4, 11), (6, 2, 10), (8, 6, 7), (9, 8, 1),
]


def normalize(v):
    length = sum(c * c for c in v) ** 0.5
    return tuple(c / length for c in v)


def sub(a, b):
    return tuple(a[i] - b[i] for i in range(3))


def cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def dot(a, b):
    return sum(a[i] * b[i] for i in range(3))


unit_vertices = [normalize(v) for v in BASE_VERTICES]

# Flat shading : chaque face obtient ses 3 propres sommets (pas de normales partagees), pour le
# look "gemme low-poly" plutot qu'une sphere lisse.
positions = []
normals = []
for a, b, c in FACES:
    pa, pb, pc = unit_vertices[a], unit_vertices[b], unit_vertices[c]
    normal = normalize(cross(sub(pb, pa), sub(pc, pa)))
    centroid = tuple((pa[i] + pb[i] + pc[i]) / 3 for i in range(3))
    # Winding non garanti a la main -- si la normale pointe vers le centre plutot que vers
    # l'exterieur, on inverse l'ordre des 2 derniers sommets pour la retourner.
    if dot(normal, centroid) < 0:
        pb, pc = pc, pb
        normal = normalize(cross(sub(pb, pa), sub(pc, pa)))
    for p in (pa, pb, pc):
        positions.append(p)
    for _ in range(3):
        normals.append(normal)

indices = list(range(len(positions)))

pos_bytes = b"".join(struct.pack("<3f", *p) for p in positions)
norm_bytes = b"".join(struct.pack("<3f", *n) for n in normals)
idx_bytes = b"".join(struct.pack("<H", i) for i in indices)


def pad4(data):
    while len(data) % 4 != 0:
        data += b"\x00"
    return data


pos_bytes = pad4(pos_bytes)
norm_bytes = pad4(norm_bytes)
idx_bytes = pad4(idx_bytes)

buffer = pos_bytes + norm_bytes + idx_bytes

pos_view_len = len(pos_bytes)
norm_view_len = len(norm_bytes)
idx_view_len = len(idx_bytes)

xs = [p[0] for p in positions]
ys = [p[1] for p in positions]
zs = [p[2] for p in positions]

gltf = {
    "asset": {"version": "2.0", "generator": "Mage mockup placeholder generator"},
    "scene": 0,
    "scenes": [{"nodes": [0]}],
    "nodes": [{"mesh": 0}],
    "meshes": [{
        "primitives": [{
            "attributes": {"POSITION": 0, "NORMAL": 1},
            "indices": 2,
            "material": 0,
        }],
    }],
    "materials": [{
        "name": "PlaceholderGrey",
        "pbrMetallicRoughness": {
            "baseColorFactor": [0.55, 0.55, 0.6, 1.0],
            "metallicFactor": 0.1,
            "roughnessFactor": 0.7,
        },
    }],
    "accessors": [
        {
            "bufferView": 0, "componentType": 5126, "count": len(positions), "type": "VEC3",
            "min": [min(xs), min(ys), min(zs)], "max": [max(xs), max(ys), max(zs)],
        },
        {"bufferView": 1, "componentType": 5126, "count": len(normals), "type": "VEC3"},
        {"bufferView": 2, "componentType": 5123, "count": len(indices), "type": "SCALAR"},
    ],
    "bufferViews": [
        {"buffer": 0, "byteOffset": 0, "byteLength": pos_view_len, "target": 34962},
        {"buffer": 0, "byteOffset": pos_view_len, "byteLength": norm_view_len, "target": 34962},
        {"buffer": 0, "byteOffset": pos_view_len + norm_view_len, "byteLength": idx_view_len, "target": 34963},
    ],
    "buffers": [{
        "byteLength": len(buffer),
        "uri": "data:application/octet-stream;base64," + base64.b64encode(buffer).decode("ascii"),
    }],
}

with open("placeholder-gem.gltf", "w", encoding="utf-8") as f:
    json.dump(gltf, f)

print(f"Ecrit : placeholder-gem.gltf ({len(FACES)} faces, {len(positions)} sommets)")
