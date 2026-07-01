import sys
from PIL import Image, ImageDraw

sys.setrecursionlimit(100_000)

BASE = [(0, 0), (0, 1), (-1, 1), (-2, 1), (-2, 2),
        (-2, 3), (-1, 3), (-1, 4), (-1, 5)]


def normalize(cells):
    mnx = min(c[0] for c in cells)
    mny = min(c[1] for c in cells)
    return tuple(sorted((x - mnx, y - mny) for x, y in cells))


def orientations(cells):
    out = []
    cur = list(cells)
    for _ in range(2):
        c = cur
        for _ in range(4):
            n = normalize(c)
            if n not in out:
                out.append(n)
            c = [(y, -x) for x, y in c]
        cur = [(-x, y) for x, y in cells]
    return out


ORIENTS = orientations(BASE)


def make_variants(allowed_oi=None):
    variants = []
    for oi, o in enumerate(ORIENTS):
        if allowed_oi is not None and oi not in allowed_oi:
            continue
        for ax, ay in o:
            variants.append((oi, [(x - ax, y - ay) for x, y in o]))
    return variants


def solve_torus(W, H, S, allowed_oi=None, node_cap=3_000_000):

    variants = make_variants(allowed_oi)

    def canon(x, y):
        k = y // H
        return (x - k * S) % W, y - k * H

    grid = [-1] * (W * H)
    placements = []
    nodes = [0]

    def rec():
        nodes[0] += 1
        if nodes[0] > node_cap:
            raise TimeoutError
        i = next((j for j, v in enumerate(grid) if v < 0), -1)
        if i < 0:
            return True # torus fully covered
        ax, ay = i % W, i // W
        tid = len(placements)
        seen = set()
        for oi, cells in variants:
            idxs = []
            ok = True
            for dx, dy in cells:
                x, y = canon(ax + dx, ay + dy)
                j = y * W + x
                if grid[j] >= 0:
                    ok = False
                    break
                idxs.append(j)
            if not ok:
                continue
            key = (oi, frozenset(idxs))
            if key in seen:
                continue
            seen.add(key)
            for j in idxs:
                grid[j] = tid
            placements.append((tid, oi, ax, ay, tuple(cells)))
            if rec():
                return True
            placements.pop()
            for j in idxs:
                grid[j] = -1
        return False

    try:
        return placements if rec() else None
    except TimeoutError:
        return None


def find_tiling(mode='refl'):
    if mode == 'rot':
        allowed = set(range(4))
    elif mode == 'refl':
        allowed = set(range(4, 8))
    else:
        allowed = None

    cands = sorted((W * H, W, H)
                   for W in range(3, 15) for H in range(3, 15)
                   if (W * H) % 9 == 0)
    for _, W, H in cands:
        for S in range(W):
            sol = solve_torus(W, H, S, allowed_oi=allowed)
            if sol:
                return W, H, S, sol
    raise RuntimeError("no periodic tiling found in search range")



LIGHT = (255, 207, 156)

DARK_0 = (LIGHT[0] * 1.1, LIGHT[1] * 1.1, LIGHT[2] * 1.1)
DARK_1 = (int(LIGHT[0] / 1.00), int(LIGHT[1] / 1.00), int(LIGHT[2] / 1.00))
DARK_2 = (int(LIGHT[0] / 1.10), int(LIGHT[1] / 1.10), int(LIGHT[2] / 1.10))
DARK_3 = (int(LIGHT[0] / 1.20), int(LIGHT[1] / 1.20), int(LIGHT[2] / 1.20))
DARK_4 = (int(LIGHT[0] / 1.35), int(LIGHT[1] / 1.35), int(LIGHT[2] / 1.35))
DARK_5 = (int(LIGHT[0] / 1.50), int(LIGHT[1] / 1.50), int(LIGHT[2] / 1.50))
DARK_6 = (int(LIGHT[0] / 1.60), int(LIGHT[1] / 1.60), int(LIGHT[2] / 1.60))
DARK_7 = (int(LIGHT[0] / 1.80), int(LIGHT[1] / 1.80), int(LIGHT[2] / 1.80))
OUTLINE= (int(LIGHT[0] / 2.00), int(LIGHT[1] / 2.00), int(LIGHT[2] / 2.00))

