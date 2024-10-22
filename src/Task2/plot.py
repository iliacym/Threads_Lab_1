import numpy as np
import tqdm
import numba as nb
import multiprocessing
from multiprocessing import shared_memory
import cv2
import time
import threading
import ctypes


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


def init(lock_, progress_value_):
    global lock, progress_value
    lock = lock_
    progress_value = progress_value_


def plot(args):
    file_num, ppx, ppy, shared_name = args

    shm = shared_memory.SharedMemory(name=shared_name)
    arr = np.ndarray((ppy, ppx, 3), dtype=np.uint8, buffer=shm.buf)

    with open(f'{BASE_PATH}/task2_coords_{file_num}.csv') as file:
        points = int(file.readline())

        with lock:
            progress_bar = tqdm.tqdm(desc=f'File {file_num}', leave=False, total=points,
                                     position=multiprocessing.current_process()._identity[0] + 1)

        i = 0
        for line in file:
            x, y, color = line.split(';')
            arr[int(y), int(x)] = get_color(float(color))

            i += 1
            if i % 1000 == 0:
                with lock:
                    progress_bar.update(1000)
                    progress_value.value += 1000

        with lock:
            progress_bar.close()
            progress_value.value += i % 1000

    shm.close()


def print_progress(lck, value, num_points):
    with lck:
        progress_bar = tqdm.tqdm(desc=f'Progress', leave=False, total=num_points, position=0)

    val = 0
    while True:
        with lck:
            progress_bar.update(value.value - val)
            val = value.value

        time.sleep(0.01)

        if val >= num_points:
            break


def main():
    with open(f'{BASE_PATH}/task2_coords_info.csv') as file:
        num_points, *size, num_files = map(int, file.readline().strip().split(';'))

    shape = (*(size[::-1]), 3)

    shm = shared_memory.SharedMemory(create=True, size=int(np.prod(shape)) * np.dtype(np.uint8).itemsize)
    global_progress_value = multiprocessing.Value(ctypes.c_ulonglong, 0)

    image_np = np.ndarray(shape, dtype=np.uint8, buffer=shm.buf)
    lock = multiprocessing.Lock()

    t = threading.Thread(target=print_progress, args=(lock, global_progress_value, num_points))
    t.start()

    with multiprocessing.Pool(initargs=(lock, global_progress_value), initializer=init) as pool:
        pool.map(plot, [(i, *size, shm.name) for i in range(num_files)])

    t.join()

    cv2.imwrite('D:/123/png.png', image_np)

    shm.close()
    shm.unlink()


BASE_PATH = '../../cmake-build-release/results/'

if __name__ == '__main__':
    main()
