#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>
#include <omp.h>
#include <fstream>
#include <queue>
using namespace std;

const int NUM_THREADS = 10; // Number of threads used in OpenMP Program

// ### TTME CONSTANTS ###

double parallel_create_nodes_time;       // Time taken to create nodes
double parallel_connect_horizontal_time; // Time taken to connect the horizontal nodes
double parallel_connect_vertical_time;   // Time taken to connect the vertical nodes

double serial_time_dfs; // Time taken to run the initial serial depth-first search for (8 * NUM_THREADS) nodes
double serial_time_bfs; // Time taken to run the initial serial breadth-first search for (8 * NUM_THREADS) nodes

double total_time_dfs; // Time taken to run the total depth-first search
double total_time_bfs; // Time taken to run the total breadth-first search

string input_path = "./Maze/maze_text_files/";
string output_path = "./Parallel/maze_parallel_text_files/";

struct node
{
    // A Node struct that is stored in memory

    int position[2];
    vector<node> neighbours;
    bool end = false;
    bool visited = false;
    struct node *parent = NULL;
};

struct placeholder
{
    // A placeholder struct for vector
    bool value = false;
    struct node *node;
};

int node_count = 0;      // Number of nodes in graph
int path_length = 0;     // Length of path found by the algorithms
int nodes_traversed = 1; // Number of nodes in the path found
int color_num = 2;       // Used to change the color of the solution
bool answered = false;

struct node *start_node; // Pointer to start node
struct node *end_node;   // Pointer to end node

void read_maze(vector<vector<int>> &maze)
{
    /*
    Reading the maze file and storing it as ones and zeros with
        1 -> Path
        0 -> Wall
    inside a 2D vector named maze.
    */

    ifstream file(input_path);
    if (!file.is_open())
        return;

    vector<int> my_vector;
    int num;

    while (file >> num)
    {
        my_vector.emplace_back(num);
    }

    int num_of_elements = my_vector.size();
    int length = sqrt(num_of_elements);

    for (int i = 0; i < length; i++)
    {
        vector<int> row;
        for (int j = 0; j < length; j++)
        {
            int element = my_vector[(i * length) + j];
            row.emplace_back(element);
        }
        maze.emplace_back(row);
    }
}

void parallel_create_nodes(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector, int height, int width)
// Create the nodes of a maze.
{
    double start_time = omp_get_wtime();
#pragma omp parallel shared(maze, node_vector, height, width) num_threads(NUM_THREADS)
    {
        int i, j;
#pragma omp for
        for (int j = 0; j < width; j++)
        { // Create the Start Node
            if (maze[0][j] == 1)
            {
                node *new_node = new node; // Creating a new node
                new_node->position[0] = 0; // Filling position
                new_node->position[1] = j;
                new_node->parent = NULL;
                start_node = new_node; // Pointing the start node
                node_count += 1;

                placeholder dummy = {true, new_node}; // Placing the start node in the node vector using dummy
                node_vector[0][j] = dummy;
            }

            // Create the End Node

            else if (maze[height - 1][j] == 1)
            {                                       // Create the End Node
                node *new_node = new node;          // Creating a new node
                new_node->position[0] = height - 1; // Filling position
                new_node->position[1] = j;
                new_node->parent = NULL;
                new_node->end = true; // Marking the end node
                end_node = new_node;  // Pointing the end node
                node_count += 1;

                placeholder dummy = {true, new_node}; // Placing the end node in the node vector using dummy
                node_vector[height - 1][j] = dummy;
            }
        }
#pragma omp for private(i, j) collapse(2)
        for (i = 1; i < height - 1; i++)
        {
            for (j = 1; j < width - 1; j++)
            {
                if (maze[i][j] == 1)
                {
                    // Looking at the neighbour colors

                    int up = maze[i - 1][j];
                    int down = maze[i + 1][j];
                    int left = maze[i][j - 1];
                    int right = maze[i][j + 1];
                    int count = 0;

                    if (up == 1)
                        count += 1;
                    if (down == 1)
                        count += 1;
                    if (left == 1)
                        count += 1;
                    if (right == 1)
                        count += 1;

                    if (count <= 1 || count >= 3)
                    { // Create a node for corner points or T juntions
#pragma omp critical
                        {
                            node *new_node = new node;
                            new_node->position[0] = i;
                            new_node->position[1] = j;
                            new_node->parent = NULL;
                            node_count += 1;

                            placeholder dummy = {true, new_node};
                            node_vector[i][j] = dummy;
                        }
                    }

                    else if (count == 2)
                    {
                        if ((up == 1 && down == 1) || (left == 1 && right == 1))
                        {
                        } // Along a path, so no need to do anything
                        else
                        {
#pragma omp critical
                            {
                                node *new_node = new node;
                                new_node->position[0] = i;
                                new_node->position[1] = j;
                                new_node->parent = NULL;
                                node_count += 1;

                                placeholder dummy = {true, new_node};
                                node_vector[i][j] = dummy;
                            }
                        }
                    }
                }
            }
        }
    }
    double end_time = omp_get_wtime();
    parallel_create_nodes_time = (end_time - start_time) * 1000.0;
}

