#include "banking.h"
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include "logger.h"
#include <sys/wait.h>

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount) {
    struct my_process *proc = parent_data;

    TransferOrder order = {
            .s_src = src,
            .s_dst = dst,
            .s_amount = amount
    };

    Message msg_send = create_message(TRANSFER, &order, sizeof(order));

    send(proc, src, &msg_send);
    log_transfer_out(&order);

    Message msg_rec;

    while (msg_rec.s_header.s_type != ACK && receive(proc, dst, &msg_rec) != 0)

    log_transfer_in(&order);
}

void parent_function(struct my_process *proc, int proc_num, int pipe_pool[proc_num+1][proc_num+1][2]) {
    parent_receive_all_started(proc, proc_num);
    log_received_all_started(proc);

    bank_robbery(proc, (local_id) proc_num);

    //send stop
    parent_send_stop_to_all(proc);

    parent_receive_all_done(proc, proc_num);
    log_received_all_done(proc);

    //get BALANCE_HISTORY from every process
    BalanceHistory history[proc_num+1];

    parent_receive_all_history(proc, proc_num, history);

    AllHistory all_history = {
            .s_history_len = proc_num
    };

    for (local_id i = 0; i < proc_num; i++) {
        all_history.s_history[i] = history[i];
    }

    print_history(&all_history);

    for (int i = 0 ; i < proc_num ; i++) {
        wait(NULL);
    }

    destroy_all_pipes(proc_num+1, pipe_pool);
    close_logfiles();
}

void child_function(struct my_process *proc, int proc_num) {
    child_send_and_receive_started(proc, proc_num);

    log_received_all_started(proc);

    //work from there

    int done = 0;
    while (done != proc_num - 1) {
        Message msg_rcv;

        if (receive_any(proc, &msg_rcv) != 0) {
            continue;
        }

        switch (msg_rcv.s_header.s_type) {
            case DONE:

                done++;
                break;

            case STOP:
                child_send_done_to_all(proc);
                //printf("time after send done: %d\n", get_lamport_time());
                break;

            case TRANSFER:
                handle_transaction(proc, &msg_rcv);
                break;

            default:
                break;
        }
    }

    log_received_all_done(proc);
    //work to here

    proc->balance_state = proc->balance_history.s_history[proc->balance_history.s_history_len-1];
    proc->balance_state.s_time = get_lamport_time();
    update_history(proc);

    Message balance_msg = create_message(BALANCE_HISTORY,
                                         &(proc->balance_history),
                                         sizeof(proc->balance_history));

    send(proc, PARENT_ID, &balance_msg);

    exit(0);
}

int main(int argc, char * argv[]) {
    open_logfiles();

    if (argc <= 3 || strcmp(argv[1], "-p") != 0 || atoi(argv[2]) < 2) {
        return 1;
    }
    const int proc_num = atoi(argv[2]);
    balance_t balance[proc_num];
    for (int i = 0 ; i < proc_num ; i++) {
        balance[i] = atoi(argv[i + 3]);
    }

    struct my_process *proc = malloc(sizeof(struct my_process));
    proc->this_pid = getpid();
    proc->this_id = PARENT_ID;
    proc->proc_num = proc_num;
    proc->write_fd = malloc(sizeof(int) * (proc_num + 1));
    proc->read_fd = malloc(sizeof(int) * (proc_num + 1));

    int pipe_pool[proc_num + 1][proc_num + 1][2];
    create_pipe_pool(proc_num+1, pipe_pool);

    proc = split(proc, balance);
    destroy_unused_pipes(proc, proc_num+1, pipe_pool);
    get_fds(proc_num + 1, pipe_pool, proc);

    // here starts the thing

    if (proc->this_id == 0) {
        parent_function(proc, proc_num, pipe_pool);
    } else {
        child_function(proc, proc_num);
    }

    return 0;
}
