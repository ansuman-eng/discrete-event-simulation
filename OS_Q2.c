#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h> 
#include<unistd.h>
sem_t a,b,c,d;// semaphores declaration
int run[9];// which process is running
int pool[4];// contains resources which can be deallocated
int int_cnt[9];// count of how many times that process has occured 
int prev_state[9];
int curr_state[9];
void res_da(int p_no, int count)
{
    int temp=-1;
    for(int i=0;i<4;i++)
        for(int j=0;j<pool[i];j++)		//==============
        {
            temp = rand()%60;
            if(i==0&&(temp<40))
            {
                printf("    Resource A deallocated after instance number %d of process %d\n",count,p_no);
        		pool[0]--;
                sem_post(&a);
            }
            else if(i==1&&(temp<45))
            {
                printf("    Resource B deallocated after instance number %d of process %d\n",count,p_no);
                pool[1]--;
                sem_post(&b);
            }
            else if(i==2&&(temp<36))
            {
                printf("    Resource C deallocated after instance number %d of process %d\n",count,p_no);
                pool[2]--;
                sem_post(&c);
            }
            else if(i==3&&(temp<40))
            {
                printf("    Resource D deallocated after instance number %d of process %d\n",count,p_no);
            	pool[3]--;
                sem_post(&d);
            }
        }
}

void *res_al(void *temp)
{
	
    int p_no= *(int *)temp;
    while(run[p_no]==1);// wait while another instance of this process is running		//======
    run[p_no]=1;
    int count = ++int_cnt[p_no];
    int free = (p_no + 2)%4;
    printf("instance number %d of process with id = %d started\n",count,p_no);
    if(p_no<=4)
    {
        if(free!=0)
        {
        printf("instance number %d of process with id = %d waiting for resource a\n",count,p_no);
        sem_wait(&a);
	    printf("instance number %d of process with id = %d allocated resource a\n",count,p_no);
        }
        if(free!=1)
        {
        printf("instance number %d of process with id = %d waiting for resource b\n",count,p_no);
        sem_wait(&b);
	    printf("instance number %d of process with id = %d allocated resource b\n",count,p_no);
        }
        if(free!=2)
        {
        printf("instance number %d of process with id = %d waiting for resource c\n",count,p_no);
        sem_wait(&c);
	    printf("instance number %d of process with id = %d allocated resource c\n",count,p_no);
        }
        if(free!=3)
        {
        printf("instance number %d of process with id = %d waiting for resource d\n",count,p_no);
        sem_wait(&d);
	    printf("instance number %d of process with id = %d allocated resource d\n",count,p_no);
        }
    	printf("instance number %d of process with id = %d STOPPED\n",count ,p_no);
    }
    free= (p_no)%5;
    if(p_no>4)
    {
        if(free==0)
        {
        printf("instance number %d of process with id = %d waiting for resource d\n",count,p_no);
        sem_wait(&a);
	    printf("instance number %d of process with id = %d allocated resource a\n",count,p_no);
        }
        else if(free==1)
        {
        printf("instance number %d of process with id = %d waiting for resource b\n",count,p_no);
        sem_wait(&b);
	    printf("instance number %d of process with id = %d allocated resource b\n",count,p_no);
        }
        else if(free==2)
        {
        printf("instance number %d of process with id = %d waiting for resource c\n",count,p_no);
        sem_wait(&c);
	    printf("instance number %d of process with id = %d allocated resource c\n",count,p_no);
        }
        else if(free==3)
        {
        printf("instance number %d of process with id = %d waiting for resource d\n",count,p_no);
        sem_wait(&d);
	    printf("instance number %d of process with id = %d allocated resource d\n",count,p_no);
        }
    	printf("instance number %d of process with id = %d STOPPED\n",count ,p_no);
    }
    free = (p_no + 2)%4;
    if(p_no<=4)
    {
        for(int i=0;i<4;i++)
        if(i!=free)
        pool[i]++;
    }
    free= (p_no)%5;
    if(p_no>4)
    {
        pool[free]++;
    }
    res_da(p_no,count);
    curr_state[p_no]++;
    run[p_no]=0;
}

void *deadlock(void * temp)
{
    while(1)
    {
        sleep(2);
	    printf("Checking deadlock\n");
        int boo=1;
        for(int i=1;i<9;i++)
        {
            if(prev_state[i]!=curr_state[i])
            {
                boo=0;
                break;
            }
        }
        if(boo==1)
	{
		printf("deadlock detected\n");
        	exit(0);
	}
        else
        {
            for(int i=1;i<9;i++)
            prev_state[i]=curr_state[i];
        }
    }
}

void main()
{
    int k;
    int va , vb, vc ,vd;
    scanf("%d %d %d %d %d",&k,&va,&vb,&vc,&vd);
    sem_init(&a , 0 , va);  // initialzation of semaphores
    sem_init(&b , 0 , vb);   
    sem_init(&c , 0 , vc);   
    sem_init(&d , 0 , vd);   
    
    pthread_t tid[8*k];// array to store thread ids so that can be joined later 
    int temp[8*k];// variable to determine process number
    for(int i=0;i<8*k;i++)// create all 8*k threads 
    {
        temp[i]= i%8+1;
        pthread_create(&tid[i],NULL,res_al,&temp[i]);
    }
    pthread_t tid_deadlock;
    pthread_create(&tid_deadlock,NULL,deadlock,NULL);
    for(int i=0;i<8*k;i++)
    pthread_join(tid[i],NULL);
}
