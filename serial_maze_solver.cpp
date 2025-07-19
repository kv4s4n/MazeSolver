#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
using namespace std;
#include <vector>
#include <queue>

string input_path = "./Maze/maze_text_files/";
string output_path = "./Serial/maze_serial_text_files/";

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
    // A placeholder struct
    bool value = false;
    struct node *node;
};

int node_count = 0;
int path_length = 0;
int nodes_traversed = 1;
int color_num = 2;
int max_len = 0;

struct node *start_node;
struct node *end_node;

void read_maze(vector<vector<int>> &maze)
{
    // Reading the maze file

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

void create_nodes(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector)
// Create the nodes of a maze.
{
    int height = maze.size();
    int width = maze[0].size();

    for (int j = 0; j < width; j++)
    { // Create the Start Node
        if (maze[0][j] == 1)
        {
            node *new_node = new node;
            new_node->position[0] = 0;
            new_node->position[1] = j;
            new_node->parent = NULL;
            start_node = new_node;
            node_count += 1;

            placeholder dummy = {true, new_node}; // Placing the start node in the node vector
            node_vector[0][j] = dummy;
        }

        else if (maze[height - 1][j] == 1)
        { // Create the End Node
            node *new_node = new node;
            new_node->position[0] = height - 1;
            new_node->position[1] = j;
            new_node->parent = NULL;
            new_node->end = true;
            end_node = new_node;
            node_count += 1;

            placeholder dummy = {true, new_node}; // Placing the end node in the node vector
            node_vector[height - 1][j] = dummy;
        }
    }

    for (int i = 1; i < height - 1; i++)
    {
        for (int j = 1; j < width - 1; j++)
        {
            if (maze[i][j] == 1)
            {
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
                    node *new_node = new node;
                    new_node->position[0] = i;
                    new_node->position[1] = j;
                    new_node->parent = NULL;
                    node_count += 1;

                    placeholder dummy = {true, new_node};
                    node_vector[i][j] = dummy;
                }

                else if (count == 2)
                {
                    if ((up == 1 && down == 1) || (left == 1 && right == 1))
                    {
                    } // Along a path, so no need to do anything
                    else
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

void connect_horizontal(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector)
// Connect all the nodes horizontally
{
    int height = node_vector.size();
    int width = node_vector[0].size();

    for (int i = 1; i < height - 1; i++)
    {
        bool prev = false;
        node *prev_node;

        for (int j = 1; j < width - 1; j++)
        {
            placeholder dummy = node_vector[i][j];

            if (maze[i][j] == 0)
            {
                prev = false;
                prev_node = NULL;
            }

            else if ((maze[i][j] == 1) && (dummy.value == true))
            {
                if (prev == false)
                {
                    prev = true;
                    prev_node = dummy.node;
                }

                else if (prev == true)
                {
                    prev_node->neighbours.emplace_back(*dummy.node);
                    dummy.node->neighbours.emplace_back(*prev_node);
                    prev_node = dummy.node;
                }
            }
        }
    }
}

void connect_vertical(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector)
// Connect all the nodes vertically
{
    int height = node_vector.size();
    int width = node_vector[0].size();

    for (int j = 1; j < width - 1; j++)
    {
        bool prev = false;
        struct node *prev_node;

        for (int i = 0; i < height; i++)
        {
            placeholder dummy = node_vector[i][j];

            if (maze[i][j] == 0)
            {
                prev = false;
                prev_node = NULL;
            }

            else if ((maze[i][j] == 1) && (dummy.value == true))
            {
                if (prev == false)
                {
                    prev = true;
                    prev_node = dummy.node;
                }

                else
                {
                    prev_node->neighbours.emplace_back(*dummy.node);
                    dummy.node->neighbours.emplace_back(*prev_node);
                    prev_node = dummy.node;
                }
            }
        }
    }
}

void reset_nodes(vector<vector<placeholder>> &node_vector)
{
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
}

void depth_first_search(vector<vector<placeholder>> &node_vector)
{

    vector<node> stack;
    stack.emplace_back(*start_node);
    start_node->visited = true;

    while (stack.size() != 0)
    {
        if (stack.size() > max_len)
        {
            max_len = stack.size();
        }
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

            if (((row_elem_pos != row_neigh_pos) || (col_elem_pos != col_neigh_pos)) && actual_neighbour.node->visited == false)
            {
                actual_neighbour.node->parent = actual_element.node;
                actual_neighbour.node->visited = true;
                stack.emplace_back(neighbour);

                if (actual_neighbour.node->end == true)
                {
                    stack.clear();
                    break;
                }
            }
        }
    }
}

void breadth_first_search(vector<vector<placeholder>> &node_vector)
{

    queue<node> node_queue;
    node_queue.push(*start_node);
    start_node->visited = true;

    while (node_queue.size() != 0)
    {
        if (node_queue.size() > max_len)
        {
            max_len = node_queue.size();
        }

        node element_node = node_queue.front();
        node_queue.pop();

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
            {
                actual_neighbour.node->parent = actual_element.node;
                actual_neighbour.node->visited = true;
                node_queue.push(neighbour);

                if (actual_neighbour.node->end == true)
                {
                    node_queue = queue<node>();
                    break;
                }
            }
        }
    }
}

void color_segment(vector<vector<int>> &maze, int i1, int j1, int i2, int j2)
{

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
    node *curr = end_node;

    while (curr->parent != NULL)
    {
        node *parent = curr->parent;
        int i1 = curr->position[0];
        int j1 = curr->position[1];
        int i2 = parent->position[0];
        int j2 = parent->position[1];

        color_segment(maze, i1, j1, i2, j2);
        nodes_traversed += 1;
        curr = curr->parent;
    }

    color_num += 1;
}

void initialise_graph(vector<vector<int>> &maze, vector<vector<placeholder>> &node_vector)
{
    create_nodes(maze, node_vector);
    connect_horizontal(maze, node_vector);
    connect_vertical(maze, node_vector);
}

void print_maze(vector<vector<int>> &maze)
{
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

    string output_last = "_serial_solved.txt";
    output_path = output_path + name + output_last;
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

    auto start_graph = chrono::high_resolution_clock::now();
    initialise_graph(maze, node_vector);
    auto end_graph = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> elapsed_time_graph = end_graph - start_graph;
    cout << "SERIAL MAZE SOLVER \n"
         << endl;
    cout << "Time taken for Initialising Graph is " << elapsed_time_graph.count() << " milliseconds" << endl;
    cout << "Number of nodes in the graph : " << node_count << endl;
    cout << "\n";

    if (start_node->visited == true)
    {
        reset_nodes(node_vector);
    }

    auto start_dfs = chrono::high_resolution_clock::now();
    depth_first_search(node_vector);
    auto end_dfs = chrono::high_resolution_clock::now();
    color_graph(maze);

    chrono::duration<double, std::milli> elapsed_time_dfs = end_dfs - start_dfs;

    cout << "DEPTH FIRST SEARCH"
         << "\n"
         << endl;
    cout << "Time taken : " << elapsed_time_dfs.count() << " milliseconds" << endl;
    cout << "Path length : " << path_length << endl;
    cout << "Number of nodes traversed : " << nodes_traversed << endl;
    cout << "Maximum number of nodes in stack : " << max_len << endl;
    cout << "\n";

    max_len = 0;
    if (start_node->visited == true)
    {
        reset_nodes(node_vector);
    }

    auto start_bfs = chrono::high_resolution_clock::now();
    breadth_first_search(node_vector);
    auto end_bfs = chrono::high_resolution_clock::now();
    color_graph(maze);

    chrono::duration<double, std::milli> elapsed_time_bfs = end_bfs - start_bfs;

    cout << "BREADTH FIRST SEARCH"
         << "\n"
         << endl;
    cout << "Time taken : " << elapsed_time_bfs.count() << " milliseconds" << endl;
    cout << "Path length : " << path_length << endl;
    cout << "Number of nodes traversed : " << nodes_traversed << endl;
    cout << "Maximum number of nodes in queue : " << max_len << endl;
    cout << "\n";

    save_maze(maze);
    return 0;
}