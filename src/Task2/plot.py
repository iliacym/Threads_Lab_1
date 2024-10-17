from PIL import Image

colors = {0: (0, 0, 0), 1: (0, 0, 102), 2: (0, 0, 255), 3: (0, 255, 255), 4: (0, 255, 0), 5: (255, 255, 255)}


def get_color(z):
    return colors[(z * 1000 + 1) // 200]


def plot_n_lines(img, f, n):
    i = 0
    w, h = img.size
    while s := f.readline():
        x, y, color = map(float, s.strip().split(';'))
        img.putpixel((int((x - screen[0]) * w / (screen[1] - screen[0])) - 1, int((y - screen[2]) * h / (screen[3] - screen[2])) - 1), get_color(color))
        i += 1
        if i == n:
            break
    return img, f


N, parts = 50000000, 5
screen = [-2, 1, -1, 1]
SIZE = (1920, 1080)

image = Image.new('RGB', SIZE, (255, 255, 255))
file = open('task2_results.csv')

for i in range(parts):
    image, file = plot_n_lines(image, file, N // parts + 1)
    print(f"{i + 1}/{parts}")
file.close()
image.show()
