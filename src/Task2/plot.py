from PIL import Image
import numpy as np
from tqdm import tqdm

colors = {0: (0, 0, 0), 1: (0, 0, 102), 2: (0, 0, 255), 3: (0, 255, 255), 4: (0, 255, 0), 5: (255, 255, 255)}


def get_color(z):
    return colors[(z * 1000 + 35) // 200]

def plot(img, arr):
    for row in tqdm(arr):
        img.putpixel((int(row[0]), int(row[1])), get_color(row[2]))

    return img


def main():
    with open('../../cmake-build-release/task2_coords.csv') as file:
        n, *size = list(map(int, file.readline().strip().split(';')))

    arr = np.loadtxt('../../cmake-build-release/task2_coords.csv', skiprows=1, delimiter=';', dtype=float)

    image = Image.new('RGB', size, (255, 255, 255))

    image = plot(image, arr)

    image.save('png.png')
    image.show()


if __name__ == '__main__':
    main()
