#include<stdio.h>
#include<stdlib.h>
#include<string.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
DEFINE TIME QUANTUM AND SCHEDULING HERE

*/
int quantum = 4;
/*Below are all global variables that'll be in use*/
int eheap_size=0;
int fcfs_pheap_size=0;
int size;
struct Event** event_heap;
struct Process** process_heap;
int current_time=0;
int total_wait=0;
int CPU=-1; 				//contains the PID of the process currently in execution. -1 means no process
struct Process * tail=NULL, * head=NULL; // tail and head of roundrobin queue
int rr_pqueue_size;

/*The structures are defined as below*/
struct process_instance
{
	int arrival_time;
	int burst_time; 
};

struct Process{
	int pid;
	char state;				//0 for ready, 1 for running, 2 for terminated
	int arrival_time;
	int cpu_burst;
	int wait_time;
	struct Process* next;
    int last_inserted;
	//char* scheduling_policy;
	//int time_quantum;
	//int preemption;		//0 for no preemption, 1 for preemption
};

struct Event{
	int eventType;			//0 for CPUBurstComplete, 1 for Timeout, 2 for Arrival
	int _time;				//time of occurance
	int pid;				//related to which process
};

/* Below is a comparator function for event heap
It compares two events on the basis of arrivaltime, eventType and PID, in that order.
Following it are standard heap methods
*/
int cmp(int i , int j)
{
    if(event_heap[i]->_time<event_heap[j]->_time)
    return -1;
    else if(event_heap[i]->_time>event_heap[j]->_time)
    return 1;
    else 
    {
        if(event_heap[i]->eventType<event_heap[j]->eventType)
        return -1;
        else if(event_heap[i]->eventType>event_heap[j]->eventType)
        return 1;
        else
        {
            if(event_heap[i]->pid<event_heap[j]->pid)
            return -1;
            else
            return 1;
        }
    }
}

void heapify_up(int i)
{
    //printf("%d\n",i);
    int parent = (i-1)/2;
    //printf("%d\n",parent);
    if(i==0)					//parent doesn't exist
    	return;
    else 						// parent exists 
    {
        if(cmp(parent,i)<0)		// parent value < child
        	return;
        else 					// parent value > child
        {
            struct Event* temp = event_heap[i];
            event_heap[i]=event_heap[parent];
            event_heap[parent]=temp;
            heapify_up(parent);
        }
    }
}

void heapify_down(int i)
{
    int c1 , c2;
    c1 = 2*i+1;
    c2 = 2*i +2;
    if(c1>eheap_size-1)			//No child
    	return;
    else if(c2>eheap_size-1)	//Just one child i.e. left child
    {
        if(cmp(c1,i)<0)			//If current node > child, then swap
        {
            struct Event* temp = event_heap[i];
            event_heap[i]=event_heap[c1];
            event_heap[c1]=temp;
            heapify_down(c1);
        }
        else
        	return;
    }
    else if(cmp(c1,i)<0||cmp(c2,i)<0)	//If any of the children is smaller
    {
        if(cmp(c1,c2)<0)				//left child is smallest
        {
            struct Event* temp = event_heap[i];
            event_heap[i]=event_heap[c1];
            event_heap[c1]=temp;
            heapify_down(c1);
        }
        else							//right child is smallest
        {
            struct Event* temp = event_heap[i];
            event_heap[i]=event_heap[c2];
            event_heap[c2]=temp;
            heapify_down(c2);
        }
    }
    else
    return;
}

struct Event* eheap_remove_top()
{
    struct Event* top=event_heap[0];
    eheap_size--;
    event_heap[0]=event_heap[eheap_size];
    heapify_down(0);
    return top;
}

void eheap_insert(struct Event* temp)
{
    //printf("Inserting event....\n");
    event_heap[eheap_size]=temp;
    eheap_size++;
    heapify_up(eheap_size-1);
    //printf("Inserted event.\n");
}

