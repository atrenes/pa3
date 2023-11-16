/**
 * @file utility.c
 * @Author Danil Khanalainen and Anna Ershova
 * @date November, 2023
 * @brief All functions that make main() less complicated
 */

#include "utility.h"
#include "logger.h"
#include "clock.h"

int create_pipe_pool(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]) {
    for (int i = 0 ; i < all_proc_num ; i++) {
        for (int j = 0 ; j < all_proc_num ; j++) {
            if (i != j) {
                int p[2];
                if (pipe(p) < 0) return 1;
                fcntl(p[0], F_SETFL, O_NONBLOCK); // non-blocking read
                fcntl(p[1], F_SETFL, O_NONBLOCK); // non-blocking write
                pipe_pool[i][j][0] = p[0];
                pipe_pool[j][i][1] = p[1];
                log_created_pipe(p[0], p[1]);
            }
        }
    }
    return 0;
}

void get_fds(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2], struct my_process *proc) {
    for (int j = 0 ; j < all_proc_num ; j++) {
        if (proc->this_id != j) {
            proc->read_fd[j] = pipe_pool[proc->this_id][j][0];
            proc->write_fd[j] = pipe_pool[proc->this_id][j][1];
        }
    }
}

struct my_process *split(struct my_process *proc, const balance_t *balance) {
    int cur_pid;
    for (local_id i = 1 ; i < (local_id) proc->proc_num + 1 ; i++) {
        cur_pid = fork();
        if (cur_pid != 0) {
            continue;
        } else {
            struct my_process *child = malloc(sizeof(struct my_process));

            child->this_pid = getpid();
            child->parent_pid = proc->this_pid;
            child->this_id = i;
            child->proc_num = proc->proc_num;
            child->read_fd = malloc(sizeof(int) * (proc->proc_num + 1));
            child->write_fd = malloc(sizeof(int) * (proc->proc_num + 1));

            child->balance_state.s_balance_pending_in = 0;
            child->balance_state.s_balance = balance[i - 1];
            child->balance_state.s_time = get_lamport_time();

            child->balance_history.s_id = i;
            child->balance_history.s_history_len = 1;
            child->balance_history.s_history[0] = child->balance_state;
            return child;
        }
    }
    return proc;
}

Message create_message(MessageType type, void* message, int size) {
    Message m;
    m.s_header.s_magic = MESSAGE_MAGIC;
    m.s_header.s_type = type;
    m.s_header.s_local_time = logical_time_increment();
    m.s_header.s_payload_len = size;

    if (message != NULL) {
        memcpy(m.s_payload, message, m.s_header.s_payload_len);
    }
    return m;
}

void destroy_all_pipes(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]) {
    for (int i = 0 ; i < all_proc_num ; i++) {
        for (int j = 0 ; j < all_proc_num ; j++) {
            if (i != j) {
                close(pipe_pool[i][j][0]);
                close(pipe_pool[i][j][1]);
                log_closed_fd(pipe_pool[i][j][0]);
                log_closed_fd(pipe_pool[i][j][1]);
            }
        }
    }
}

void destroy_unused_pipes(struct my_process *proc, int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]) {
    for (int i = 0 ; i < all_proc_num ; i++) {
        for (int j = 0 ; j < all_proc_num ; j++) {
            if (i != j && i != proc->this_id) {
                close(pipe_pool[i][j][0]);
                close(pipe_pool[i][j][1]);
                log_closed_fd(pipe_pool[i][j][0]);
                log_closed_fd(pipe_pool[i][j][1]);
            }
        }
    }
}

