#include<iostream>
#include<queue>
#include<algorithm>
using namespace std;
class jobNode{
    public:
    int job_id,start_time,runtime,memory,device,priority,Finish_time,turn_arround_time,device_hold,time_executed;
    jobNode* next;
    jobNode(int job,int time,int dev,int mem,int p,int run){
        job_id = job;
        start_time = time;
        device=dev;
        memory=mem;
        priority = p;
        device_hold=0;
        runtime=run;
        Finish_time=0;
        time_executed=0;
        next = NULL;
    }
};

queue<jobNode*> run_process,wait_process;
vector<jobNode*> completed;

class Hold1{
    public:
    jobNode* head;
    Hold1(){
        head=NULL;
    }
    void add(jobNode* j){
        if(head==NULL) {  head= j; return;}
            if(head->runtime > j->runtime)  {
                j->next=head;
                head= j;
                return ;
            } else {
                head->next =j;
               return;
            };
            jobNode* temp=head;
        while(temp->next!=NULL){
             if(temp->next->runtime>j->runtime){
                 j->next = temp->next;
                 temp->next = j;
                 return;
             }temp=temp->next;

        }
        temp->next = j;
    }
};
class Hold2{
    public:
    jobNode* head;
    Hold2(){
        head=NULL;
    }
    void add(jobNode* j){
        if(head==NULL) {
            head= j;
            return;
        }
       jobNode* temp=head;
        while(temp->next!=NULL){
        temp=temp->next;
        }
        temp->next = j;
    }
};

Hold1 h1;
Hold2 h2;

void run_till_time(int time,int &remaining_memory,int &resource_remaining,int time_quantum,int& start_time){ //function that run till the time current command come
    //adding into run queue
    while(!wait_process.empty()){  //check in wait queue first;
        jobNode* front = wait_process.front();
        if(front->device<=resource_remaining){
            wait_process.pop();
            run_process.push(front);
        }else{
            break;
        }
    }
    while(h1.head!=NULL && h1.head->start_time<=time && h1.head->memory>=remaining_memory){ //check for holded process due to resources
        run_process.push(h1.head);
        remaining_memory-= h1.head->memory;
        h1.head=h1.head->next;
    }while(h2.head!=NULL && h2.head->memory>=remaining_memory && h2.head->start_time<=time ){ //in h2 too
        run_process.push(h2.head);
        remaining_memory-= h2.head->memory;
        h2.head=h2.head->next;
    }
    //process sheduling
    int till_time= time;
    int time_last_process_out=start_time;
    while(!run_process.empty() && till_time>time_last_process_out){ 
          jobNode* front = run_process.front();
          run_process.pop();
          if(front->runtime < time_quantum){
             front->time_executed+=front->runtime;
             completed.push_back(front);
             resource_remaining+=front->device_hold;
             front->device+=front->device_hold;
             front->device_hold=0;
             remaining_memory+=front->memory;
             front->Finish_time =time_last_process_out+front->runtime;
             time_last_process_out = front->Finish_time;
          }else if(front->runtime == time_quantum){
              front->time_executed+=time_quantum;
              completed.push_back(front);
              resource_remaining+=front->device_hold;
              front->device+=front->device_hold;
              front->device_hold=0;
              front->Finish_time= time_last_process_out+time_quantum;
              time_last_process_out = front->Finish_time;
              remaining_memory+=front->memory;
          }else{
           front->time_executed+=time_quantum;
           front->Finish_time= time_last_process_out+time_quantum;
           time_last_process_out = front->Finish_time;
           front->runtime-=time_quantum;
           run_process.push(front);
          }
    }
    start_time = time_last_process_out;
    // cout<<till_time<<" "<<time_last_process_out<<endl;
    if(till_time>time_last_process_out ){
        if(run_process.empty() && wait_process.empty() && h1.head==NULL && h2.head==NULL)
        cout<<"Done"<<endl;
        else
        run_till_time(time,remaining_memory,resource_remaining,time_quantum,start_time);
    }
}