void eheap_build_max_heap()
{
    for(int i=(eheap_size-1)/2;i>=0;i--)
    heapify_down(i);
}

/* Below is a comparator function for process heap
It compares two events on the basis of arrivaltime and PID, in that order.
Following it are standard heap methods
*/
int cmp1(int i , int j)
{
    if(process_heap[i]->arrival_time<process_heap[j]->arrival_time)
    return -1;
    else if(process_heap[i]->arrival_time>process_heap[j]->arrival_time)
    return 1;
    else
    {
        if(process_heap[i]->pid<process_heap[j]->pid)
        return -1;
        else
        return 1;
    }
}

void fcfs_p_heapify_up(int i)
{
    int parent = (i-1)/2;
    if(i==0)
    	return;
    else // parent exists 
    {
        if(cmp1(parent,i)<0)// parent value < child
        	return;
        else // parent value > child
        {
            struct Process* temp = process_heap[i];
            process_heap[i]=process_heap[parent];
            process_heap[parent]=temp;
            fcfs_p_heapify_up(parent);
        }
    }
}

void fcfs_p_heapify_down(int i)
{
    int c1 , c2;
    c1 = 2*i+1;
    c2 = 2*i +2;
    if(c1>fcfs_pheap_size-1)
    return;
    else if(c2>fcfs_pheap_size-1)
    {
        if(cmp1(c1,i)<0)
        {
            struct Process* temp = process_heap[i];
            process_heap[i]=process_heap[c1];
            process_heap[c1]=temp;
            fcfs_p_heapify_down(c1);
        }
        else
        return;
    }
    else if(cmp1(c1,i)<0||cmp1(c2,i)<0)
    {
        if(cmp1(c1,c2)<0)
        {
            struct Process* temp = process_heap[i];
            process_heap[i]=process_heap[c1];
            process_heap[c1]=temp;
            fcfs_p_heapify_down(c1);
        }
        else
        {
            struct Process* temp = process_heap[i];
            process_heap[i]=process_heap[c2];
            process_heap[c2]=temp;
            fcfs_p_heapify_down(c2);
        }
    }
    else
    return;
}

struct Process* fcfs_pheap_remove_top()
{
   struct Process* top=process_heap[0];
   fcfs_pheap_size--;
   process_heap[0]=process_heap[fcfs_pheap_size];
   printf("Process %d removed from FCFS Process Heap. Size is %d\n",top->pid,fcfs_pheap_size);
   fcfs_p_heapify_down(0);
   return top;
}

void fcfs_pheap_insert(struct Process* temp)
{
    process_heap[fcfs_pheap_size]=temp;
    fcfs_pheap_size++;
    printf("Process %d inserted in FCFS Process Heap. Size is %d\n",temp->pid,fcfs_pheap_size);
    fcfs_p_heapify_up(fcfs_pheap_size-1);
}

void fcfs_pheap_build_max_heap()
{
    for(int i=(fcfs_pheap_size-1)/2;i>=0;i--)
    	fcfs_p_heapify_down(i);
}

/*Below is a comparator function for the sorting of process list*/
int comparator(const void *p, const void *q)
{
	int first_arriv=(*((struct process_instance*)p)).arrival_time;
	int first_burst=(*((struct process_instance*)p)).burst_time;
	int second_arriv=(*((struct process_instance*)q)).arrival_time;
	int second_burst=(*((struct process_instance*)q)).burst_time;

	if(first_arriv!=second_arriv)
	{
		return first_arriv-second_arriv;
	}
	else
	{
		return first_burst-second_burst;
	}
}

/*Below are the methods to handle the RoundRobin Queue*/
struct Process * rr_pqueue_remove()
{
    
    struct Process * temp;
    temp = head;
    head = head->next;
    rr_pqueue_size--;
    printf("Process %d removed from RR Queue\n", temp->pid);
    return temp;
}