void print_history(const AllHistory * history) {
    if (history == NULL) {
        fprintf(stderr, "print_history: history is NULL!\n");
        exit(1);
    }

    typedef struct {
        int balance;
        int pending;
    } Pair;

    int max_time = 0;
    int has_pending = 0;
    int nrows = history->s_history_len + 2; // 0 row (parent) is not used + Total row
    Pair table[nrows][MAX_T];
    memset(table, 0, sizeof(table));

    for (int i = 0; i < history->s_history_len; ++i) {
        for (int j = 0; j < history->s_history[i].s_history_len; ++j) {
            const BalanceState * change = &history->s_history[i].s_history[j];
            int id = history->s_history[i].s_id;
            table[id][change->s_time].balance = change->s_balance;
            table[id][change->s_time].pending = change->s_balance_pending_in;
            if (max_time < change->s_time) {
                max_time = change->s_time;
            }
            if (change->s_balance_pending_in > 0) {
                has_pending = 1;
            }
        }
    }

    if (max_time > MAX_T) {
        fprintf(stderr, "print_history: max value of s_time: %d, expected s_time < %d!\n",
                max_time, MAX_T);
        return;
    }

    // Calculate total sum
    for (int j = 0; j <= max_time; ++j) {
        int sum = 0;
        for (int i = 1; i <= history->s_history_len; ++i) {
            sum += table[i][j].balance + table[i][j].pending;
        }
        table[nrows-1][j].balance = sum;
        table[nrows-1][j].pending = 0;
    }

    // pretty print
    fflush(stderr);
    fflush(stdout);

    const char * cell_format_pending = " %d (%d) ";
    const char * cell_format = " %d ";

    char buf[128];
    int max_cell_width = 0;
    for (int i = 1; i <= history->s_history_len; ++i) {
        for (int j = 0; j <= max_time; ++j) {
            if (has_pending) {
                sprintf(buf, cell_format_pending, table[i][j].balance, table[i][j].pending);
            } else {
                sprintf(buf, cell_format, table[i][j].balance);
            }
            int width = strlen(buf);
            if (max_cell_width < width) {
                max_cell_width = width;
            }
        }
    }

    const char * const first_column_header = "Proc \\ time |";
    const int first_column_width = strlen(first_column_header);
    const int underscrores = (first_column_width + 1) + (max_cell_width + 1) * (max_time + 1);

    char hline[underscrores + 2];
    for (int i = 0; i < underscrores; ++i) {
        hline[i] = '-';
    }
    hline[underscrores] = '\n';
    hline[underscrores + 1] = '\0';

    if (has_pending) {
        printf("\nFull balance history for time range [0;%d], $balance ($pending):\n", max_time);
    } else {
        printf("\nFull balance history for time range [0;%d], $balance:\n", max_time);
    }
    printf("%s", hline);

    printf("%s ", first_column_header);
    for (int j = 0; j <= max_time; ++j) {
        printf("%*d |", max_cell_width - 1, j);
    }
    printf("\n");
    printf("%s", hline);

    for (int i = 1; i <= history->s_history_len; ++i) {
        printf("%11d | ", i);
        for (int j = 0; j <= max_time; ++j) {
            if (has_pending) {
                sprintf(buf, cell_format_pending, table[i][j].balance, table[i][j].pending);
            } else {
                sprintf(buf, cell_format, table[i][j].balance);
            }
            printf("%*s|", max_cell_width, buf);
        }
        printf("\n");
        printf("%s", hline);
    }

    printf("      Total | ");
    for (int j = 0; j <= max_time; ++j) {
        printf("%*d |", max_cell_width - 1, table[nrows-1][j].balance);
    }
    printf("\n");
    printf("%s", hline);
}

void update_history(struct my_process *proc) {
    for (timestamp_t t = proc->balance_history.s_history_len; t < proc->balance_state.s_time; t++) {
        proc->balance_history.s_history[t] = proc->balance_history.s_history[t-1];
        proc->balance_history.s_history[t].s_time = t;
    }
    proc->balance_history.s_history[proc->balance_state.s_time] = proc->balance_state;
    proc->balance_history.s_history_len = proc->balance_state.s_time + 1;
}

void update_pending(struct my_process *proc, timestamp_t time_message, balance_t transaction_money) {
    for (timestamp_t time_ = time_message - 1 ; time_ < proc->balance_history.s_history_len - 1 ; time_++) {
        proc->balance_history.s_history[time_].s_balance_pending_in = transaction_money;
    }
}