void parallel_connect_horizontal(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector, int height, int width)
// Connect all the nodes horizontally
{
    double start_time = omp_get_wtime();
#pragma omp parallel shared(maze, node_vector, height, width) num_threads(NUM_THREADS)
    {
        int i, j;
#pragma omp for private(i, j)
        for (i = 1; i < height - 1; i++)
        {
            bool prev = false;
            node *prev_node;

            for (j = 1; j < width - 1; j++)
            {
                placeholder dummy = node_vector[i][j];

                if (maze[i][j] == 0)
                // If a wall pixel is found, reset previous node to null
                {
                    prev = false;
                    prev_node = NULL;
                }

                else if ((maze[i][j] == 1) && (dummy.value == true))
                {
                    if (prev == false)
                    // If the first node is found along the traversal, track this node
                    {
                        prev = true;
                        prev_node = dummy.node;
                    }

                    else if (prev == true)
                    {
                        // Connect the previsouly tracked node to this node and track this node
                        prev_node->neighbours.emplace_back(*dummy.node);
                        dummy.node->neighbours.emplace_back(*prev_node);
                        prev_node = dummy.node;
                    }
                }
            }
        }
    }

    double end_time = omp_get_wtime();
    parallel_connect_horizontal_time = (end_time - start_time) * 1000.0;
}

void parallel_connect_vertical(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector, int height, int width)
// Connect all the nodes vertically
{
    double start_time = omp_get_wtime();
#pragma omp parallel shared(maze, node_vector, height, width) num_threads(NUM_THREADS)
    {
        int i, j;
#pragma omp for private(i, j)
        for (j = 1; j < width - 1; j++)
        {
            bool prev = false;
            struct node *prev_node;

            for (i = 0; i < height; i++)
            {
                placeholder dummy = node_vector[i][j];

                if (maze[i][j] == 0)
                // If a wall pixel is found, reset previous node to null
                {
                    prev = false;
                    prev_node = NULL;
                }

                else if ((maze[i][j] == 1) && (dummy.value == true))
                {
                    // If this is the first node in the traversal, track it.
                    if (prev == false)
                    {
                        prev = true;
                        prev_node = dummy.node;
                    }

                    else
                    // Connect the previously tracked node to this node and track this node.
                    {
                        prev_node->neighbours.emplace_back(*dummy.node);
                        dummy.node->neighbours.emplace_back(*prev_node);
                        prev_node = dummy.node;
                    }
                }
            }
        }
    }
    double end_time = omp_get_wtime();
    parallel_connect_vertical_time = (end_time - start_time) * 1000.0;
}

void print_maze(vector<vector<int>> &maze)
{
    // Debug function to check the solution
    int height = maze.size();
    int width = maze[0].size();

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            cout << maze[i][j] << " ";
        }
        cout << endl;
    }
}

void save_maze(vector<vector<int>> &maze)
{
    // Save function to save the maze as a text file.
    ofstream out(output_path);
    int length = maze.size();
    int width = maze[0].size();

    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < width; j++)
        {
            out << maze[i][j] << " ";
        }
        out << "\n";
    }

    out.close();
}

