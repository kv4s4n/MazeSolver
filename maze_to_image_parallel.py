import numpy as np
import matplotlib.pyplot as plt


name = input("Enter the name of the file without .txt (or) .png extension : ")

input_path = "./Parallel/maze_parallel_text_files/"
input_path = input_path + name + "_parallel_solved.txt"

output_path = "./Parallel/maze_parallel_images/"
output_path = output_path + name + "_parallel_solved.png"


black = np.array([0.0, 0.0, 0.0, 1.0])
white = np.array([1.0, 1.0, 1.0, 1.0])
red = np.array([1.0, 0.0, 0.0, 1.0])
green = np.array([0.0, 1.0, 0.0, 1.0])
blue = np.array([0.0, 0.0, 1.0, 1.0])
yellow = np.array([1.0, 1.0, 0.0, 1.0])
cyan = np.array([0.0, 1.0, 1.0, 1.0])
magenta = np.array([1.0, 0.0, 1.0, 1.0])
olive = np.array([0.5, 0.5, 0.0, 1.0])
teal = np.array([0.0, 0.5, 0.5, 1.0])
navy = np.array([0.0, 0.0, 0.5, 1.0])
maroon = np.array([0.5, 0.0, 0.0, 1.0])

color_chart = {
    0: black,
    1: white,
    2: red,
    3: blue,
    4: green,
    5: yellow,
    6: cyan,
    7: magenta,
    8: olive,
    9: teal,
    10: navy,
    11: maroon,
}


maze = np.loadtxt(input_path)
height, width = np.shape(maze)
picture_of_maze = np.zeros((height, width, 4))

for i in range(height):
    for j in range(width):
        picture_of_maze[i][j] = color_chart[maze[i][j]]

plt.imshow(picture_of_maze)
plt.axis("off")
plt.show()
plt.imsave(fname=output_path, cmap="viridis", arr=picture_of_maze, format="png")
