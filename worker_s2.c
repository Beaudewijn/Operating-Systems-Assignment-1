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
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()

#include "messages.h"
#include "service2.h"

static void rsleep (int t);

char* name = "NO_NAME_DEFINED";
mqd_t dealer2worker;
mqd_t worker2dealer;


int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the
    //    arguments)
    //  * repeatedly:
    //      - read from the S2 message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do the job 
    //      - write the results to the Rsp message queue
    //    until there are no more tasks to do
    //  * close the message queues

    char *s2_queue_name = argv[1];
    char *rsp_queue_name = argv[2];
    mqd_t s2_queue = mq_open(s2_queue_name, O_RDONLY);
    mqd_t rsp_queue = mq_open(rsp_queue_name, O_WRONLY);

    while (true) {
        job_msg_t job;
        mq_receive(s2_queue, (char *) &job, sizeof (s2_queue), NULL);

        rsleep(10000);

        int result = service(job.data);
        rsp_msg_t rsp;
        rsp.request_id = job.request_id;
        rsp.result = result;

        mq_send(rsp_queue, (const char*)&rsp, sizeof(rsp_queue), 0);
    }
    mq_close(s2_queue);
    mq_close(rsp_queue);

    return(0);
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