void prepare_strings(string name)
{
    // Function to create the path and filename required to read and write the maze.
    string input_last = ".txt";
    input_path = input_path + name + input_last;

    string output_last = "_parallel_solved.txt";
    output_path = output_path + name + output_last;
}

void reset_nodes(vector<vector<placeholder>> &node_vector)
{
    /*
    Uses the depth first search functionality in reverse without the end
    node feature, to change the value of visited from true to false for
    each node.
    */

    vector<node> stack;
    stack.emplace_back(*start_node);
    start_node->visited = false;

    while (stack.size() != 0)
    {
        node element_node = stack.back();
        stack.pop_back();

        int row_elem_pos = element_node.position[0];
        int col_elem_pos = element_node.position[1];

        placeholder actual_element = node_vector[row_elem_pos][col_elem_pos];

        int neighbour_size = actual_element.node->neighbours.size();

        for (int i = 0; i < neighbour_size; i++)
        {
            node neighbour = actual_element.node->neighbours[i];
            int row_neigh_pos = neighbour.position[0];
            int col_neigh_pos = neighbour.position[1];

            placeholder actual_neighbour = node_vector[row_neigh_pos][col_neigh_pos];

            if (((row_elem_pos != row_neigh_pos) || (col_elem_pos != col_neigh_pos)) && actual_neighbour.node->visited == true)
            {
                actual_neighbour.node->parent = NULL;
                actual_neighbour.node->visited = false;
                stack.emplace_back(neighbour);
            }
        }
    }

    path_length = 0;
    nodes_traversed = 1;
    answered = false;
}

void initialise_graph(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector)
{
    // Utility function to create a graph using the 3 functions below
    int height = maze.size();
    int width = maze[0].size();

    parallel_create_nodes(maze, node_vector, height, width);
    parallel_connect_horizontal(maze, node_vector, height, width);
    parallel_connect_vertical(maze, node_vector, height, width);
}

void depth_first_search(vector<vector<placeholder>> &node_vector, vector<node> &stack)
{
    // Serial depth first search

    // This function is used to solve small mazes, and is used to populate the stack for the parallel depth first search

    stack.emplace_back(*start_node); // Placing the start node on the stack
    start_node->visited = true;      // Marking the start node as visited

    while (stack.size() != 0)
    {
        node element_node = stack.back();
        stack.pop_back(); // Removing from stack

        int row_elem_pos = element_node.position[0];
        int col_elem_pos = element_node.position[1];

        placeholder actual_element = node_vector[row_elem_pos][col_elem_pos]; // Pointing to the actual node in maze

        int neighbour_size = actual_element.node->neighbours.size();

        for (int i = 0; i < neighbour_size; i++) // Traversing through neighbours
        {
            node neighbour = actual_element.node->neighbours[i];

            int row_neigh_pos = neighbour.position[0];
            int col_neigh_pos = neighbour.position[1];

            placeholder actual_neighbour = node_vector[row_neigh_pos][col_neigh_pos]; // Pointing actual neighbours

            if (((row_elem_pos != row_neigh_pos) || (col_elem_pos != col_neigh_pos)) && actual_neighbour.node->visited == false)
            {
                actual_neighbour.node->parent = actual_element.node;
                actual_neighbour.node->visited = true;
                stack.emplace_back(neighbour);

                if (actual_neighbour.node->end == true)
                {
                    stack.clear();
                    answered = true;
                    break;
                }
            }
        }

        if (stack.size() >= 8 * NUM_THREADS)
        {
            break; // Each thread will get atleast 8 nodes to iterate through
        }
    }
}

