#include <iostream>
#include "hw2_output.h"
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <ctime>
struct SoldierType {
    int NProperPrivates;
    int NSmokers;
};
std::vector<std::vector<SoldierType>> soldiers;

struct Coord {
    int x_coord;
    int y_coord;
};
class ProperPrivate {
public:
    int gid;
    int s_i;
    int s_j;
    int t_delay;
    int n_g;
    int semaphore_index;
    struct timespec ts;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
    std::vector<std::vector<int>> startingPts;
    std::vector<Coord> locked_areas;
    void unlocking(){
        for (auto & locked_area : locked_areas) {
            soldiers[locked_area.x_coord][locked_area.y_coord].NProperPrivates --;
        }
        locked_areas.clear();
    }

};
struct Commands{
    std::string command;
    int time;
};


class Commander{
public:
    std::vector<Commands> commands;
};
class Smoker {
public:
    int sid;
    int t_s;
    int n_s;
    std::vector<std::vector<Coord>> flickingPoints;
    std::vector<Coord> center_of_smoking;
    std::vector<int> cigaretteCounts;
    struct timespec smoker_timespec;
    int cond_var_index;
    int flag = 0;
    int flick_index = 0;
    std::vector<Coord> locked_areas_smoker;
    pthread_mutex_t smoker_object_mutex = PTHREAD_MUTEX_INITIALIZER;
    void unlocking_smoker(){
        for (auto & i : locked_areas_smoker) {
            soldiers[i.x_coord][i.y_coord].NSmokers --;

        }
        locked_areas_smoker.clear();
    }
};

int x_dim,y_dim;
std::vector<std::vector<int>> grid;
std::vector<std::vector<int>> demo;
int privateCount,smokerCount = 0,orderCount = 0;
std::vector<ProperPrivate> properPrivates;
std::vector<Smoker> sneakySmokers;
pthread_cond_t* condVars;
pthread_mutex_t* mutexes;
pthread_cond_t* smoker_cond_vars;
pthread_mutex_t* smoker_mutexes;

pthread_cond_t breakCond,waitCond;
pthread_mutex_t grid_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unlock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t test_mut = PTHREAD_MUTEX_INITIALIZER;
bool breakCase = false;
bool stopCase = false;
Commander commander;
void computeWaitingTime(struct timespec &ts,int waiting_time){
    ts.tv_sec += waiting_time / 1000;
    long time_nano = ts.tv_nsec + (((long) waiting_time % 1000) * 1000000);
    if (time_nano > 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec = time_nano - 1000000000;
    }
    else{
        ts.tv_nsec = time_nano;
    }
}

