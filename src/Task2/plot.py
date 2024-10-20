import numpy as np
import tqdm
import numba as nb
import multiprocessing
from multiprocessing import shared_memory
import cv2

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

    return b, g, r


def get_color(z):
    if z == 0:
        return 0, 0, 0
    elif z > 0.97:
        return 255, 255, 255
    else:
        return tuple(int(i * 255) for i in hsv_to_rgb(z))


def init(lock_):
    global lock
    lock = lock_


def plot(args):
    thread, ppx, ppy, shared_name = args

    shm = shared_memory.SharedMemory(name=shared_name)
    arr = np.ndarray((ppy, ppx, 3), dtype=np.uint8, buffer=shm.buf)

    with open(f'{BASE_PATH}/task2_coords_{thread}.csv') as file:
        points = int(file.readline())

        with lock:
            progress_bar = tqdm.tqdm(desc=f'Process: {thread}', leave=False, total=points, position=thread)

        i = 0
        for line in file:
            x, y, color = line.split(';')
            arr[int(y), int(x)] = get_color(float(color))

            i += 1
            if i % 1000 == 0:
                with lock:
                    progress_bar.update(1000)

        with lock:
            progress_bar.close()

    shm.close()


def main():
    with open(f'{BASE_PATH}/task2_coords_info.csv') as file:
        num_points, *size, threads = map(int, file.readline().strip().split(';'))

    shape = (*(size[::-1]), 3)

    shm = shared_memory.SharedMemory(create=True, size=int(np.prod(shape)) * np.dtype(np.uint8).itemsize)
    image_np = np.ndarray(shape, dtype=np.uint8, buffer=shm.buf)
    lock = multiprocessing.Lock()

    with multiprocessing.Pool(processes=threads, initargs=(lock,), initializer=init) as pool:
        pool.map(plot, [(i, *size, shm.name) for i in range(threads)])

    cv2.imwrite('png.png', image_np)

    shm.close()
    shm.unlink()


if __name__ == '__main__':
    main()
