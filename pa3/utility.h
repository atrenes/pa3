/**
 * @file utility.h
 * @Author Danil Khanalainen and Anna Ershova
 * @date October, 2023
 */

#ifndef RASPRED1_UTILITY_H
#define RASPRED1_UTILITY_H
#include "ipc.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "banking.h"
#include <stdbool.h>
#include "clock.h"

#define PARENT_ID 0

struct my_process {
    int this_pid;
    int parent_pid;
    local_id this_id;
    int proc_num;
    int *read_fd;
    int *write_fd;
    BalanceHistory balance_history;
    BalanceState balance_state;
};

int create_pipe_pool(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]);
void get_fds(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2], struct my_process *proc);
struct my_process *split(struct my_process *proc, const balance_t *balance);
Message create_message(MessageType type, void* message, int size);
void destroy_all_pipes(int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]);
void destroy_unused_pipes(struct my_process *proc, int all_proc_num, int pipe_pool[all_proc_num][all_proc_num][2]);
void handle_transaction(struct my_process *proc, Message *msg);
void child_send_done_to_all(struct my_process *proc);
void parent_receive_all_started(struct my_process *proc, int proc_num);
void parent_send_stop_to_all(struct my_process *proc);
void parent_receive_all_done(struct my_process *proc, int proc_num);
void parent_receive_all_history(struct my_process *proc, int proc_num, BalanceHistory *history);
void child_send_and_receive_started(struct my_process *proc, int proc_num);
void update_history(struct my_process *proc);

#endif //RASPRED1_UTILITY_H
