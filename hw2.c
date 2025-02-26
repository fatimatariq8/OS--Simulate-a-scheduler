#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//process control block (PCB)
struct pcb 
{
	unsigned int pid;
	char pname[20];
	unsigned int ptimeleft;
	unsigned int ptimearrival;
    unsigned int pturnaround;
	unsigned int presponsetime;
};

typedef struct pcb pcb;

//queue node
struct dlq_node 
{
	struct dlq_node *pfwd;
	struct dlq_node *pbck;
	struct pcb *data;
}; 

typedef struct dlq_node dlq_node;

//queue
struct dlq 
{
    int size;
	struct dlq_node *head;
	struct dlq_node *tail;
};

typedef struct dlq dlq;
 
//function to add a pcb to a new queue node
dlq_node * get_new_node (pcb *ndata) 
{
	if (!ndata)
		return NULL;

	dlq_node *new = malloc(sizeof(dlq_node));
	if(!new)
    {
		fprintf(stderr, "Error: allocating memory\n");exit(1);
	}

	new->pfwd = new->pbck = NULL;
	new->data = ndata;
	return new;
}

//function to add a node to the tail of queue
void add_to_tail (dlq *q, dlq_node *new)
{
	if (!new)
		return;

	if (q->head==NULL)
    {
		if(q->tail!=NULL)
        {
			fprintf(stderr, "DLList inconsitent.\n"); exit(1);
		}
		q->head = new;
		q->tail = q->head;
        q->size=1;
	}
	else 
    {		
		new->pfwd = q->tail;
		new->pbck = NULL;
		new->pfwd->pbck = new;
		q->tail = new;
        q->size++;
	}
    
}

//function to remove a node from the head of queue
dlq_node* remove_from_head(dlq * const q){
	if (q->head==NULL){ //empty
		if(q->tail!=NULL){fprintf(stderr, "DLList inconsitent.\n"); exit(1);}
		return NULL;
	}
	else if (q->head == q->tail) { //one element
		if (q->head->pbck!=NULL || q->tail->pfwd!=NULL) {
			fprintf(stderr, "DLList inconsitent.\n"); exit(1);
		}

		dlq_node *p = q->head;
		q->head = NULL;
		q->tail = NULL;
	
		p->pfwd = p->pbck = NULL;
        q->size--;
		return p;
	}
	else { // normal
		dlq_node *p = q->head;
		q->head = q->head->pbck;
		q->head->pfwd = NULL;
	
		p->pfwd = p->pbck = NULL;
        q->size--;
		return p;
	}
}

//function to print our queue
void print_q (const dlq *q) 
{
	dlq_node *n = q->head;
	if (n == NULL)
		return;

	while (n) 
    {
		printf("%s(%d),", n->data->pname, n->data->ptimeleft);
		n = n->pbck;
	}
}

//function to check if the queue is empty
int is_empty (const dlq *q)
{
	if (q->head == NULL && q->tail==NULL)
		return 1;
	else if (q->head != NULL && q->tail != NULL)
		return 0;
	else 
    {
		fprintf(stderr, "Error: DLL queue is inconsistent."); exit(1);
	}
}

//function to sort the queue on completion time
void sort_by_timetocompletion(const dlq *q) 
{ 
	// bubble sort
	dlq_node *start = q->tail;
	dlq_node *end = q->head;
	
	while (start != end) 
    {
		dlq_node *node = start;
		dlq_node *next = node->pfwd;

		while (next != NULL) 
        {
			if (node->data->ptimeleft < next->data->ptimeleft)
            {
				// do a swap
				pcb *temp = node->data;
				node->data = next->data;
				next->data = temp;
			}
			node = next;
			next = node->pfwd;
		}
		end = end ->pbck;
	} 
}