void job_come(jobNode* job,int &remaining_memory,int& resource_remaining,int time_quantum,int &current_time,int total_memory){  //when a job come needed to add in hold or run queue
   if(job->memory>total_memory){
     cout<<"job rejected"<<endl;
   }else{
       int firstvalue=current_time;
       if(job->memory<=remaining_memory ){
           run_process.push(job);
           remaining_memory-=job->memory;
       }else{
           if(job->priority==1) h1.add(job);
           else h2.add(job);
       }
       if(run_process.size()==1){ current_time = run_process.front()->start_time;}
       run_till_time(job->start_time,remaining_memory,resource_remaining,time_quantum,current_time);
   }
}
void request_devices(int time,int id,int num_of_devices,int &remaining_memory,int &resource_remaining, int time_quantum,int &start_time){ //function to request device if any process needs
   run_till_time(time,remaining_memory,resource_remaining,time_quantum,start_time);
    cout<<"Current Available Main Memory :"<<remaining_memory<<endl;
    cout<<"Current Devices :"<<resource_remaining<<endl;
   queue<jobNode*> copy_run =run_process;
   while(!copy_run.empty()){   //check in running queue for the given id 
       jobNode* front = copy_run.front();
       copy_run.pop();
       if(front->job_id==id && num_of_devices<=resource_remaining){
           front->device_hold+=num_of_devices; //provide resources to them 
           front->device-=num_of_devices;
           resource_remaining-=num_of_devices;  
       }else if(front->job_id==id){
           wait_process.push(front);
           queue<jobNode*> copy_run_new =run_process;
           while(!run_process.empty()) run_process.pop();
            while(!copy_run_new.empty()){   //check in running queue for the given id 
              jobNode* curr = copy_run_new.front();
              copy_run_new.pop();
              if(curr->job_id==front->job_id) continue;
              run_process.push(curr);
            }
   }
 }
}
void release_devices(int time,int id, int num_of_devices,int &remaining_memory,int& resource_remaining, int time_quantum,int &start_time){ //function to release device if any process has
      run_till_time(time,remaining_memory,resource_remaining,time_quantum,start_time);
      queue<jobNode*> copy_run =run_process;
      while(!copy_run.empty()){
       jobNode* front = copy_run.front();
       copy_run.pop();
       if(front->job_id==id){
           front->device_hold-=num_of_devices; //release resources of the given job id 
           front->device+=num_of_devices;
           resource_remaining+=num_of_devices;  
       }
     }
}
void print(int time,int &remaining_memory,int &resource_remaining,int time_quantum,int &start_time){
    run_till_time(time,remaining_memory,resource_remaining,time_quantum,start_time);
    cout<<"Completed one"<<endl;
    cout<<"--------------------------------------------------------"<<endl;
    cout<<"Job ID    Arrival Time    Finish Time    Turnaround Time"<<endl;
    sort(completed.begin(),completed.end());
     for(auto x: completed){
         cout<<"   "<<x->job_id<<"           "<<x->start_time<<"               "<<x->Finish_time<<"             "<<x->Finish_time-x->start_time<<endl;
     }
    
     jobNode* temp = h1.head;
     cout<<"Hold Queue 1:"<<endl;
     cout<<"-------------------------"<<endl;
     cout<<"Job ID    Run Time"<<endl;
     cout<<"========================="<<endl;
     while(temp!=NULL){
         cout<<"   "<<temp->job_id<<"         "<<temp->runtime<<endl;
         temp=temp->next;
     }
     cout<<"Hold Queue 2:"<<endl;
     cout<<"-------------------------"<<endl;
     cout<<"Job ID    Run Time"<<endl;
     cout<<"========================="<<endl;
     jobNode* temp2 = h2.head;
     while(temp2!=NULL){
           cout<<"   "<<temp2->job_id<<"         "<<temp2->runtime<<endl;
           temp2=temp2->next;
     }
    cout<<"Ready Queue:"<<endl;
    cout<<"----------------------------------"<<endl;
    cout<<"Job ID    Run Time    Time Accrued"<<endl;
    cout<<"=================================="<<endl;
    int id_of_runing=-1,time_of_running,left_of_running,finish_time;
     queue<jobNode*> copy_run =run_process;
      while(!copy_run.empty()){
       jobNode* front = copy_run.front();
       if(front->Finish_time>=time) {
           finish_time=front->Finish_time;
           id_of_runing = front->job_id;
           time_of_running = front->time_executed;
           left_of_running=front->runtime;
           copy_run.pop();
           continue;
       }
       cout<<"  "<<front->job_id<<"         "<<front->runtime <<"            "<<front->time_executed<<endl;
       copy_run.pop();
     }
     cout<<"Process Running on CPU:"<<endl;
    cout<<"----------------------------------"<<endl;
    cout<<"Job ID    Time Accrued    Run Time"<<endl;
    cout<<"=================================="<<endl;
    if(id_of_runing!=-1)
    {
        if(finish_time>time){
               cout<<"  "<<id_of_runing<<"         "<<time_of_running-(finish_time-time)<<"            "<<left_of_running+(finish_time-time)<<endl;
        }else
      cout<<"  "<<id_of_runing<<"         "<<time_of_running<<"            "<<left_of_running<<endl;
    }
     cout<<"Wait Queue:"<<endl;
    cout<<"----------------------------------"<<endl;
    cout<<"Job ID    Run Time    Time Accrued"<<endl;
    cout<<"=================================="<<endl;
     queue<jobNode*> copy_wait = wait_process;
     while(!copy_wait.empty()){
         jobNode* front = copy_wait.front();
        cout<<"  "<<front->job_id<<"         "<<front->Finish_time<<"            "<<front->runtime<<endl;
         copy_wait.pop();
     }
   
}
int main(){
    char a;
    a='a';
    int current_time,serial_devices,current_memory,time_quantum;
    cout<<"Enter System Configuration\n";
    cin>>current_time>>current_memory>>serial_devices>>time_quantum;
    int remaining_memory = current_memory;
    int resource_remaining=serial_devices;
        while(a!='z'){
            cout<<"A for add process"<<endl;
            cout<<"Q for request Resources"<<endl;
            cout<<"L for release Resources"<<endl;
            cout<<"D for Print Resources"<<endl;
            cin>>a;
            switch(a){
                    case 'A':
                           {
                            int start_time,job_id,memory_needed,serial_devices_need,runtime, priority;
                            cout<<"Enter Process Details  "<<endl;
                            cin>>start_time>>job_id>>memory_needed>>serial_devices_need>>runtime>>priority;
                            jobNode* j = new jobNode(job_id,start_time,serial_devices_need,memory_needed,priority,runtime);
                            job_come(j,remaining_memory,resource_remaining,time_quantum,current_time,current_memory); 
                            break;
                           }
                    case 'Q':
                           {
                            int num_of_devices, job_id_to_Req,at_time;
                            cout<<"Resource request"<<endl;
                            cin>>at_time>>job_id_to_Req>>num_of_devices;
                            request_devices(at_time,job_id_to_Req,num_of_devices,remaining_memory,resource_remaining,time_quantum,current_time);
                            break;
                           }
                    case  'L':
                            {
                            int num_of_devices, job_id_to_Req,at_time;
                            cout<<"Release of devices"<<endl;
                            cin>>at_time>>job_id_to_Req>>num_of_devices;
                            release_devices(at_time,job_id_to_Req,num_of_devices,remaining_memory,resource_remaining, time_quantum,current_time);
                            break;
                            }
                    case  'D' :
                    {
                            int at_time;
                            cin>>at_time;
                            cout<<"At time "<<at_time<<endl;
                           
                            print(at_time,remaining_memory,resource_remaining,time_quantum,current_time);
                            break;
                    }
                    default:
                        a='z';
                }
        }


}