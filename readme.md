## Important 

Read the **MAZE_DOCUMENT.txt** before proceeding for better understanding

Choose from the maze images located in the following path.

   **MAZE_IMAGES_PATH = ./MazeSolver/Maze/maze_images**

## Getting Started

1. Open a new Command Line Interface (CLI).
2. Make a new directory named **_MazeSolver_** using the following command.
```
mkdir MazeSolver
```
3. Navigate to the newly created **_MazeSolver_** directory using the following command. 
```
cd MazeSolver
```
4. Clone the Github Repository for **_MazeSolver_** using the following command.
```
git clone https://github.com/kv4s4n/MazeSolver.git
```


## Serial Maze Solver

1. To run the serial maze solver program, go to the directory and enter the following command.
```
g++ serial_maze_solver.cpp -o sms
```

2. A binary named _sms_ will be created. To run the binary, enter the following command. 
```
./sms
```

3. In the prompt asking for the name of the maze, type in the name of the maze you want to solve. Eg :- To solve for _perfect2k.png_, type **perfect2k** without any extensions. The solution will be saved as a text file in the following path.

   **MAZE_SERIAL_TEXT_FILE_PATH = ./MazeSolver/Serial/maze_serial_text_files**

4. To convert the text file to an image, run the following command.
```
python3 maze_to_image_serial.py
```

5. Enter the name **perfect2k** in the prompt without any extension.

6. The final solution image will be saved as a png file in the following path

    **MAZE_SERIAL_IMAGE_SOLUTION_PATH = ./MazeSolver/Serial/maze_serial_images**


## Parallel Maze Solver

**Note :-** To change the number of threads, open the following program in your text editor and change the **const int NUM_THREADS** to another number. Default is 8.

1. To run the parallel maze solver program, go to the directory and enter the following command.
```
g++ -fopenmp parallel_maze_solver.cpp -o pms
```

2. A binary named _pms_ will be created. To run the binary, enter the following command.
```
./pms
```

3. In the prompt asking for the name of the maze, type in the name of the maze you want to solve. Eg :- To solve for _perfect2k.png_, type **perfect2k** without any extensions. The solution will be saved as a text file in the following path.

    **MAZE_PARALLEL_TEXT_FILE_PATH = ./MazeSolver/Parallel/maze_parallel_text_files**

4. To convert the text file to an image, run the following command. 
```
python3 maze_to_image_parallel.py
```

5. Enter the name **perfect2k** in the prompt without any extension.

6. The final solution image will be saved as a png file in the following path

    **MAZE_PARALLEL_IMAGE_SOLUTOIN_PATH = ./MazeSolver/Parallel/maze_parallel_images**


## Note :- 

Due to GitHub file size limits **_combo6k.png_** and **_perfect4k.png_** does not have an equivalent text file. 

1. To create the text file, go to the **Maze** directory using the following command.
```
cd ./MazeSolver/Maze
```

2. Run the python program that converts maze image to text file using the following command.
```
python3 image_to_maze.py
```

3. Enter the name of the maze **combo6k** or **perfect4k** without any extension.