/**
 * @file logger.c
 * @Author Danil Khanalainen and Anna Ershova
 * @date November, 2023
 */

#include <stdlib.h>
#include "logger.h"
#include <string.h>

int event_logfile;
int pipes_logfile;

void open_logfiles(void) {
    event_logfile = open(events_log, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC);
    pipes_logfile = open(pipes_log, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC);
}

void log_started(struct my_process *proc) {
    int len = snprintf(NULL, 0, log_started_fmt, get_lamport_time(), proc->this_id, proc->this_pid, proc->parent_pid, proc->balance_state.s_balance);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_started_fmt, get_lamport_time(), proc->this_id, proc->this_pid, proc->parent_pid, proc->balance_state.s_balance);
    write(event_logfile, buffer, strlen(buffer));
    printf(log_started_fmt, get_lamport_time(), proc->this_id, proc->this_pid, proc->parent_pid, proc->balance_state.s_balance);
//    free(buffer);
}

void log_received_all_started(struct my_process *proc) {
    int len = snprintf(NULL, 0, log_received_all_started_fmt, get_lamport_time(), proc->this_id);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_received_all_started_fmt, get_lamport_time(), proc->this_id);
    write(event_logfile, buffer, strlen(buffer));
    printf(log_received_all_started_fmt, get_lamport_time(), proc->this_id);
//    free(buffer);
}

void log_done(struct my_process *proc) {
    int len = snprintf(NULL, 0, log_done_fmt, get_lamport_time(), proc->this_id, proc->balance_state.s_balance);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_done_fmt, get_lamport_time(), proc->this_id, proc->balance_state.s_balance);
    write(event_logfile, buffer, strlen(buffer));
    printf(log_done_fmt, get_lamport_time(), proc->this_id, proc->balance_state.s_balance);
//    free(buffer);
}

void log_received_all_done(struct my_process *proc) {
    int len = snprintf(NULL, 0, log_received_all_done_fmt, get_lamport_time(), proc->this_id);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_received_all_done_fmt, get_lamport_time(), proc->this_id);
    write(event_logfile, buffer, strlen(buffer));
    printf(log_received_all_done_fmt, get_lamport_time(), proc->this_id);
//    free(buffer);
}

void log_transfer_out(TransferOrder *order) {
    int len = snprintf(NULL, 0, log_transfer_out_fmt, get_lamport_time(), order->s_src, order->s_amount, order->s_dst);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_transfer_out_fmt, get_lamport_time(), order->s_src, order->s_amount, order->s_dst);
    write(event_logfile, buffer, strlen(buffer));
    printf(log_transfer_out_fmt, get_lamport_time(), order->s_src, order->s_amount, order->s_dst);
}

void log_transfer_in(TransferOrder *order) {
    int len = snprintf(NULL, 0, log_transfer_in_fmt, get_lamport_time(), order->s_dst, order->s_amount, order->s_src);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_transfer_in_fmt, get_lamport_time(), order->s_dst, order->s_amount, order->s_src);
    write(event_logfile, buffer, strlen(buffer));
    printf(log_transfer_in_fmt, get_lamport_time(), order->s_dst, order->s_amount, order->s_src);
}

void log_created_pipe(int read_fd, int write_fd) {
    int len = snprintf(NULL, 0, log_created_pipe_fmt, read_fd, write_fd);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_created_pipe_fmt, read_fd, write_fd);
    write(pipes_logfile, buffer, strlen(buffer));
//    free(buffer);
}

void log_closed_fd(int fd) {
    int len = snprintf(NULL, 0, log_closed_fd_fmt, fd);
    char *buffer = malloc(sizeof(char) * len);
    snprintf(buffer, len+1, log_closed_fd_fmt, fd);
    write(pipes_logfile, buffer, strlen(buffer));
//    free(buffer);
}

void close_logfiles(void) {
    close(event_logfile);
    close(pipes_logfile);
}
