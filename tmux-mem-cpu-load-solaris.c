/**
 * tmux-mem-cpu-load-solaris v0.1
 * written by Dmitry Geurkov (troydm) <d.geurkov@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <stdio.h>
#include <sys/sysinfo.h>
#include <kstat.h>
#include <errno.h>
#include <unistd.h>

static kstat_ctl_t *kc = NULL;

struct cpu_stat_info {
    unsigned long user1, user2;
    unsigned long idle1, idle2;
    unsigned long kernel1, kernel2;
    unsigned long wait1, wait2;
};

void getcpustat(unsigned int interval, char* msg, int graphs, int showload){
    
    kstat_t *ks = NULL;

    // Count cpus
    unsigned int i = 0;
    unsigned int cpus = 0;
    while(1){
        char cpu_stat_id[60];
        sprintf(cpu_stat_id,"cpu_stat%d",i);
        if((ks = kstat_lookup(kc, "cpu_stat", -1, cpu_stat_id)) != NULL){              
            cpus++;
            i++;
        }else{
            break;
        }
    }

    // Allocate struct to hold cpus info
    struct cpu_stat_info *cpu_infos = malloc(cpus * sizeof(struct cpu_stat_info));

    i = 0;
    while(1){
        char cpu_stat_id[60];
        sprintf(cpu_stat_id,"cpu_stat%d",i);
        if((ks = kstat_lookup(kc, "cpu_stat", -1, cpu_stat_id)) != NULL){              
            kstat_read(kc, ks, 0);
            cpu_stat_t *kscpu = ((cpu_stat_t *) ks->ks_data);
            cpu_infos[i].idle1 = kscpu->cpu_sysinfo.cpu[CPU_IDLE];
            cpu_infos[i].user1 = kscpu->cpu_sysinfo.cpu[CPU_USER];
            cpu_infos[i].kernel1 = kscpu->cpu_sysinfo.cpu[CPU_KERNEL];
            cpu_infos[i].wait1 = kscpu->cpu_sysinfo.cpu[CPU_WAIT];
        }else{
            break;
        }
        i = i+1;
    }

    usleep(interval*1000000);
  
    i = 0;
    while(1){
        char cpu_stat_id[60];
        sprintf(cpu_stat_id,"cpu_stat%d",i);
        if((ks = kstat_lookup(kc, "cpu_stat", -1, cpu_stat_id)) != NULL){              
            kstat_read(kc, ks, 0);
            cpu_stat_t *kscpu = ((cpu_stat_t *) ks->ks_data);
            cpu_infos[i].idle2 = kscpu->cpu_sysinfo.cpu[CPU_IDLE];
            cpu_infos[i].user2 = kscpu->cpu_sysinfo.cpu[CPU_USER];
            cpu_infos[i].kernel2 = kscpu->cpu_sysinfo.cpu[CPU_KERNEL];
            cpu_infos[i].wait2 = kscpu->cpu_sysinfo.cpu[CPU_WAIT];
        }else{
            break;
        }
        i = i+1;
    }

    double total_idle = 0;
    double total_user = 0;
    double total_kernel = 0;
    double total_wait = 0;
    
    for(i = 0;i < cpus; i++){
        double idle = 100*(cpu_infos[i].idle2-cpu_infos[i].idle1);
        double user = 100*(cpu_infos[i].user2-cpu_infos[i].user1);
        double kernel = 100*(cpu_infos[i].kernel2-cpu_infos[i].kernel1);
        double wait = 100*(cpu_infos[i].wait2-cpu_infos[i].wait1);

        double load = (idle+user+kernel+wait)/100;
        idle /= load;
        user /= load;
        kernel /= load;
        wait /= load;

        total_idle += idle;
        total_user += user;
        total_kernel += kernel;
        total_wait += wait;
    }

    total_idle /= cpus;
    total_user /= cpus;
    total_kernel /= cpus;
    total_wait /= cpus;

    if(graphs>0){
        double total_load = 100-total_idle;
        
        char loadb[10];
        sprintf(loadb,"%3.1f%%",total_load);
        unsigned int loadl = strlen(loadb);
        
        double gs = 100/graphs;
        total_load += gs/2;

        for(i=0;i<graphs;i++){
            msg[i+1] = ' ';
        }

        unsigned loadp = (graphs-loadl)/2;
        for(i=0;i<loadl;i++){
            msg[1+loadp+i] = loadb[i];
        }
        
        msg[0] = '[';
        double ga = 0;
        for(i=0;i<graphs;i++){
            ga += gs;
            if(total_load >= ga && msg[i+1] == ' ') 
                msg[i+1] = '|';
        }
        msg[i] = ']';
        msg[i+1] = 0;
    }else{
        sprintf(msg,"I:%3.1f%% U:%3.1f%% K:%3.1f%% W:%3.1f%%", total_idle,total_user,total_kernel,total_wait);
    }

    free(cpu_infos);

    if(showload){
        double avg_1m = 0;
        double avg_5m = 0;
        double avg_15m = 0;

        // Calculate load averages for 1m 5m 15m
        if((ks = kstat_lookup(kc, "unix", -1, "system_misc")) != NULL){              
            kstat_read(kc, ks, 0);
            kstat_named_t *kn = kstat_data_lookup(ks, "avenrun_1min");
            avg_1m = ((double)kn->value.ui32/(1<<8)); 
            kn = kstat_data_lookup(ks, "avenrun_5min");
            avg_5m = ((double)kn->value.ui32/(1<<8)); 
            kn = kstat_data_lookup(ks, "avenrun_15min");
            avg_15m = ((double)kn->value.ui32/(1<<8)); 
        }

        sprintf(msg+strlen(msg)," %3.2f %3.2f %3.2f",avg_1m,avg_5m,avg_15m);
    }
}

void getmem(char* msg){
    // Get system page size
    int pagesize = sysconf(_SC_PAGESIZE)/1024;
    // Lookup system_pages kstat node
    kstat_t *ks = kstat_lookup(kc, "unix", 0, "system_pages");
    kstat_read(kc, ks, 0);
    // Get free memory
    kstat_named_t *kn = kstat_data_lookup(ks, "freemem");
    int freemem = kn->value.ui32; 
    freemem = (pagesize*freemem)/1024;
    // Get physical memory
    kn = kstat_data_lookup(ks, "physmem");
    int physmem = kn->value.ui32;
    physmem = (pagesize*physmem)/1024;
    sprintf(msg,"%d/%dMB",freemem,physmem);
}

char* getarg(int argc,char** argv, char* arg){
    int i = 0;
    for(i=1;i<argc;i++){
        char* s = strstr(argv[i],arg);
        if(s != NULL)
            return s;
    }

    return NULL;
}

int getargint(int argc,char** argv, char* arg){
    int len = strlen(arg);
    arg = getarg(argc,argv,arg);
    if(arg != NULL){
        arg = (char*)(arg+len);
        return atoi(arg);
    }

    return 0;
}

int main(int argc, char** argv){
    // Open kstat
    if ((kc = kstat_open()) == NULL){
	fprintf(stderr, "Unable to open kstat.\n");
	return(-1);
    }

    // Check memory option
    int showmem = 1;
    if(getarg(argc,argv,"-mem") != NULL)
        showmem = 0; 

    char msg[512];
    int l = 0;
    if(showmem){
        getmem(msg);
        l = strlen(msg);
        msg[l] = ' ';
        l += 1;
    }

    // Check cpu options
    int interval = 3;
    int graphs = 0;
    int load = 1;
    
    if(getarg(argc,argv,"-load") != NULL)
        load = 0;

    int argint = getargint(argc,argv,"--interval=");
    if(argint > 0)
        interval = argint;

    int arggrp = getargint(argc,argv,"--graphs=");
    if(arggrp > 0)
        graphs = arggrp;

    getcpustat(interval,msg+l,graphs,load);
    
    printf("%s",msg);
    
    kstat_close(kc);
}
