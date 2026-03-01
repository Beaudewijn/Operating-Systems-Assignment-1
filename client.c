/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
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
#include "request.h"

static void rsleep (int t);


int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the message queue (whose name is provided in the
    //    arguments)
    //  * repeatingly:
    //      - get the next job request 
    //      - send the request to the Req message queue
    //    until there are no more requests to send
    //  * close the message queue

    char *req_queue_name = argv[1];
    mqd_t req_queue = mq_open(req_queue_name, O_WRONLY);
    
    while(true) {
        int jobID, data, serviceID;

        int result = getNextRequest(&jobID, &data, &serviceID);
        if (result == NO_REQ) {
            break;
        }
        if (result != NO_ERR) {
            fprintf(stderr, "getNextRequest returned error %d\n", result);
            break;
        }

        REQ_QUEUE msg;
        msg.request_id = jobID;
        msg.service_id = service_ID;
        msg.data = data;

        if (mq_send(req_queue, (const char*)&msg, sizeof(req_msg_t), 0) == -1) {
            perror("mq_send Req_queue");
            break;
        }
    }

    mq_close(req_queue);
    return (0);
}
