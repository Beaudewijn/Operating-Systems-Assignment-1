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

#ifndef MESSAGES_H
#define MESSAGES_H

// define the data structures for your messages here

//Req_queue
typedef struct {
    int request_id;
    int service_id;
    int data;
} req_msg_t;

//S1_queue, S2_queue
typedef struct {
    int request_id;
    int data;
} job_msg_t;

//Rsp_queue
typedef struct {
    int request_id;
    int result;
} rsp_msg_t;


#endif
