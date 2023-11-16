/**
 * @file ipc.c
 * @Author Danil Khanalainen and Anna Ershova
 * @date October, 2023
 */

#include "../pa2/utility.h"
#include "clock.h"

int send(void * self, local_id dst, const Message * msg) {
    struct my_process *proc = self;
    if (proc->this_id == dst) {
        return 1;
    }
    write(proc->write_fd[dst], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    return 0;
}

int send_multicast(void * self, const Message * msg) {
    struct my_process *proc = self;
    for (int i = 0 ; i < proc->proc_num+1 ; i++) {
        if (i != proc->this_id) {
            write(proc->write_fd[i], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
        }
    }
    return 0;
}

int receive(void * self, local_id from, Message * msg) {
    struct my_process *proc = self;
    ssize_t header_read = read(proc->read_fd[from], &(msg->s_header), sizeof(MessageHeader));
    if (msg->s_header.s_payload_len != 0 && header_read == sizeof(MessageHeader)) {
        ssize_t payload = read(proc->read_fd[from], msg->s_payload, msg->s_header.s_payload_len);
        if (payload != msg->s_header.s_payload_len) {
            return 1;
        }
        logical_time_choose(msg->s_header.s_local_time);
        proc->balance_state.s_time = logical_time_increment();
        return 0;
    } else if (header_read == sizeof(MessageHeader) && msg->s_header.s_payload_len == 0) {
        logical_time_choose(msg->s_header.s_local_time);
        proc->balance_state.s_time = logical_time_increment();
        return 0; //payload is really == 0
    }
    return 1;
}

int receive_any(void * self, Message * msg) {
    struct my_process *proc = self;
    for (int i = 0 ; i < proc->proc_num+1 ; i++) {
        if (i != proc->this_id) {
            ssize_t header_read = read(proc->read_fd[i], &(msg->s_header), sizeof(MessageHeader));
            if (header_read == sizeof(MessageHeader) && msg->s_header.s_payload_len != 0) {
                ssize_t payload = read(proc->read_fd[i], msg->s_payload, msg->s_header.s_payload_len);
                if (payload != msg->s_header.s_payload_len) {
                    return 1;
                }
                logical_time_choose(msg->s_header.s_local_time);
                proc->balance_state.s_time = logical_time_increment();
                return 0;
            } else if (header_read == sizeof(MessageHeader) && msg->s_header.s_payload_len == 0) {
                logical_time_choose(msg->s_header.s_local_time);
                proc->balance_state.s_time = logical_time_increment();
                return 0; // payload == 0, why not?
            }
        }
    }
    return 1;
}