void parallel_depth_first_search(vector<vector<placeholder>> &node_vector, vector<node> &stack)
{
    bool terminated = false;                          // If solution is found, a shared variable
    int num_of_elements = stack.size() / NUM_THREADS; // Number of elements each thread will get
    vector<node> thread_stack;                        // This private stack is used by threads to iterate through nodes
    vector<node> work_balance;                        // This public stack supplies nodes to free threads for work balancing and is populated by other full nodes.

    double start_time = omp_get_wtime();

#pragma omp parallel shared(node_vector, stack, terminated, num_of_elements, work_balance) private(thread_stack) num_threads(NUM_THREADS)
    {
        int thread_num = omp_get_thread_num(); // Get the individual thread number

        for (int i = 0; i < num_of_elements; i++)
        {
            int index = thread_num + (i * NUM_THREADS);
            thread_stack.emplace_back(stack[index]); // Place the elements in the private thread stack.
        }

#pragma omp master
        {
            if (stack.size() % NUM_THREADS != 0)
            {
                int last_index = num_of_elements * NUM_THREADS;

                for (int j = last_index; j < stack.size(); j++)
                {
                    thread_stack.emplace_back(stack[j]); // Place the remaining elements in the master thread stack
                }
            }
        }

        while (terminated == false) // Till end node is not reached
        {
            if (thread_stack.size() == 0)
            // If thread is free, takes nodes from the non-empty shared work balance vector
            {
#pragma omp critical // Makes sure that multiple threads do not access the same node.
                {
                    if (work_balance.size() != 0)
                    {
                        node element_node = work_balance.back();
                        work_balance.pop_back();
                        thread_stack.emplace_back(element_node);
                    }
                }
            }
            while (thread_stack.size() != 0)
            {
                node element_node = thread_stack.back(); // Taking the element from stack
                thread_stack.pop_back();

                int row_elem_pos = element_node.position[0];
                int col_elem_pos = element_node.position[1];

                placeholder actual_element = node_vector[row_elem_pos][col_elem_pos];

                int neighbour_size = actual_element.node->neighbours.size();

                for (int i = 0; i < neighbour_size; i++)
                {
                    node neighbour = actual_element.node->neighbours[i];

                    int row_neigh_pos = neighbour.position[0];
                    int col_neigh_pos = neighbour.position[1];

                    placeholder actual_neighbour = node_vector[row_neigh_pos][col_neigh_pos];

                    if (((row_elem_pos != row_neigh_pos) || (col_elem_pos != col_neigh_pos)) && actual_neighbour.node->visited == false)
                    // The race condition incurred by updation of node's parent and visited, by multiple threads is benign.
                    {
                        actual_neighbour.node->parent = actual_element.node;
                        actual_neighbour.node->visited = true;

                        if (work_balance.size() <= 2 * NUM_THREADS && thread_stack.size() >= 8)
                        // If work balance vector has too few and the private stack has too many, then the node is sent to the shared work balance vector
                        {
#pragma omp critical
                            work_balance.emplace_back(neighbour);
                        }

                        else
                        // Move the node to the thread's private stack
                        {
                            thread_stack.emplace_back(neighbour);
                        }

                        if (actual_neighbour.node->end == true)
                        // If end node is found, clear the thread stack and change the boolean value of terminated to true
                        {
                            thread_stack.clear();
                            terminated = true;
#pragma omp flush // Update this value across all threads, so, they can stop working
                            break;
                        }
                    }
                }
            }
        }
    }

    double end_time = omp_get_wtime();
    double elapsed_time = ((end_time - start_time) * 1000.0) + serial_time_dfs;
    total_time_dfs = elapsed_time;
}

void breadth_first_search(vector<vector<placeholder>> &node_vector, vector<node> &stack)
{
    // Serial Breadth First Search
    // This function is used to solve small mazes, and to populate the stack with nodes that the parallel implementation will use.

    queue<node> node_queue;       // Initialising the queue
    node_queue.push(*start_node); // Pushing the start node to the queue
    start_node->visited = true;   // Marking the start node as visited

    while (node_queue.size() != 0)
    {
        node element_node = node_queue.front(); // Removing the node from queue
        node_queue.pop();

        int row_elem_pos = element_node.position[0];
        int col_elem_pos = element_node.position[1];

        placeholder actual_element = node_vector[row_elem_pos][col_elem_pos]; // Pointing towards the actual node

        int neighbour_size = actual_element.node->neighbours.size();

        for (int i = 0; i < neighbour_size; i++)
        {
            node neighbour = actual_element.node->neighbours[i];

            int row_neigh_pos = neighbour.position[0];
            int col_neigh_pos = neighbour.position[1];

            placeholder actual_neighbour = node_vector[row_neigh_pos][col_neigh_pos];

            if (((row_elem_pos != row_neigh_pos) || (col_elem_pos != col_neigh_pos)) && actual_neighbour.node->visited == false)
            // Processing the neighbour nodes and adding them to the queue.
            {
                actual_neighbour.node->parent = actual_element.node;
                actual_neighbour.node->visited = true;
                node_queue.push(neighbour);

                if (actual_neighbour.node->end == true)
                // End node is found
                {
                    node_queue = queue<node>();
                    break;
                }
            }
        }

        if (node_queue.size() >= (8 * NUM_THREADS))
        // Each thread has atleast 8 nodes to traverse through
        {
            break;
        }
    }

    // Transferring the nodes from node queue to the vector
    while (node_queue.size() != 0)
    {
        node element_node = node_queue.front();
        node_queue.pop();

        stack.emplace_back(element_node);
    }
}

