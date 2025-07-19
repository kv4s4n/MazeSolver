import numpy as np
import matplotlib.pyplot as plt

input_path = "./maze_images/"
output_path = "./maze_text_files/"

name = input("Enter the name of the maze without .png extension : ")
input_path = input_path + name + ".png"
output_path = output_path + name + ".txt"

image = plt.imread(fname=input_path)
maze = np.array(image)

black = np.array([0.0, 0.0, 0.0, 1.0])
white = np.array([1.0, 1.0, 1.0, 1.0])

height, width, depth = np.shape(maze)

conv_maze = np.zeros((height, width), dtype=int)

for i in range(height):
    for j in range(width):
        if maze[i][j].all() == white.all():
            conv_maze[i][j] = 1

np.savetxt(output_path, conv_maze, fmt="%i")
