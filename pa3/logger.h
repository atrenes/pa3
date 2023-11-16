/**
 * @file logger.h
 * @Author Danil Khanalainen and Anna Ershova
 * @date October, 2023
 */

#ifndef RASPRED1_LOGGER_H
#define RASPRED1_LOGGER_H

#include "common.h"
#include "pa2345.h"
#include "utility.h"
#include <stdio.h>
#include <sys/file.h>
#include "clock.h"

static const char *const log_created_pipe_fmt = "CREATED pipe (read fd %d, write fd %d)\n";
static const char *const log_closed_fd_fmt = "CLOSED fd %d\n";

void open_logfiles(void);
void log_started(struct my_process *proc);
void log_received_all_started(struct my_process *proc);
void log_done(struct my_process *proc);
void log_received_all_done(struct my_process *proc);
void log_transfer_in(TransferOrder *order);
void log_transfer_out(TransferOrder *order);
void log_created_pipe(int read_fd, int write_fd);
void log_closed_fd(int fd);
void close_logfiles(void);

#endif //RASPRED1_LOGGER_H