DARK_OI = {
    'rot':  1,
    'refl': 5,
}


def render(W, H, S, sol, reps_x, reps_y, cell, fname, border=None, dark_oi=1):
    """Repeat the torus solution reps_x x reps_y times and save a PNG.
    border = boundary thickness in px (default: thin, scaled to cell size)."""
    CW, CH = W * reps_x, H * reps_y 
    inst = [[None] * CW for _ in range(CH)]
    color = {}

    for j in range(-3, reps_y + 4):
        for i in range(-5 - (reps_y * S) // W, reps_x + 5):
            ox, oy = i * W + j * S, j * H
            for tid, oi, ax, ay, cells in sol:
                iid = (tid, i, j)
                hit = False
                for dx, dy in cells:
                    px, py = ax + dx + ox, ay + dy + oy
                    if 0 <= px < CW and 0 <= py < CH:
                        inst[py][px] = iid
                        hit = True
                if hit:
                    color[iid] = {0: DARK_0, 1: DARK_1, 2: DARK_2, 3: DARK_3,
                                4: DARK_4, 5: DARK_5, 6: DARK_6, 7: DARK_7}[oi]

    assert all(inst[y][x] is not None
               for y in range(CH) for x in range(CW)), "coverage gap!"

    t = border if border is not None else 1
    lo = t // 2
    hi = t - 1 - lo
    m = t

    def px(c):
        return m + c * cell

    img = Image.new("RGB", (CW * cell + 2 * m, CH * cell + 2 * m),
                    (255, 255, 255))
    d = ImageDraw.Draw(img)
    for y in range(CH):
        for x in range(CW):
            d.rectangle([px(x), px(y), px(x + 1), px(y + 1)],
                        fill=color[inst[y][x]])

    def vseg(xe, y0, y1):
        X = px(xe)
        d.rectangle([X - lo, px(y0) - lo, X + hi, px(y1) + hi], fill=OUTLINE)

    def hseg(ye, x0, x1):
        Y = px(ye)
        d.rectangle([px(x0) - lo, Y - lo, px(x1) + hi, Y + hi], fill=OUTLINE)

    for y in range(CH):
        for x in range(CW):
            a = inst[y][x]
            if x + 1 < CW and inst[y][x + 1] != a:
                vseg(x + 1, y, y + 1)
            if y + 1 < CH and inst[y + 1][x] != a:
                hseg(y + 1, x, x + 1)
    vseg(0, 0, CH)
    vseg(CW, 0, CH)
    hseg(0, 0, CW)
    hseg(CH, 0, CW)

    img.save(fname)
    print(f"saved {fname}  {img.size[0]}x{img.size[1]} px, "
          f"{reps_x * reps_y * len(sol)} tiles, border {t}px")


if __name__ == "__main__":
    MODE = 'refl'

    print(f"Searching for {MODE} tiling...")
    W, H, S, sol = find_tiling(mode=MODE)
    print(f"Found: torus {W}x{H}, shear {S}, "
          f"{len(sol)} tiles per period, "
          f"orientations used: {sorted(set(p[1] for p in sol))}")

    dark_oi = DARK_OI.get(MODE, 1)

    # render(W, H, S, sol, 3,  2,  30, "tiling_1_small.png",  dark_oi=dark_oi)
    # render(W, H, S, sol, 6,  4,  18, "tiling_2_medium.png", dark_oi=dark_oi)
    # render(W, H, S, sol, 12, 8,  10, "tiling_3_large.png",  dark_oi=dark_oi)
    render(W, H, S, sol, 24, 16,  30, "tiling_4_huge.png",   dark_oi=dark_oi)