void parallel_breadth_first_search(vector<vector<placeholder>> &node_vector, vector<node> &stack)
{
    bool terminated = false;                          // If end node is not found
    int num_of_elements = stack.size() / NUM_THREADS; // Number of elements each thread will get
    queue<node> thread_queue;                         // Each thread's private queue for node traversing
    vector<node> work_balance;                        // Shared vector used by free threads to populate their private queues.

    double start_time = omp_get_wtime();

#pragma omp parallel shared(node_vector, stack, terminated, num_of_elements, work_balance) private(thread_queue) num_threads(NUM_THREADS)
    {
        int thread_num = omp_get_thread_num(); // Get the thread's number

        for (int i = 0; i < num_of_elements; i++)
        {
            int index = thread_num + (i * NUM_THREADS);
            thread_queue.push(stack[index]); // Adding the nodes to the threads' private queues
        }

#pragma omp master
        {
            if (stack.size() % NUM_THREADS != 0)
            {
                int last_index = num_of_elements * NUM_THREADS;

                for (int j = last_index; j < stack.size(); j++)
                {
                    thread_queue.push(stack[j]); // Adding the remaining nodes to the master thread's private queue.
                }
            }
        }

        while (terminated == false) // End node is not found
        {
            if (thread_queue.size() == 0) // Free threads can get nodes from the non-empty work balance vector
            {
#pragma omp critical // Makes sure that multiple threads do not get the same node
                {
                    if (work_balance.size() != 0)
                    {
                        node element_node = work_balance.back();
                        work_balance.pop_back();
                        thread_queue.push(element_node);
                    }
                }
            }
            while (thread_queue.size() != 0) // While private queues are populated
            {
                node element_node = thread_queue.front();
                thread_queue.pop(); // Removing the node from the queue for processing

                int row_elem_pos = element_node.position[0];
                int col_elem_pos = element_node.position[1];

                placeholder actual_element = node_vector[row_elem_pos][col_elem_pos];

                int neighbour_size = actual_element.node->neighbours.size();

                for (int i = 0; i < neighbour_size; i++) // Traversing through the node's neighbours
                {
                    node neighbour = actual_element.node->neighbours[i];

                    int row_neigh_pos = neighbour.position[0];
                    int col_neigh_pos = neighbour.position[1];

                    placeholder actual_neighbour = node_vector[row_neigh_pos][col_neigh_pos];

                    if (((row_elem_pos != row_neigh_pos) || (col_elem_pos != col_neigh_pos)) && actual_neighbour.node->visited == false)
                    // The race condition incurred by multiple threads trying to update the same node's parent and visited is benign.
                    {
                        actual_neighbour.node->parent = actual_element.node;
                        actual_neighbour.node->visited = true;

                        if (work_balance.size() <= 2 * NUM_THREADS && thread_queue.size() >= 8)
                        // Sending to node to work balance if thread has too many and work balance has too few nodes.
                        {
#pragma omp critical
                            work_balance.emplace_back(neighbour);
                        }

                        else
                        // Else push to the thread's private queue
                        {
                            thread_queue.push(neighbour);
                        }

                        if (actual_neighbour.node->end == true)
                        // End node is found
                        {
                            thread_queue = queue<node>(); // Clear the queue
                            terminated = true;            // Update the value of the boolean variable to true
#pragma omp flush                                         // Makes sure that variables have this boolean variable's value updated
                            break;
                        }
                    }
                }
            }
        }
    }

    double end_time = omp_get_wtime();
    double elapsed_time = ((end_time - start_time) * 1000.0) + serial_time_bfs;
    total_time_bfs = elapsed_time;
}