void rr_pqueue_insert(struct Process * temp)
{
    printf("Process %d inserted in RR Queue\n", temp->pid);
    if(head==NULL && tail==NULL)
    {
        head=temp;
        tail=temp;
        temp->next=NULL;
    }
    temp->next=NULL;
    tail->next= temp;
    tail = temp;
    rr_pqueue_size++;
}

/*Generate new Event/Process instances from below*/
struct Event* newEvent(int pid, int type, int _time)
{
	struct Event* temp=(struct Event*)malloc(sizeof(struct Event));
	temp->eventType=type;				//0 for CPUBurstComplete, 1 for Timeout, 2 for Arrival
	temp->pid=pid;						//PID for the associated Process
	temp->_time=_time;					//Time of the event
	return temp;
}

struct Process* newProcess(int pid, char state, int arrival_time, int cpu_burst,int last_inserted)
{
	struct Process* temp=(struct Process*)malloc(sizeof(struct Process));
	temp->pid=pid;						//PID of the process
	temp->state=state;					//'0' for Ready, '1' for Running
	temp->arrival_time=arrival_time;	//Arrival Time for the Process
	temp->cpu_burst=cpu_burst;			//Burst Time for the Process
	temp->wait_time=0;					//Wait Time for the Process
    temp->last_inserted=last_inserted;              //Time at which process was last inserted into ready queue

	return temp;
}


