#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef struct meta {
    FILE* file;
    int lines;
    int cols;
    int x;
    int y;
    int best;
    char empty;
    char full;
    char obstacle;
    char **grid_origin;
    int **grid_solved;
} meta_t;

int min_of_three(int one, int two, int three) {
    if (one < two)
        return one < three ? one : three;
    return two < three ? two : three;
}

/* Returns true if every character in line is a valid map character. */
bool is_valid_line(char *line, meta_t *data) {
    for (int i = 0; line[i]; i++) {
        if (line[i] != data->empty && line[i] != data->obstacle)
            return false;
    }
    return true;
}

/* Frees the first `count` rows of grid_origin, then grid_origin itself. */
void free_grid_origin(meta_t *data, int count) {
    for (int i = 0; i < count; i++)
        free(data->grid_origin[i]);
    free(data->grid_origin);
    data->grid_origin = NULL;
}

bool read_map_grid(meta_t *data) {
    size_t cap = 0;
    char *line = NULL;
    int last_len = -1;

    data->grid_origin = malloc(sizeof(char *) * (data->lines + 1));
    if (!data->grid_origin)
        return false;

    for (int counter = 0; counter < data->lines; counter++) {
        ssize_t len = getline(&line, &cap, data->file);

        /* Fewer lines in file than the header promised. */
        if (len < 0 || line == NULL) {
            free(line);
            free_grid_origin(data, counter);
            return false;
        }

        /* Every line must end with a newline. */
        if (line[len - 1] != '\n') {
            free(line);
            free_grid_origin(data, counter);
            return false;
        }

        /* Strip \r\n only — space is a valid map character. */
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';

        /* At least one cell per line. */
        if (len == 0) {
            free(line);
            free_grid_origin(data, counter);
            return false;
        }

        /* All lines must have the same length. */
        if (last_len == -1)
            last_len = (int)len;
        if ((int)len != last_len) {
            free(line);
            free_grid_origin(data, counter);
            return false;
        }

        /* Only empty/obstacle characters allowed. */
        if (!is_valid_line(line, data)) {
            free(line);
            free_grid_origin(data, counter);
            return false;
        }

        data->grid_origin[counter] = strdup(line);
        if (!data->grid_origin[counter]) {
            free(line);
            free_grid_origin(data, counter);
            return false;
        }
        free(line);
        line = NULL;
        cap = 0;
    }

    data->cols = last_len;
    return true;
}

bool read_map(meta_t *data) {
    size_t cap = 0;
    char *line = NULL;

    ssize_t len = getline(&line, &cap, data->file);
    if (len <= 0 || line == NULL)
        return (free(line), false);

    /* First character must be a digit (line count). */
    if (line[0] < '0' || line[0] > '9')
        return (free(line), false);

    data->lines = line[0] - '0';
    int index = 1;
    while (line[index] >= '0' && line[index] <= '9')
        data->lines = data->lines * 10 + (line[index++] - '0');

    /* At least one line required. */
    if (data->lines <= 0)
        return (free(line), false);

    /*
     * Exactly three printable characters must follow the number,
     * then only a newline or end-of-string — no extra characters.
     */
    if (!isprint((unsigned char)line[index])     ||
        !isprint((unsigned char)line[index + 1]) ||
        !isprint((unsigned char)line[index + 2]))
        return (free(line), false);

    data->empty    = line[index++];
    data->obstacle = line[index++];
    data->full     = line[index++];

    if (line[index] != '\n' && line[index] != '\r' && line[index] != '\0')
        return (free(line), false);

    /* All three characters must be distinct. */
    if (data->empty == data->obstacle ||
        data->empty == data->full     ||
        data->obstacle == data->full)
        return (free(line), false);

    free(line);
    return read_map_grid(data);
}

bool solve(meta_t *data) {
    data->best = -1;
    data->grid_solved = calloc(data->lines + 1, sizeof(int *));
    if (!data->grid_solved)
        return false;

    for (int i = 0; i < data->lines; i++) {
        data->grid_solved[i] = calloc(data->cols, sizeof(int));
        if (!data->grid_solved[i])
            return false;
    }

    for (int y = 0; y < data->lines; y++) {
        for (int x = 0; x < data->cols; x++) {
            if (y == 0 || x == 0) {
                data->grid_solved[y][x] = (data->grid_origin[y][x] == data->obstacle) ? 0 : 1;
                continue;
            }
            if (data->grid_origin[y][x] == data->obstacle) {
                data->grid_solved[y][x] = 0;
            } else {
                data->grid_solved[y][x] = min_of_three(
                    data->grid_solved[y - 1][x],
                    data->grid_solved[y][x - 1],
                    data->grid_solved[y - 1][x - 1]
                ) + 1;
            }
            if (data->grid_solved[y][x] > data->best) {
                data->best = data->grid_solved[y][x];
                data->x = x;
                data->y = y;
            }
        }
    }

    if (data->best >= 1) {
        for (int y = data->y - data->best + 1; y <= data->y; y++)
            for (int x = data->x - data->best + 1; x <= data->x; x++)
                data->grid_origin[y][x] = data->full;
    }

    return true;
}

int main(int ac, char **av) {
    meta_t data = {0};

    if (ac == 2) {
        data.file = fopen(av[1], "r");
        if (!data.file)
            return (puts("Error: invalid map"), 1);
    } else {
        data.file = stdin;
    }

    if (!read_map(&data) || !solve(&data))
        return (puts("Error: invalid map"), 1);

    for (int i = 0; i < data.lines; i++)
        printf("%s\n", data.grid_origin[i]);

    return 0;
}