void color_segment(vector<vector<int>> &maze, int i1, int j1, int i2, int j2)
{
    // This function is used to color the segment between 2 nodes in the maze

    int distance = abs(i2 - i1) + abs(j2 - j1);
    path_length += distance;

    if (i1 == i2)
    {
        if (j2 < j1)
        {
            for (int j = j2; j <= j1; j++)
            {
                maze[i1][j] = color_num;
            }
        }

        else
        {
            for (int j = j1; j <= j2; j++)
            {
                maze[i1][j] = color_num;
            }
        }
    }

    else if (j1 == j2)
    {
        if (i2 < i1)
        {
            for (int i = i2; i <= i1; i++)
            {
                maze[i][j1] = color_num;
            }
        }

        else
        {
            for (int i = i1; i <= i2; i++)
            {
                maze[i][j1] = color_num;
            }
        }
    }
}

void color_graph(vector<vector<int>> &maze)
{
    // Colors the maze path by traversing the parents from the end node to the start node that is orphan.
    node *curr = end_node;

    while (curr->parent != NULL)
    {
        node *parent = curr->parent;
        int i1 = curr->position[0];
        int j1 = curr->position[1];
        int i2 = parent->position[0];
        int j2 = parent->position[1];

        color_segment(maze, i1, j1, i2, j2); // Using the color segment functionality
        nodes_traversed += 1;
        curr = curr->parent;
    }

    color_num += 1;
}

int main()
{

    string name;
    cout << "Enter name of the file without .txt extension : ";
    getline(cin, name);

    prepare_strings(name);
    vector<vector<int>> maze;
    read_maze(maze);

    int length = maze.size();
    placeholder dummy = {false, NULL};
    vector<placeholder> row(length, dummy);
    vector<vector<placeholder>> node_vector(length, row);

    initialise_graph(maze, node_vector);
    double total_time = parallel_create_nodes_time + parallel_connect_horizontal_time + parallel_connect_vertical_time;

    cout << "PARALLEL MAZE SOLVER \n"
         << endl;
    cout << "Number of Threads = " << NUM_THREADS << endl;
    cout << " " << endl;
    cout << "Time taken to initialise graph is " << total_time << " milliseconds" << endl;
    cout << "Number of nodes in graph = " << node_count << endl;
    cout << " " << endl;

    vector<node> stack;

    auto start_dfs = chrono::high_resolution_clock::now();
    depth_first_search(node_vector, stack);
    auto end_dfs = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> elapsed_time_dfs = end_dfs - start_dfs;
    serial_time_dfs = elapsed_time_dfs.count();

    if (answered != true)
    {
        parallel_depth_first_search(node_vector, stack);
    }

    color_graph(maze);

    cout << "DEPTH FIRST SEARCH \n"
         << endl;
    cout << "Time taken : " << total_time_dfs << " milliseconds" << endl;
    cout << "Path length : " << path_length << endl;
    cout << "Number of nodes traversed : " << nodes_traversed << endl;
    cout << " " << endl;

    vector<node> new_stack;
    vector<vector<int>> new_maze;

    placeholder new_dummy = {false, NULL};
    vector<placeholder> new_row(length, new_dummy);
    vector<vector<placeholder>> new_node_vector(length, new_row);

    read_maze(new_maze);
    initialise_graph(new_maze, new_node_vector);

    nodes_traversed = 1;
    path_length = 0;

    auto start_bfs = chrono::high_resolution_clock::now();
    breadth_first_search(new_node_vector, new_stack);
    auto end_bfs = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> elapsed_time_bfs = end_bfs - start_bfs;
    serial_time_bfs = elapsed_time_bfs.count();

    if (answered != true)
    {
        parallel_breadth_first_search(new_node_vector, new_stack);
    }

    color_graph(maze);

    cout << "BREADTH FIRST SEARCH \n"
         << endl;
    cout << "Time taken : " << total_time_bfs << " milliseconds" << endl;
    cout << "Path length : " << path_length << endl;
    cout << "Number of nodes traversed : " << nodes_traversed << endl;

    save_maze(maze);

    return 0;
}