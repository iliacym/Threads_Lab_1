from PIL import Image
import numpy as np
import tqdm
import numba as nb
import multiprocessing

BASE_PATH = '../../cmake-build-release/'


@nb.njit(fastmath=True)
def hsv_to_rgb(h):
    h = h * 6
    i = int(h) % 6
    f = h - i
    p = 0.4
    v = 1
    q = (1 - f * 0.6)
    t = (1 - (1 - f) * 0.6)

    if i == 0:
        r, g, b = v, t, p
    elif i == 1:
        r, g, b = q, v, p
    elif i == 2:
        r, g, b = p, v, t
    elif i == 3:
        r, g, b = p, q, v
    elif i == 4:
        r, g, b = t, p, v
    else:
        r, g, b = v, p, q

    return r, g, b



def get_color(z):
    if z == 0:
        return 0, 0, 0
    elif z > 0.97:
        return 255, 255, 255
    else:
        return tuple(int(i * 255) for i in hsv_to_rgb(z))


def plot(args) -> np.ndarray:
    thread, ppx, ppy = args
    arr = np.zeros((ppy, ppx, 3), dtype=np.uint8)

    with open(f'{BASE_PATH}/task2_coords_{thread}.csv') as file:
        points = int(file.readline())

        for line in tqdm.tqdm(file, leave=False, total=points, position=thread):
            x, y, color = line.split(';')

            arr[int(y), int(x)] = get_color(float(color))

    return arr


def main():
    with open(f'{BASE_PATH}/task2_coords_info.csv') as file:
        num_points, *size, threads = map(int, file.readline().strip().split(';'))

    with multiprocessing.Pool(processes=threads) as pool:
        results = pool.map(plot, [(i, *size) for i in range(threads)])

    image_np = sum(results)

    image = Image.fromarray(image_np, 'RGB')

    image.save('png.png')
    image.show()


if __name__ == '__main__':
    main()