//function to sort the queue on arrival time
void sort_by_arrival_time (const dlq *q) 
{
	// bubble sort
	dlq_node *start = q->tail;
	dlq_node *end = q->head;
	
	while (start != end) 
    {
		dlq_node *node = start;
		dlq_node *next = node->pfwd;

		while (next != NULL) 
        {
			if (node->data->ptimearrival < next->data->ptimearrival)
            {
				// do a swap
				pcb *temp = node->data;
				node->data = next->data;
				next->data = temp;
			}
			node = next;
			next = node->pfwd;
		}
		end = end->pbck;
	}
}

//function to tokenize the one row of data
pcb* tokenize_pdata (char *buf) 
{
	pcb* p = (pcb*) malloc(sizeof(pcb));
	if(!p)
    { 
        fprintf(stderr, "Error: allocating memory.\n"); exit(1);
    }

	char *token = strtok(buf, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token pname\n"); exit(1);
    }  
	strcpy(p->pname, token);

	token = strtok(NULL, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token pid\n"); exit(1);
    }  
	p->pid = atoi(token);

	token = strtok(NULL, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token duration\n"); exit(1);
    } 
	 
	p->ptimeleft= atoi(token);

	token = strtok(NULL, ":\n");
	if(!token)
    { 
        fprintf(stderr, "Error: Expecting token arrival time\n"); exit(1);
    }  
	p->ptimearrival = atoi(token);

	token = strtok(NULL, ":\n");
	if(token)
    { 
        fprintf(stderr, "Error: Oh, what've you got at the end of the line?\n");exit(1);
    } 

	return p;
}