void handle_transaction(struct my_process *proc, Message *msg) {
    TransferOrder *order = (TransferOrder*) msg->s_payload;

    if (proc->this_id == order->s_src) {
        //printf("my time: %d\n", proc->balance_state.s_time);
        proc->balance_state.s_balance -= order->s_amount;
        update_history(proc);

        TransferOrder order_resend = {
                .s_src = order->s_src,
                .s_dst = order->s_dst,
                .s_amount = order->s_amount
        };

        //explicit usage of create_message because it updates timestamp in message
        Message msg_resend = create_message(TRANSFER, &order_resend, sizeof(order_resend));

        send(proc, order->s_dst, &msg_resend);
        return;
    }
    if (proc->this_id == order->s_dst) { // receiver
        proc->balance_state.s_balance += order->s_amount;
        proc->balance_state.s_balance_pending_in = 0;
        update_history(proc);
        update_pending(proc, msg->s_header.s_local_time, order->s_amount);
        Message m = create_message(ACK, NULL, 0);
        send(proc, PARENT_ID, &m);
        return;
    }
}

void child_send_done_to_all(struct my_process *proc) {
    int len = snprintf(NULL, 0, log_done_fmt, get_lamport_time(), proc->this_id, proc->balance_state.s_balance);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_done_fmt, get_lamport_time(), proc->this_id, proc->balance_state.s_balance);

    Message m = create_message(DONE, buffer, len);
    send_multicast(proc, &m);

    log_done(proc);

    free(buffer);
}

void parent_receive_all_started(struct my_process *proc, int proc_num) {
    Message m;
    int received_num = 0;
    while (received_num != proc_num) {
        if (receive_any(proc, &m) == 0 && m.s_header.s_type == STARTED) {
            received_num++;
            //printf("time after receive %d\n", get_lamport_time());
        }
    }
}

void parent_send_stop_to_all(struct my_process *proc) {
    Message m_stop = create_message(STOP, NULL, 0);

    send_multicast(proc, &m_stop);
}

void parent_receive_all_done(struct my_process *proc, int proc_num) {
    Message m;
    int received_num = 0;
    while (received_num != proc_num) {
        for (local_id i = 1 ; i < proc_num + 1 ; i++) {
            if (receive(proc, i, &m) == 0 && m.s_header.s_type == DONE) {
                received_num++;
            }
        }
    }
}

void parent_receive_all_history(struct my_process *proc, int proc_num, BalanceHistory *history) {
    Message m_balance;
    int received_num = 0;

    while (received_num != proc_num) {
        for (local_id i = 0 ; i < proc_num ; i++) {
            if (receive(proc, i+1, &m_balance) == 0 && m_balance.s_header.s_type == BALANCE_HISTORY) {
                received_num++;

                //history[i] = *((BalanceHistory*) m_balance.s_payload);
                memcpy(&(history[i]), (BalanceHistory*) m_balance.s_payload, m_balance.s_header.s_payload_len);
            }
        }
    }
}

void child_send_and_receive_started(struct my_process *proc, int proc_num) {
    char *buffer = malloc(sizeof(char) * MAX_MESSAGE_LEN);
    snprintf(buffer, MAX_MESSAGE_LEN, log_started_fmt, get_lamport_time(), proc->this_id, proc->this_pid, proc->parent_pid, proc->balance_state.s_balance);
    Message m = create_message(STARTED, buffer, (int) strlen(buffer));

    send_multicast(proc, &m);
    log_started(proc);

    int received_num = 0;
    while (received_num != proc_num - 1) {
        for (local_id i = 1 ; i < proc_num + 1 ; i++) {
            if (i != proc->this_id) {
                if (receive(proc, i, &m) == 0 && m.s_header.s_type == STARTED) {
                    received_num++;
                }
            }
        }
    }
}