int main()
{
	struct process_instance process_table[20];
	FILE* stream=fopen("process1.csv","r");
	char *field1;
	char *field2;
	char line[1024];
	int i=0;
	while(fgets(line,1024,stream))
	{
		char* tmp=strdup(line);
		field1=strtok(line,",");
		field2=strtok(NULL,",\n");
		//printf("%s %s\n",field1,field2);
		process_table[i].arrival_time=atoi(field1);
		process_table[i].burst_time=atoi(field2);
		i++;
		free(tmp);
	}
	size=i;
	event_heap=(struct Event **)malloc(size*3*sizeof(struct Event *));					//An array of pointers, with size = 3*size, since each process can have 3 events
	process_heap=(struct Process **)malloc(size*sizeof(struct Process *));				//An array of pointers, with size = size, since each process can be a single entry
	qsort((void*)process_table,size,sizeof(process_table[0]),comparator);				//Sort the process list and make it into process table. Index=PID
	printf("Printing Process Table\n");
	for(i=0;i<size;i++)
	{
		printf("PID: %d Arrival: %d Burst: %d\n",i,process_table[i].arrival_time,process_table[i].burst_time);	//Debug
		struct Event* first_event=newEvent(i,2,process_table[i].arrival_time);				//Event corresponding to the very first process arrival
		eheap_insert(first_event);															//Enter the very first process as an arrival event
	}
	
	
	struct Event* current_event;
	struct Process* current_process;
	while(eheap_size>0)
	{
		current_event=eheap_remove_top();
		printf("PID: %d Event_Type: %d Occurance_Time: %d\n",current_event->pid,current_event->eventType,current_event->_time);
		current_time=current_event->_time;
		if(current_event->eventType==0)
		{

			if(CPU==current_event->pid)                  //CHECK IF THE PROCESS CORRESPONDING TO THE EVENT IS THERE IN THE CPU OR NOT
            {
                printf("CPU Burst Completed for %d\n", current_process->pid);
    			current_process->state='2';         //Terminated
    			total_wait+=current_process->wait_time;
                printf("Wait time for %d is %d\n", current_process->pid,current_process->wait_time);
    			if(rr_pqueue_size>0)
    			{
    				
    				current_process=rr_pqueue_remove();
                    printf("Starting to execute %d ....\n", current_process->pid);
                    CPU=current_process->pid;                                                                       //Set the current process to execution
                    current_process->state='1';                                                                     //Current time - time of entry to queue must be wait time
                    current_process->wait_time+=(current_time-current_process->last_inserted);                      //Calculate how long was it in the queue

                    int executed_for=((current_time-current_process->arrival_time)-current_process->wait_time);     //How much time has it occupied CPU
                    int leftover_burst=(current_process->cpu_burst - executed_for);                                 //How much burst is left
                    int completion_time=current_time+leftover_burst;                                                //Time for CPUBurst Completion

                    if(leftover_burst<=quantum)
                    {
                        struct Event* cpu_burst_event=newEvent(current_process->pid,0,completion_time);             //Make a terminating event and insert in heap
                        eheap_insert(cpu_burst_event);
                    }
                    else
                    {
                        struct Event* timer_event=newEvent(current_process->pid,1,current_time+quantum);            //Make a timer condition
                        eheap_insert(timer_event); 
                    }
    			}
    			else																				//No process has arrived yet
    			{
    				if(fcfs_pheap_size>0)
    				{
    				    //Take process from fcfs queue
    				    current_process=fcfs_pheap_remove_top();												//Grab the process with minimum arrival time (FCFS). Break tie with PID
        				printf("Starting to execute %d ....\n", current_process->pid);
                        //printf("CURRENT TIME %d, ARRIVAL TIME %d\n",current_time,current_process->arrival_time);
        				CPU=current_process->pid;														//Set the current process to execution
        				current_process->state='1';														//Status changed to running
        				current_process->wait_time+=(current_time-current_process->last_inserted);			//Current time - time of entry to queue must be wait time
        				
                        int executed_for=((current_time-current_process->arrival_time)-current_process->wait_time);     //How much time has it occupied CPU
                        int leftover_burst=(current_process->cpu_burst - executed_for);                                 //How much burst is left
                        int completion_time=current_time+leftover_burst;                                                //Time for CPUBurst Completion


        				struct Event* cpu_burst_event=newEvent(current_process->pid,0,completion_time);	//Make a terminating event and insert in heap
        				eheap_insert(cpu_burst_event);
    				}
    				else
    				{
    				    CPU=-1;              //Turn CPU state to idle
    				}
    																			
    			}
            }									
			
		}
		else if(current_event->eventType==1)
		{
			printf("Time Out\n");
            printf("CPU was executing %d, timeout is for %d\n", CPU,current_event->pid);
			//Print waittime for the process
			//This will never happen in FCFS
			if(CPU==current_event->pid)    //Redundant                                                                        //Time out occurs when the process is running
                {
                    //printf("Time Out\n");                                                                           //Give the alert
                    current_process->state='0'; //ready queue                                                                     //Ready State
                    current_process->last_inserted=current_time;    
                    rr_pqueue_insert(current_process);                                                                  //Insert the running process into the ready queue
                    current_process=rr_pqueue_remove();                                                             //Remove a process using RR

                    current_process->wait_time+=(current_time-current_process->last_inserted);                      //Calculate how long was it in the queue
                    CPU=current_process->pid;

                    int executed_for=((current_time-current_process->arrival_time)-current_process->wait_time);     //How much time has it occupied CPU
                    int leftover_burst=(current_process->cpu_burst - executed_for);                                 //How much burst is left
                    int completion_time=current_time+leftover_burst;                                                //Time for CPUBurst Completion

                    if(leftover_burst<=quantum)
                    {
                        struct Event* cpu_burst_event=newEvent(current_process->pid,0,completion_time);             //Make a terminating event and insert in heap
                        eheap_insert(cpu_burst_event);
                    }
                    else
                    {
                        struct Event* timer_event=newEvent(current_process->pid,1,current_time+quantum);            //Make a timer condition
                        eheap_insert(timer_event); 
                    }   
                }
            //else do nothing because process has already terminated, no use of this timer event
            //Print waittime for the process
            //This will never happen in FCFS
		}
		else
		{
			printf("Arrival event\n");
			//When an arrival event occurs with idle CPU, add the CPU Burst Completion event for that PID too
			int arrival_time=process_table[current_event->pid].arrival_time;
			int burst_time=process_table[current_event->pid].burst_time;
			if(burst_time>8)
			{
			    //FCFS
			    if(CPU==-1)
    			{
    				int completion_time = arrival_time + burst_time;								//Only holds for the processes which came while CPU was idle
    				struct Event* cpu_burst_event=newEvent(current_event->pid,0,completion_time);	//Make a terminating event and insert in heap
    				eheap_insert(cpu_burst_event);
    				current_process=newProcess(current_event->pid,'1',arrival_time,burst_time,current_time);		//we make a processfor completion but don't add it to queue
    				CPU=current_process->pid;															//Since CPU was idle, the process is sent for execution
    			}
    			else
    			{
    				struct Process* temp= newProcess(current_event->pid,'0',arrival_time,burst_time,current_time);
    				fcfs_pheap_insert(temp);
    			}

			}
			else
			{
			    //RR
			    if(CPU==-1 && burst_time<=quantum)
                {
                    int completion_time = arrival_time + burst_time;                                //Only holds for the processes which came while CPU was idle
                    struct Event* cpu_burst_event=newEvent(current_event->pid,0,completion_time);   //Make a terminating event and insert in heap
                    eheap_insert(cpu_burst_event);
                    current_process=newProcess(current_event->pid,'1',arrival_time,burst_time,current_time);     //we make a processfor completion but don't add it to queue
                    CPU=current_event->pid;                                                         //Since CPU was idle, the process is sent for execution
                    printf("Execution started for %d and Completion Event pushed\n", current_event->pid);
                }
                else if(CPU==-1)
                {
                    struct Event* timer_event=newEvent(current_event->pid,1,current_time+quantum);    //Make a timer condition and insert in heap
                    eheap_insert(timer_event);
                    current_process=newProcess(current_event->pid,'1',arrival_time,burst_time,current_time);     //we make a processfor completion but don't add it to queue
                    CPU=current_event->pid;                                                         //Since CPU was idle, the process is sent for execution
                    printf("Execution started for %d and Timer Event pushed\n", current_event->pid);
                }
                else
                {                
                    //IF A FCFS PROCESS IS RUNNING CURRENTLY, THROW IT OUT

                    if(current_process->cpu_burst>8)
                    {
                        printf("\n");
                        printf("FCFS WAS RUNNING. THROWING IT OUT.\n");
                        printf("\n");

                        current_process->state='0';                                                                          //Ready State
                        current_process->last_inserted=current_time;   
                        
                        fcfs_pheap_insert(current_process);                                                                  //Insert the running process into the ready queue
                        if(burst_time<=quantum)
                        {
                            int completion_time = arrival_time + burst_time;                                //Only holds for the processes which came while CPU was idle
                            struct Event* cpu_burst_event=newEvent(current_event->pid,0,completion_time);   //Make a terminating event and insert in heap
                            eheap_insert(cpu_burst_event);
                            current_process=newProcess(current_event->pid,'1',arrival_time,burst_time,current_time);     //we make a processfor completion but don't add it to queue
                            CPU=current_event->pid;                                                         //Since CPU was idle, the process is sent for execution
                            printf("Execution started for %d and Completion Event pushed\n", current_event->pid);
                        }
                        else
                        {
                            struct Event* timer_event=newEvent(current_event->pid,1,current_time+quantum);    //Make a timer condition and insert in heap
                            eheap_insert(timer_event);
                            current_process=newProcess(current_event->pid,'1',arrival_time,burst_time,current_time);     //we make a processfor completion but don't add it to queue
                            CPU=current_event->pid;                                                         //Since CPU was idle, the process is sent for execution
                            printf("Execution started for %d and Timer Event pushed\n", current_event->pid);
                        }

                    }
                    //If it reaches here, it means that RR process was running
                    else
                    {
                        struct Process* temp= newProcess(current_event->pid,'0',arrival_time,burst_time,current_time);
                        temp->last_inserted=current_time;
                        rr_pqueue_insert(temp);
                    }
                    
                    
                }

			    
			}
			
		}

	}

    printf("Total wait time %d\n", total_wait);
    printf("AWT is %f\n",((float)total_wait)/size);
	printf("Current Time %d\n", current_time);

}

