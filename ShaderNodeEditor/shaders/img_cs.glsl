#version 430

// Define the size of the grid
layout (local_size_x = 16, local_size_y = 16) in;

// Define the layout of the buffer holding the current state of the grid
layout (rgba8, binding = 0) uniform image2D current_state;

// Define the layout of the buffer to store the next state of the grid
layout (rgba8, binding = 1) uniform image2D next_state;

// Function to count the number of living neighbors of a cell
int count_neighbors(vec2 pos) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            vec2 neighbor_pos = pos + vec2(i, j);
            vec4 neighbor = imageLoad(current_state, ivec2(neighbor_pos));
            if (neighbor.r > 0.5) {
                count++;
            }
        }
    }
    return count;
}

void main() {
    // Get the position of the current thread
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

    // Get the current state of the cell
    vec4 current = imageLoad(current_state, pos);

    // Count the number of living neighbors
    int num_neighbors = count_neighbors(vec2(pos));

    // Apply the Game of Life rules
    vec4 next;
    if (current.r > 0.5 && (num_neighbors < 2 || num_neighbors > 3)) {
        // Cell dies from underpopulation or overpopulation
        next = vec4(0.0, 0.0, 0.0, 1.0);
    } else if (current.r < 0.5 && num_neighbors == 3) {
        // Cell is born
        next = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        // Cell stays the same
        next = current;
    }

    // Write the next state to the buffer
    imageStore(next_state, pos, next);
}
