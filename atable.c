typedef long long i64;
typedef unsigned int u32;
typedef unsigned long long u64;

#define KILOBYTE (1024)
#define MEGABYTE (1024*KILOBYTE)

#define BUFFER_SIZE (512 * MEGABYTE)
#define MAX_CELLS 16384 * 512
#define MAX_COLS 512

#define STDIN_FD 0
#define STDOUT_FD 1

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct Cell
{
    u32 row;
    u32 col;

    char *data;
    u32 length;
} Cell;

char input[BUFFER_SIZE];
char output[BUFFER_SIZE];
int column_max_cell_length[MAX_COLS];
Cell cells[MAX_CELLS];
u32 cell_number;

__attribute__((naked)) void exit(int code)
{
    asm("mov $60,%%rax;"
        "mov %[rcode],%%edi;"
        "syscall"
        :
        : [rcode] "r" (code)
        : "%rax", "%edi"
        );
}

i64 read(int file_descriptor, void *buffer, u64 n)
{
    i64 bytes_read = 0;

    asm("mov $0,%%rax;"
        "mov %[fd],%%edi;"
        "mov %[buffer],%%rsi;"
        "mov %[n],%%rdx;"
        "syscall;"
        "mov %%rax, %[bytes_read]"
        : [bytes_read] "=r" (bytes_read)
        : [fd] "r" (file_descriptor), [buffer] "r" (buffer), [n] "r" (n)
        : "%rax", "%edi", "%rsi", "%rdx", "%rax"
        );

    return bytes_read;
}

i64 write(int file_descriptor, void *buffer, u64 n)
{
    i64 bytes_written = 0;

    asm("mov $1,%%rax;"
        "mov %[fd],%%edi;"
        "mov %[buffer],%%rsi;"
        "mov %[n],%%rdx;"
        "syscall;"
        "mov %%rax, %[bytes_written]"
        : [bytes_written] "=r" (bytes_written)
        : [fd] "r" (file_descriptor), [buffer] "r" (buffer), [n] "r" (n)
        : "%rax", "%edi", "%rsi", "%rdx", "%rax"
        );

    return bytes_written;
}

int is_whitespace(char c)
{
    return c == ' ' || (c >= 0x09 && c <= 0x0d);
}

void fill_cells(char *buffer, u64 n)
{
    int row = 0;
    char *line = 0x0;
    int line_length = 0; 

    for(;buffer[0] != '\0';)
    {
        line = buffer;
        line_length = 0;

        for(; !(buffer[0] == '\n' || buffer[0] == '\0'); buffer++)
        {
            line_length += 1;
        }

        if(buffer[0] == '\n')
        {
            buffer++; // skip newline
        }

        while(is_whitespace(line[0]) && line_length > 0)
        {
            line++;
            line_length--;
        }

        while(is_whitespace(line[line_length - 1]) && line_length > 0)
        {
            line_length--;
        }

        if(line_length > 0) {
            line++; //skip the first pipe
            line_length -= 2; // skip the first and last pipe

            int col = 0;
            for(;;)
            {
                Cell *cell = &cells[cell_number++];
                cell->row = row;
                cell->col = col;
                cell->data = line;

                while(!(cell->length == line_length || cell->data[cell->length] == '|'))
                {
                    cell->length += 1;
                }

                line += cell->length;
                line_length -= cell->length;

                while(is_whitespace(cell->data[0]) && cell->length > 0)
                {
                    cell->data++;
                    cell->length--;
                }

                while(is_whitespace(cell->data[cell->length - 1]) && cell->length > 0)
                {
                    cell->length--;
                }

                column_max_cell_length[col] = MAX(column_max_cell_length[col], cell->length);

                if(line_length == 0)
                {
                    break;
                }

                if(line[0] == '|')
                {
                    line++;
                    line_length--;
                }

                col += 1;
            }

            row += 1;
        }
    }
}

int print_cells_to_buffer(char *buffer)
{
    int written = 0;

    for(int i = 0; i < cell_number; i++)
    {
        Cell *cell = &cells[i];
        if(cell->col == 0)
        {
            if(cell->row != 0)
            {
                buffer[written++] = '\n';
            }

            buffer[written++] = '|';
        }

        buffer[written++] = ' ';

        int padding = column_max_cell_length[cell->col] - cell->length;
        for(int m = 0; m < padding; m++)
        {
            buffer[written++] = ' ';
        }

        for(int k = 0; k < cell->length; k++)
        {
            buffer[written++] = cell->data[k];
        }

        buffer[written++] = ' ';
        buffer[written++] = '|';
    }

    buffer[written++] = '\n';

    return written;
}

void _start()
{
    i64 res = read(STDIN_FD, input, BUFFER_SIZE - 1);

    fill_cells(input, res);
    int to_write = print_cells_to_buffer(output);

    write(STDOUT_FD, output, to_write);
    exit(res);
}