void* smokerRoutine(void* smokerPtr) {
    auto *aSmoker = (Smoker *) smokerPtr;
    hw2_notify(SNEAKY_SMOKER_CREATED,aSmoker->sid,0,0);
    for (int i = 0; i < aSmoker->n_s ; ++i) {
        if(false){
            waitPoint:
            pthread_cond_wait(&waitCond,&aSmoker->smoker_object_mutex);
            if(stopCase){
                hw2_notify(SNEAKY_SMOKER_STOPPED,aSmoker->sid,0,0);
                goto stopPoint;
            }
        }


        int center_x = aSmoker->center_of_smoking.at(i).x_coord;
        int center_y = aSmoker->center_of_smoking.at(i).y_coord;
        pthread_mutex_lock(&wait_mutex);
        if(soldiers[center_x][center_y].NProperPrivates == 0 && demo[center_x][center_y] == 0){
            demo[center_x][center_y] = 1;
            soldiers[center_x][center_y].NSmokers ++;
            Coord temp_coord{};
            temp_coord.x_coord = aSmoker->center_of_smoking[i].x_coord;
            temp_coord.y_coord = aSmoker->center_of_smoking[i].y_coord;
            aSmoker->locked_areas_smoker.push_back(temp_coord);


        }
        else{
            pthread_mutex_unlock(&wait_mutex);
            goto waitPoint;
        }
        for (int j = 0; j < 8 ; ++j) {
            int d_x = aSmoker->flickingPoints[i][j].x_coord;
            int d_y = aSmoker->flickingPoints[i][j].y_coord;
            if(soldiers[d_x][d_y].NProperPrivates == 0 ){
                soldiers[d_x][d_y].NSmokers ++;
                Coord dummy_coord{};
                dummy_coord.x_coord = d_x;dummy_coord.y_coord = d_y;
                aSmoker->locked_areas_smoker.push_back(dummy_coord);
            }
            else{
                pthread_mutex_lock(&unlock_mutex);
                aSmoker->unlocking_smoker();
                demo[aSmoker->center_of_smoking[i].x_coord][aSmoker->center_of_smoking[i].y_coord] = 0;
                pthread_mutex_unlock(&unlock_mutex);
                pthread_mutex_unlock(&wait_mutex);
                goto waitPoint;
            }
        }
        pthread_mutex_unlock(&wait_mutex);

        if(stopCase){
            hw2_notify(SNEAKY_SMOKER_STOPPED,aSmoker->sid,0,0);
            goto unlockPoint;
        }
        hw2_notify(SNEAKY_SMOKER_ARRIVED,aSmoker->sid,aSmoker->center_of_smoking[i].x_coord,aSmoker->center_of_smoking[i].y_coord);
        clock_gettime(CLOCK_REALTIME,&aSmoker->smoker_timespec);
        computeWaitingTime(aSmoker->smoker_timespec,aSmoker->t_s);
        pthread_cond_timedwait(&(smoker_cond_vars[aSmoker->cond_var_index]),&(smoker_mutexes[aSmoker->cond_var_index]),&aSmoker->smoker_timespec);
        if(stopCase){
            hw2_notify(SNEAKY_SMOKER_STOPPED,aSmoker->sid,0,0);
            goto unlockPoint;
        }
        while(aSmoker->cigaretteCounts[i] > 0){
            if(stopCase){
                hw2_notify(SNEAKY_SMOKER_STOPPED,aSmoker->sid,0,0);
                goto unlockPoint;
            }
            if(aSmoker->flag == 8){aSmoker->flag = 0;}
            pthread_mutex_lock(&grid_mutex);
            grid[aSmoker->flickingPoints[i][aSmoker->flag].x_coord][aSmoker->flickingPoints[i][aSmoker->flag].y_coord] ++;
            aSmoker->cigaretteCounts[i] --;

            pthread_mutex_unlock(&grid_mutex);
            hw2_notify(SNEAKY_SMOKER_FLICKED,aSmoker->sid,aSmoker->flickingPoints[i][aSmoker->flag].x_coord,aSmoker->flickingPoints[i][aSmoker->flag].y_coord);
            aSmoker->flag ++;
            if(aSmoker->cigaretteCounts[i] > 0){
                clock_gettime(CLOCK_REALTIME,&aSmoker->smoker_timespec);
                computeWaitingTime(aSmoker->smoker_timespec,aSmoker->t_s);
                pthread_cond_timedwait(&(smoker_cond_vars[aSmoker->cond_var_index]),&(smoker_mutexes[aSmoker->cond_var_index]),&aSmoker->smoker_timespec);
            }


            if(stopCase){
                hw2_notify(SNEAKY_SMOKER_STOPPED,aSmoker->sid,0,0);
                goto unlockPoint;
            }
        }

        hw2_notify(SNEAKY_SMOKER_LEFT,aSmoker->sid,0,0);
        aSmoker->flag = 0;
        unlockPoint:
        pthread_mutex_lock(&unlock_mutex);
        aSmoker->unlocking_smoker();
        demo[aSmoker->center_of_smoking[i].x_coord][aSmoker->center_of_smoking[i].y_coord] = 0;
        pthread_cond_broadcast(&waitCond);
        pthread_mutex_unlock(&unlock_mutex);
        if(stopCase){
            goto stopPoint;
        }

    }
    hw2_notify(SNEAKY_SMOKER_EXITED,aSmoker->sid,0,0);
    stopPoint:
    return nullptr;
}
void* commanderRoutine(void* commanderPtr) {
    auto *aCommander = (Commander *) commanderPtr;
    std::vector<Commands> commandInstance = aCommander->commands;
    for (int i = 0; i < orderCount; ++i) {

        if(commandInstance[i].command == "break"){
            if(!i) {
                usleep(commandInstance[0].time * 1000);
            }
            else{
                usleep((commandInstance[i].time - commandInstance[i - 1].time) * 1000);
            }
            breakCase = true;
            hw2_notify(ORDER_BREAK,0,0,0);
            for (int j = 0; j < privateCount; ++j) {
                pthread_cond_signal(&condVars[j]);
            }
            pthread_cond_broadcast(&waitCond);
        }
        else if(commandInstance[i].command == "continue"){
            if(!i) {
                usleep(commandInstance[0].time * 1000);
            }
            else{
                if(commandInstance[i].time - commandInstance[i - 1].time == 1){
                    usleep((commandInstance[i].time - commandInstance[i - 1].time) * 1000 + 20000);
                }
                else{
                    usleep((commandInstance[i].time - commandInstance[i - 1].time) * 1000);
                }
            }
            hw2_notify(ORDER_CONTINUE,0,0,0);
            breakCase = false;
            pthread_cond_broadcast(&breakCond);
        }
        else if(commandInstance[i].command == "stop"){
            if(!i) {
                usleep(commandInstance[0].time * 1000);
            }
            else{
                usleep((commandInstance[i].time - commandInstance[i - 1].time) * 1000);
            }
            hw2_notify(ORDER_STOP,0,0,0);
            stopCase = true;
            pthread_cond_broadcast(&breakCond);
            for (int j = 0; j < privateCount; ++j) {
                pthread_cond_signal(&condVars[j]);
            }
            for (int j = 0; j < smokerCount; ++j) {
                pthread_cond_signal(&smoker_cond_vars[j]);
            }
            pthread_cond_broadcast(&waitCond);
        }

    }
    return nullptr;
}
void* properPrivateRoutine(void* privatePtr) {
    auto *aPrivate = (ProperPrivate *) privatePtr;
    hw2_notify(PROPER_PRIVATE_CREATED,aPrivate->gid,0,0);

    for (int i = 0; i < aPrivate->n_g; ++i) {
        if(false) {
            breakPoint:

            pthread_cond_wait(&breakCond,&aPrivate->mutex);
            if(stopCase){
                hw2_notify(PROPER_PRIVATE_STOPPED,aPrivate->gid,0,0);
                pthread_mutex_lock(&unlock_mutex);
                aPrivate->unlocking();
                pthread_mutex_unlock(&unlock_mutex);
                goto stopPoint;
            }
            hw2_notify(PROPER_PRIVATE_CONTINUED,aPrivate->gid,0,0);
        }
        if(false){
            waitPoint:
            pthread_cond_wait(&waitCond,&aPrivate->mutex2);
            if(breakCase){
                hw2_notify(PROPER_PRIVATE_TOOK_BREAK,aPrivate->gid,0,0);
                goto breakPoint;
            }
            if(stopCase){
                hw2_notify(PROPER_PRIVATE_STOPPED,aPrivate->gid,0,0);
                goto stopPoint;
            }
        }

        int startptX = aPrivate->startingPts.at(i).at(0);
        int startptY = aPrivate->startingPts.at(i).at(1);
        pthread_mutex_lock(&wait_mutex);
        for (int j = startptX; j < startptX + aPrivate->s_i   ; ++j) {
            for (int k = startptY; k < startptY + aPrivate->s_j ; ++k) {
                if(soldiers[j][k].NSmokers == 0 && soldiers[j][k].NProperPrivates == 0){
                    soldiers[j][k].NProperPrivates ++;
                    Coord dummy_coord{};
                    dummy_coord.x_coord = j;dummy_coord.y_coord = k;
                    aPrivate->locked_areas.push_back(dummy_coord);
                }
                else{
                    aPrivate->unlocking();
                    pthread_mutex_unlock(&wait_mutex);
                    goto waitPoint;
                }
            }
        }
        pthread_mutex_unlock(&wait_mutex);
        if(breakCase) {
            hw2_notify(PROPER_PRIVATE_TOOK_BREAK,aPrivate->gid,0,0);
            pthread_mutex_lock(&unlock_mutex);
            aPrivate->unlocking();
            pthread_cond_broadcast(&waitCond);
            pthread_mutex_unlock(&unlock_mutex);
            goto breakPoint;
        }
        if(stopCase){
            hw2_notify(PROPER_PRIVATE_STOPPED,aPrivate->gid,0,0);
            goto unlockPoint;
        }
        hw2_notify(PROPER_PRIVATE_ARRIVED,aPrivate->gid,aPrivate->startingPts.at(i).at(0),aPrivate->startingPts.at(i).at(1));
        if(grid[aPrivate->startingPts.at(i).at(0)][aPrivate->startingPts.at(i).at(1)] > 0){
            clock_gettime(CLOCK_REALTIME,&aPrivate->ts);
            computeWaitingTime(aPrivate->ts,aPrivate->t_delay);
            pthread_cond_timedwait(&condVars[aPrivate->semaphore_index],&mutexes[aPrivate->semaphore_index],&aPrivate->ts);
        }
        //clock_gettime(CLOCK_REALTIME,&aPrivate->ts);
        //computeWaitingTime(aPrivate->ts,aPrivate->t_delay);
        //pthread_cond_timedwait(&condVars[aPrivate->semaphore_index],&mutexes[aPrivate->semaphore_index],&aPrivate->ts);
        if(breakCase) {
            hw2_notify(PROPER_PRIVATE_TOOK_BREAK,aPrivate->gid,0,0);
            pthread_mutex_lock(&unlock_mutex);
            aPrivate->unlocking();
            pthread_cond_broadcast(&waitCond);
            pthread_mutex_unlock(&unlock_mutex);
            goto breakPoint;
        }
        if(stopCase){
            hw2_notify(PROPER_PRIVATE_STOPPED,aPrivate->gid,0,0);
            goto unlockPoint;
        }
        for (int j = aPrivate->startingPts.at(i).at(0); j < aPrivate->startingPts.at(i).at(0) + aPrivate->s_i  ; ++j) {
            for (int k = aPrivate->startingPts.at(i).at(1); k < aPrivate->startingPts.at(i).at(1) + aPrivate->s_j  ; ++k) {
                if(breakCase) {
                    hw2_notify(PROPER_PRIVATE_TOOK_BREAK,aPrivate->gid,0,0);
                    pthread_mutex_lock(&unlock_mutex);
                    aPrivate->unlocking();
                    pthread_cond_broadcast(&waitCond);
                    pthread_mutex_unlock(&unlock_mutex);
                    goto breakPoint;
                }
                if(stopCase){
                    hw2_notify(PROPER_PRIVATE_STOPPED,aPrivate->gid,0,0);
                    goto unlockPoint;
                }

                while(grid.at(j).at(k) > 0) {
                    hw2_notify(PROPER_PRIVATE_GATHERED,aPrivate->gid,j,k);
                    pthread_mutex_lock(&grid_mutex);
                    grid.at(j).at(k) --;
                    pthread_mutex_unlock(&grid_mutex);
                    if(grid[j][k] > 0 ){
                        clock_gettime(CLOCK_REALTIME,&aPrivate->ts);
                        computeWaitingTime(aPrivate->ts,aPrivate->t_delay);
                        pthread_cond_timedwait(&condVars[aPrivate->semaphore_index],&mutexes[aPrivate->semaphore_index],&aPrivate->ts);
                    }
                    else{
                        if(((j == (aPrivate->startingPts.at(i).at(0) + aPrivate->s_i - 1)) && (k == (aPrivate->startingPts.at(i).at(1) + aPrivate->s_j - 1)))){
                            continue;
                        }
                        else{
                            clock_gettime(CLOCK_REALTIME,&aPrivate->ts);
                            computeWaitingTime(aPrivate->ts,aPrivate->t_delay);
                            pthread_cond_timedwait(&condVars[aPrivate->semaphore_index],&mutexes[aPrivate->semaphore_index],&aPrivate->ts);
                        }
                    }

                    if(breakCase) {
                        hw2_notify(PROPER_PRIVATE_TOOK_BREAK,aPrivate->gid,0,0);
                        pthread_mutex_lock(&unlock_mutex);
                        aPrivate->unlocking();
                        pthread_cond_broadcast(&waitCond);
                        pthread_mutex_unlock(&unlock_mutex);
                        goto breakPoint;
                    }
                    if(stopCase){
                        hw2_notify(PROPER_PRIVATE_STOPPED,aPrivate->gid,0,0);
                        goto unlockPoint;
                    }
                }

            }
        }
        hw2_notify(PROPER_PRIVATE_CLEARED,aPrivate->gid,0,0);
        unlockPoint:
        pthread_mutex_lock(&unlock_mutex);
        aPrivate->unlocking();
        pthread_cond_broadcast(&waitCond);
        pthread_mutex_unlock(&unlock_mutex);
        if(stopCase){
            goto stopPoint;
        }

    }
    hw2_notify(PROPER_PRIVATE_EXITED,aPrivate->gid,0,0);
    stopPoint:
    return nullptr;
}