//implement the FIFO scheduling code
void sched_FIFO(dlq *const p_fq, int *p_time)
{
    //add code here to implement FIFO scheduling logic
    double avg_responsetime = 0.0;
    double avg_turnaround = 0.0;
	double total_responsetime = 0.0;
    double total_turnaround = 0.0;
    double avg_throughput = 0.0;
    int N = p_fq->size; //size of queue 
    //printf("%d\n", N);
    dlq_node *current_node = malloc(sizeof(dlq_node)); //allocating memory
    //first node is the first one in queue since its sorted acc to arrival times
    current_node = remove_from_head(p_fq);
    
    dlq ready; //ready queue
    //printf("1");
    ready.head = NULL;
    ready.tail = NULL;
    int flag=0; //indicates if even one process has started executing
    int first_time = 0; //for the first process
    
    (*p_time)++; //since clock tick starting from 1
    while(current_node){
        int flag1=0; 
        pcb *current_process = current_node->data; 
        //if no processes are being executed
        if (current_process->ptimearrival!=(*p_time-1) && flag==0){
            printf("%d:idle:", (*p_time));
        } else{
            if (flag==0){
                first_time = (*p_time); //this is the time of when the first process starts running in the scheduler 
                //updating its response time (only for first process in scheudler)
                current_process->presponsetime = (*p_time) - current_process->ptimearrival;
                total_responsetime+=current_process->presponsetime;
            }
            flag=1;
            printf("%d:%s:", (*p_time), current_process->pname);
 
            current_process->ptimeleft--;
  
            if (!is_empty(&ready)){
                print_q(&ready);
                printf(":\n");
                flag1=1; }
            if (current_process->ptimeleft==0){ //if process has finished
                //updating its turnaround time. The process has completed, we can calculate its turnaround time now 
                current_process->pturnaround = (*p_time) - current_process->ptimearrival;
                total_turnaround+=current_process->pturnaround;
                //taking the next process from our ready queue
                current_node = remove_from_head(&ready);
                if (current_node == NULL) {
                    printf("empty:\n");
                    free(current_node); //free the allocated memory if it's NULL
                    break;
                }
                current_process = current_node->data;
                //updating its response time
                current_process->presponsetime = (*p_time+1) - current_process->ptimearrival;
                total_responsetime+=current_process->presponsetime;
            }
        }
        if (is_empty(&ready) && flag1==0){ //if ready queue is empty and processes have been executed
            printf("empty:\n");}
        
        //temporary node
        dlq_node *n = p_fq->head;
        //if the node's time arrival == clock tick, add it in the ready queue
        while (n!=NULL && n->data->ptimearrival==(*p_time)) { 
            dlq_node *node_to_move = remove_from_head(p_fq);
            add_to_tail(&ready, node_to_move);
            n = p_fq->head;
        }
        
        (*p_time)++; //clock tick 
    }
    free(current_node);
    avg_throughput = (double)N/(((*p_time)-first_time)+1); //total time - the time when the scheduler started running the first process
    avg_turnaround = total_turnaround/(double)N;
	avg_responsetime = total_responsetime/(double)N;
    printf("Average throughput: %f\n", avg_throughput);
    printf("Average response time: %f\n", avg_responsetime);
	printf("Average turnaround time: %f\n", avg_turnaround);

}
//implement the SJF scheduling code
void sched_SJF(dlq *const p_fq, int *p_time)
{
    //add code here to implement SJF scheduling logic
    //printf("sjf");
    double avg_responsetime = 0.0;
    double avg_turnaround = 0.0;
	double total_responsetime = 0.0;
    double total_turnaround = 0.0;
    double avg_throughput = 0.0;
    int N = p_fq->size; //size of queue

    dlq_node *current_node = malloc(sizeof(dlq_node));
    current_node = remove_from_head(p_fq);
    
    dlq ready;
    ready.head = NULL;
    ready.tail = NULL;
    int flag=0;
    int first_time=0;
    
    (*p_time)++;
    while(current_node){
        int flag1=0;
        pcb *current_process = current_node->data;
        //printf("%d:", (*p_time));
        if (current_process->ptimearrival!=(*p_time-1) && flag==0){
            printf("%d:idle:", (*p_time));
        } else{
            if (flag==0){ //if it's the first process
                first_time = (*p_time);
                current_process->presponsetime = (*p_time) - current_process->ptimearrival;
                total_responsetime+=current_process->presponsetime;
            }
            flag=1;
            printf("%d:%s:", (*p_time), current_process->pname);
            //printf("4");
            current_process->ptimeleft--;
            //printf("%d",current_process->ptimeleft );
            if (!is_empty(&ready)){
                print_q(&ready);
                printf(":\n");
                flag1=1; }
            if (current_process->ptimeleft==0){
                current_process->pturnaround = (*p_time) - current_process->ptimearrival;
                total_turnaround+=current_process->pturnaround;
                current_node = remove_from_head(&ready);
                
                if (current_node == NULL) {
                    printf("empty:\n");
                    free(current_node);  // Free the allocated memory if it's NULL
                    break;  
                }
                current_process = current_node->data;
                current_process->presponsetime = (*p_time+1) - current_process->ptimearrival;
                //printf("%d\n",current_process->presponsetime);
                total_responsetime+=current_process->presponsetime;
            }
        }
        if (is_empty(&ready) && flag1==0){
            printf("empty:\n");}
        // } else{
        //     print_q(&ready);
        //     printf(":\n");
        // }
        
        dlq_node *n = p_fq->head;
        while (n!=NULL && n->data->ptimearrival==(*p_time)) {
            dlq_node *node_to_move = remove_from_head(p_fq);
            add_to_tail(&ready, node_to_move);
            sort_by_timetocompletion(&ready); //sorting according to time till completion
            n = p_fq->head;
        }
        (*p_time)++;
    }
    free(current_node);
    avg_throughput = (double)N/(((*p_time)-first_time)+1);
    avg_turnaround = total_turnaround/(double)N;
	avg_responsetime = total_responsetime/(double)N;
    printf("Average throughput: %f\n", avg_throughput);
    printf("Average response time: %f\n", avg_responsetime);
	printf("Average turnaround time: %f\n", avg_turnaround);
}
//implement the STCF scheduling code
void sched_STCF(dlq *const p_fq, int *p_time)
{
    double avg_responsetime = 0.0;
    double avg_turnaround = 0.0;
	double total_responsetime = 0.0;
    double total_turnaround = 0.0;
    double avg_throughput = 0.0;
    int N = p_fq->size;
    int first_time = 0;

    
    dlq_node *current_node = malloc(sizeof(dlq_node));
    current_node = remove_from_head(p_fq);
    
    dlq ready;
    //printf("1");
    ready.head = NULL;
    ready.tail = NULL;
    int flag=0;
    
    (*p_time)++;
    while(current_node){
        int flag1=0;
        pcb *current_process = current_node->data;
        //printf("%d:", (*p_time));
        if (current_process->ptimearrival!=(*p_time-1) && flag==0){
            printf("%d:idle:", (*p_time));
        } else{
            if (flag==0){
                first_time = (*p_time);
                printf("%d", first_time);
                current_process->presponsetime = (*p_time) - current_process->ptimearrival;
                total_responsetime+=current_process->presponsetime;
            }
            flag=1;
            int x = -1;
            if (ready.head != NULL && ready.head->data != NULL) {
            x = ready.head->data->ptimeleft;}
            // printf("%d\n", x);
            if (ready.head!= NULL &&current_process->ptimeleft>x){ //if current process' time left is more than the next one's
                add_to_tail(&ready, current_node); //put the current process back in ready queue
                current_node = remove_from_head(&ready); //start executing the next process

                if (current_node == NULL) {
                    printf("empty:\n");
                    free(current_node);  // Free the allocated memory if it's NULL
                    break;  
                }
                current_process = current_node->data;

                current_process->presponsetime = (*p_time) - current_process->ptimearrival;
                total_responsetime+=current_process->presponsetime;
                printf("%d\n",current_process->presponsetime);
                
            } else if (ready.head!= NULL && current_process->ptimeleft==0){
                current_process->pturnaround = (*p_time-1) - current_process->ptimearrival;
                total_turnaround+=current_process->pturnaround;
                //printf("%d\n",current_process->pturnaround);
                current_node = remove_from_head(&ready);
                if (current_node == NULL) {
                    printf("empty:\n");
                    free(current_node);  // Free the allocated memory if it's NULL
                    break;  
                }
                current_process = current_node->data;
                current_process->presponsetime = (*p_time) - current_process->ptimearrival;
                total_responsetime+=current_process->presponsetime;
 
            } else if (ready.head==NULL && current_process->ptimeleft==1){
                //this is for the last process in the queue since it will not go in the prev else if condition 
                current_process->pturnaround = (*p_time) - current_process->ptimearrival;
                total_turnaround+=current_process->pturnaround;
                //printf("%d\n",current_process->pturnaround);
                current_node=NULL;
                free(current_node);
            }
            printf("%d:%s:", (*p_time), current_process->pname);
            current_process->ptimeleft--;
           // printf("%d\n",current_process->ptimeleft );
            if (!is_empty(&ready)){
                sort_by_timetocompletion(&ready);
                print_q(&ready);
                printf(":\n");
                flag1=1; }
        }
        if (is_empty(&ready) && flag1==0){
            printf("empty:\n");}

        dlq_node *n = p_fq->head;
        while (n!=NULL && n->data->ptimearrival==(*p_time)) {
            dlq_node *node_to_move = remove_from_head(p_fq);
            add_to_tail(&ready, node_to_move);
            sort_by_timetocompletion(&ready);
            n = p_fq->head;
        }
        
 
        (*p_time)++;
    }
    free(current_node);
    avg_throughput = (double)N/(((*p_time)-first_time));
    avg_turnaround = total_turnaround/(double)N;
	avg_responsetime = total_responsetime/(double)N;
    printf("Average throughput: %f\n", avg_throughput);
    printf("Average response time: %f\n", avg_responsetime);
	printf("Average turnaround time: %f\n", avg_turnaround);
}
//implement the RR scheduling code
void sched_RR(dlq *const p_fq, int *p_time)
{
    double avg_responsetime = 0.0;
    double avg_turnaround = 0.0;
	double total_responsetime = 0.0;
    double total_turnaround = 0.0;
    double avg_throughput = 0.0;
    int N = p_fq->size;
    int first_time = 0;

    dlq_node *current_node = malloc(sizeof(dlq_node));
    current_node = remove_from_head(p_fq);
    
    dlq ready;
    //printf("1");
    ready.head = NULL;
    ready.tail = NULL;
    int flag=0;
    
    (*p_time)++;
    while(current_node){
        int flag1=0;
        pcb *current_process = current_node->data;
        if (current_process->ptimearrival!=(*p_time-1) && flag==0){
            printf("%d:idle:", (*p_time));
        } else{
            if (flag==0){
                first_time = (*p_time); 
                current_process->presponsetime = (*p_time) - current_process->ptimearrival;
                total_responsetime+=current_process->presponsetime;
                printf("%d\n",current_process->presponsetime);
            }
            flag=1;
            printf("%d:%s:", (*p_time), current_process->pname);

            if (!is_empty(&ready)){
                print_q(&ready);
                printf(":\n");
                flag1=1; }
      
            if (current_process->ptimeleft>1){ //1 is the time slice, if its been executed for 1 clock tick and time remaining is more than time slice then move to next process
                current_process->ptimeleft--;
                
                add_to_tail(&ready, current_node); //add current back to ready queue
                current_node = remove_from_head(&ready);
                if (current_node == NULL) {
                    printf("empty:\n");
                    free(current_node);  //free the allocated memory if it's NULL
                    break;  
                }
                current_process = current_node->data;
                if (current_process->presponsetime==0){ //if process has finished running
                    current_process->presponsetime = (*p_time+1) - current_process->ptimearrival; //added 1 because we are including the current clock tick as well
                    printf("%d\n",current_process->presponsetime);
                    total_responsetime+=current_process->presponsetime;
                
                }
                
            } else{ //if time remaining is equal to time slice
                // the process has finished running, we can calculate its turnaround time now
                current_process->pturnaround = (*p_time) - current_process->ptimearrival; 
                total_turnaround+=current_process->pturnaround;
                current_node = remove_from_head(&ready);
                if (current_node == NULL) {
                    printf("empty:\n");
                    free(current_node);  // Free the allocated memory if it's NULL
                    break;  
                }
                current_process = current_node->data;
            }
            
        }
        if (is_empty(&ready) && flag1==0){
            printf("empty:\n");}

        dlq_node *n = p_fq->head;
        while (n!=NULL && n->data->ptimearrival==(*p_time)) {
            dlq_node *node_to_move = remove_from_head(p_fq); //temp node 
            add_to_tail(&ready, node_to_move);
            n = p_fq->head;
        }
        
        (*p_time)++;
    }
    free(current_node);
    avg_throughput = (double)N/(((*p_time)-first_time)+1);
    avg_turnaround = total_turnaround/(double)N;
	avg_responsetime = total_responsetime/(double)N;
    printf("Average throughput: %f\n", avg_throughput);
    printf("Average response time: %f\n", avg_responsetime);
	printf("Average turnaround time: %f\n", avg_turnaround);
}

int main()
{
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    int N = 0;
    char tech[20]={'\0'};
    char buffer[100]={'\0'};
    scanf("%d", &N);
    //printf("%d", N);
    scanf("%s", tech);
     
    dlq queue;
    queue.head = NULL;
    queue.tail = NULL;
    for(int i=0; i<N; ++i)
    {   
        scanf("%s\n", buffer); 
        //printf("%s\n", buffer); 
        pcb *p = tokenize_pdata(buffer);
        add_to_tail (&queue, get_new_node(p) );  
    }
    //print_q(&queue);
    
    unsigned int system_time = 0;
 
    sort_by_arrival_time (&queue);

    
    // run scheduler
    if(!strncmp(tech,"FIFO",4))
        sched_FIFO(&queue, &system_time);
    else if(!strncmp(tech,"SJF",3))
        sched_SJF(&queue, &system_time);
    else if(!strncmp(tech,"STCF",4))
        sched_STCF(&queue, &system_time);
    else if(!strncmp(tech,"RR",2))
        sched_RR(&queue, &system_time);
    else
        fprintf(stderr, "Error: unknown POLICY\n");



    
    return 0;
}