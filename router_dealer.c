/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Beau Pullens (2099993)
 * Jingxuan Zhang (2088746)
 * Anna Schrasser (2069237)
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>    // for execlp
#include <mqueue.h>    // for mq


#include "settings.h"  
#include "messages.h"

#define STUDENT_NAME "42"
#define SIGTERM 15

char client2dealer_name[30];
char dealer2worker1_name[30];
char dealer2worker2_name[30];
char worker2dealer_name[30];

int main (int argc, char * argv[]) {
  if (argc != 1) {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
  // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)

    sprintf (client2dealer_name, "/mq_request_%s_%d", STUDENT_NAME, getpid());
    sprintf (dealer2worker1_name, "/mq_S1_%s_%d", STUDENT_NAME, getpid());
    sprintf (dealer2worker2_name, "/mq_S2_%s_%d", STUDENT_NAME, getpid());
    sprintf (worker2dealer_name, "/mq_response_%s_%d", STUDENT_NAME, getpid());

    // begin setting queue attributes and creating queues
    struct mq_attr attr;
    attr.mq_maxmsg = MQ_MAX_MESSAGES;

    // request queue
    attr.mq_msgsize = sizeof (req_msg_t);
    mqd_t mq_fd_request = mq_open (client2dealer_name, O_RDWR | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &attr);

    // S1 queue
    attr.mq_msgsize = sizeof (job_msg_t);
    mqd_t mq_fd_S1 = mq_open (dealer2worker1_name, O_RDWR | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &attr);

    // S2 queue
    attr.mq_msgsize = sizeof (job_msg_t);
    mqd_t mq_fd_S2 = mq_open (dealer2worker2_name, O_RDWR | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &attr);

    // response queue
    attr.mq_msgsize = sizeof (rsp_msg_t);
    mqd_t mq_fd_response = mq_open (worker2dealer_name, O_RDWR | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &attr);

    //  * create the child processes (see process_test() and
    //    message_queue_test())
    //  * read requests from the Req queue and transfer them to the workers
    //    with the Sx queues
    //  * read answers from workers in the Rep queue and print them
    //  * wait until the client has been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // start client
    pid_t client_pid = fork();

    if (client_pid < 0) {
        perror("fork client failed");
        exit(1);
    }
    if (client_pid == 0) {
        execlp("./client", "client", client2dealer_name, NULL);
        perror("execlp client failed");
        exit(1);
    }

    // start workers for service 1
    pid_t workers_s1[N_SERV1];
    
    for (int i = 0; i < N_SERV1; i++) {
        workers_s1[i] = fork();
        if (workers_s1[i] < 0) {
            perror("fork worker_s1 failed");
            exit(1);
        }
        if (workers_s1[i] == 0) {
            execlp("./worker_s1", "worker_s1", dealer2worker1_name, worker2dealer_name, NULL);
            perror("execlp worker_s1 failed");
            exit(1);
        }
    }

    // start workers for service 2
    pid_t workers_s2[N_SERV2];

    for (int i = 0; i < N_SERV2; i++) {
        workers_s2[i] = fork();
        if (workers_s2[i] < 0) {
            perror("fork worker_s2 failed");
            exit(1);
        }
        if (workers_s2[i] == 0) {
            execlp("./worker_s2", "worker_s2", dealer2worker2_name, worker2dealer_name, NULL);
            perror("execlp worker_s2 failed");
            exit(1);
        }
    }

    bool client_done = false;
    int pending_jobs = 0;

    bool have_req = false;
    req_msg_t held_req;

    while (true) {
        // if not yet storing a request, try to read a new one
        if (!have_req) {
            ssize_t r = mq_receive(mq_fd_request, (char *)&held_req, sizeof(held_req), NULL);
            if (r >= 0) {
                have_req = true;
            } else if (errno != EAGAIN) {
                // if receiving gives erorr EAGAIN then the request queue is empty so continue, otherwise real error
                perror("mq_receive Req");
                break;
            }
        }
        
        // if it is storing a request, send it to the job queues
        if (have_req) {
            // create job object
            job_msg_t job;
            job.request_id = held_req.request_id;
            job.data = held_req.data;

            if (held_req.service_id == 1 || held_req.service_id == 2) {
                mqd_t target = (held_req.service_id == 1) ? mq_fd_S1 : mq_fd_S2;

                if (mq_send(target, (char *)&job, sizeof(job), 0) == 0) {
                    // forwarded successfully, increase num of pending jobs and remove currently held message
                    pending_jobs++;
                    have_req = false;
                } else {
                    // if EAGAIN error is given when sending then job queue is full, otherwise real error
                    if (errno != EAGAIN) {
                        perror("mq_send failed");
                        break;
                    }
                }
            }
        }

        // try to read a job response
        rsp_msg_t rsp;
        ssize_t s = mq_receive(mq_fd_response, (char *)&rsp, sizeof(rsp), NULL);
        if (s >= 0) {
            // write the response to output and decrease number of jobs being processed
            printf("%d -> %d\n", rsp.request_id, rsp.result);
            fflush(stdout);
            pending_jobs--;
        } else {
            if (errno != EAGAIN) {
                perror("mq_receive Rsp failed");
                break;
            }
        }

        // check if client is done
        if (!client_done)
        {
            int status;
            pid_t w = waitpid(client_pid, &status, WNOHANG);
            if (w > 0) {
                client_done = true;
            }
        }

        // if client is done and there are no jobs left currently being processed then start termination
        if (client_done && pending_jobs <= 0 && !have_req) {
            // also make sure there are no job responses left that need to be printed nor are there requests not yet sent to workers
            struct mq_attr a_req, a_rsp;
            if (mq_getattr(mq_fd_request, &a_req) == 0 
                && mq_getattr(mq_fd_response, &a_rsp) == 0 
                && a_req.mq_curmsgs == 0 
                && a_rsp.mq_curmsgs == 0) 
            {
                break;
            }
        }
    }

    // terminate all the workers
    for (int i = 0; i < N_SERV1; i++) {
        kill(workers_s1[i], SIGTERM);
    }
    for (int i = 0; i < N_SERV2; i++) {
        kill(workers_s2[i], SIGTERM);
    }

    for (int i = 0; i < N_SERV1; i++) {
        waitpid(workers_s1[i], NULL, 0);
    }
    for (int i = 0; i < N_SERV2; i++) {
        waitpid(workers_s2[i], NULL, 0);
    }

    // close and unlink all queues
    mq_close(mq_fd_request);
    mq_close(mq_fd_S1);
    mq_close(mq_fd_S2);
    mq_close(mq_fd_response);

    mq_unlink(client2dealer_name);
    mq_unlink(dealer2worker1_name);
    mq_unlink(dealer2worker2_name);
    mq_unlink(worker2dealer_name);

    // Important notice: make sure that the names of the message queues
    // contain your goup number (to ensure uniqueness during testing)
  
  return (0);
}