int main() {
    std::cin >> x_dim;
    std::cin >> y_dim;
    for (int i = 0; i < x_dim ; ++i) {
        std::vector<SoldierType> soldierType;
        for (int j = 0; j < y_dim ; ++j) {
            SoldierType s{};
            s.NProperPrivates = 0;
            s.NSmokers = 0;
            soldierType.push_back(s);
        }
        soldiers.push_back(soldierType);
    }
    for (int i = 0; i < x_dim ; ++i) {
        std::vector<int> temp(y_dim);
        std::vector<int> temp2(y_dim,0);
        for (int j = 0; j < y_dim ; ++j) {
            std::cin >> temp[j];
        }
        grid.push_back(temp);
        demo.push_back(temp2);
    }
    std::cin >> privateCount;
    for (int i = 0; i < privateCount ; ++i) {
        ProperPrivate properPrivate;
        std::cin >> properPrivate.gid;
        std::cin >> properPrivate.s_i;
        std::cin >> properPrivate.s_j;
        std::cin >> properPrivate.t_delay;
        std::cin >> properPrivate.n_g;
        for (int j = 0; j < properPrivate.n_g; ++j) {
            std::vector<int> temp(2);
            for (int k = 0; k < 2 ; ++k) {
                std::cin >> temp[k];
            }
            properPrivate.startingPts.push_back(temp);
            properPrivate.semaphore_index = i;
        }
        properPrivates.push_back(properPrivate);
    }

    if(!std::cin.eof()){
        std::cin >> orderCount;
        for (int i = 0; i < orderCount ; ++i) {
            struct Commands a_command;
            std::cin >> a_command.time;
            std::cin >> a_command.command;
            commander.commands.push_back(a_command);
        }
    }

    if(!std::cin.eof()) {
        std::cin >> smokerCount;
        for (int i = 0; i < smokerCount; ++i) {
            Smoker smoker_instance;
            std::cin >> smoker_instance.sid;
            std::cin >> smoker_instance.t_s;
            std::cin >> smoker_instance.n_s;
            for (int j = 0; j < smoker_instance.n_s; ++j) {
                Coord center_coords{};
                std::vector<Coord> flicking_coord_array;
                int smoking_count;
                std::cin >> center_coords.x_coord;
                std::cin >> center_coords.y_coord;
                std::cin >> smoking_count;
                Coord c1{}, c2{}, c3{}, c4{}, c5{}, c6{}, c7{}, c8{};
                c1.x_coord = center_coords.x_coord - 1;
                c1.y_coord = center_coords.y_coord - 1;
                flicking_coord_array.push_back(c1);
                c2.x_coord = center_coords.x_coord - 1;
                c2.y_coord = center_coords.y_coord;
                flicking_coord_array.push_back(c2);
                c3.x_coord = center_coords.x_coord - 1;
                c3.y_coord = center_coords.y_coord + 1;
                flicking_coord_array.push_back(c3);
                c4.x_coord = center_coords.x_coord;
                c4.y_coord = center_coords.y_coord + 1;
                flicking_coord_array.push_back(c4);
                c5.x_coord = center_coords.x_coord + 1;
                c5.y_coord = center_coords.y_coord + 1;
                flicking_coord_array.push_back(c5);
                c6.x_coord = center_coords.x_coord + 1;
                c6.y_coord = center_coords.y_coord;
                flicking_coord_array.push_back(c6);
                c7.x_coord = center_coords.x_coord + 1;
                c7.y_coord = center_coords.y_coord - 1;
                flicking_coord_array.push_back(c7);
                c8.x_coord = center_coords.x_coord;
                c8.y_coord = center_coords.y_coord - 1;
                flicking_coord_array.push_back(c8);
                smoker_instance.center_of_smoking.push_back(center_coords);
                smoker_instance.flickingPoints.push_back(flicking_coord_array);
                smoker_instance.cigaretteCounts.push_back(smoking_count);
                smoker_instance.cond_var_index = i;
            }
            sneakySmokers.push_back(smoker_instance);
        }
    }

    breakCond = PTHREAD_COND_INITIALIZER;
    waitCond = PTHREAD_COND_INITIALIZER;

    condVars = new pthread_cond_t[privateCount];
    mutexes = new pthread_mutex_t [privateCount];
    for (int i = 0; i < privateCount; ++i) {
        condVars[i] = PTHREAD_COND_INITIALIZER;
        mutexes[i] = PTHREAD_MUTEX_INITIALIZER;
    }
    if(smokerCount > 0) {
        smoker_cond_vars = new pthread_cond_t[smokerCount];
        smoker_mutexes = new pthread_mutex_t[smokerCount];
        for (int i = 0; i < smokerCount; ++i) {
            smoker_cond_vars[i] = PTHREAD_COND_INITIALIZER;
            smoker_mutexes[i] = PTHREAD_MUTEX_INITIALIZER;
        }
    }
    pthread_t privateThreads[privateCount];
    pthread_t smokerThreads[smokerCount];
    pthread_t commanderThread;
    hw2_init_notifier();
    pthread_create(&commanderThread, nullptr, commanderRoutine,(void*)&commander);
    for (int i = 0; i < privateCount; ++i) {
        pthread_create(privateThreads + i, nullptr,properPrivateRoutine,(void*)&properPrivates.at(i));
    }

    for (int i = 0; i < smokerCount ; ++i) {
        pthread_create(smokerThreads + i, nullptr,smokerRoutine,(void*)&sneakySmokers.at(i));
    }

    pthread_join(commanderThread, nullptr);

    for (int i = 0; i < smokerCount ; ++i) {
        pthread_join(smokerThreads[i], nullptr);
    }

    for (int i = 0; i < privateCount ; ++i) {
        pthread_join(privateThreads[i], nullptr);
    }
    delete [] condVars;
    delete [] mutexes;
    delete [] smoker_cond_vars;
    delete [] smoker_mutexes;

    return 0;
}
