#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "parser.h"
class Bundle{
private:
    char* bundle_name;
    std :: vector<char**> args;
public:
    Bundle(char* bundle_name, std :: vector<char**> args) {
        this->bundle_name = bundle_name;
        this->args = args;
    }

    char *getBundleName() const {
        return bundle_name;
    }

    const std::vector<char **> &getArgs() const {
        return args;
    }
};
void multi_process_bundle(std::vector<Bundle> bundleObjects,int bundleCount,bundle_execution* bundles) {
    int bundleToRepeater[bundleCount - 1][2];
    int child_status;
    int totalProcess = bundleCount - 1;
    for (int i = 0; i < bundleObjects.size(); ++i) {
        totalProcess += bundleObjects.at(i).getArgs().size();
    }

    for (int i = 0; i < bundleCount ; ++i) {
        if (i == 0){
            pipe(bundleToRepeater[i]);
            for (int j = 0; j < bundleObjects.at(i).getArgs().size() ; ++j) {
                int pid = fork();
                if(!pid) {
                    int input_file;
                    if(bundles[i].input != nullptr) {
                        input_file = open(bundles[i].input,O_APPEND | O_RDONLY,0777);
                        dup2(input_file,STDIN_FILENO);
                        close(input_file);
                    }
                    close(bundleToRepeater[i][0]);
                    dup2(bundleToRepeater[i][1],STDOUT_FILENO);
                    close(bundleToRepeater[i][1]);
                    execvp(bundleObjects.at(i).getArgs().at(j)[0],bundleObjects.at(i).getArgs().at(j));
                }
            }
        }
        else if (i > 0){
            int processCount = bundleObjects.at(i).getArgs().size();
            int repeaterToBundle[processCount][2];
            for (int j = 0; j < processCount ; ++j) {
                pipe(repeaterToBundle[j]);
            }

            int repeaterid = fork();
            if(!repeaterid) {
                //repeater do stuff
                char readVal;
                close(bundleToRepeater[i - 1][1]);
                for (int j = 0; j < processCount ; ++j) {
                    close(repeaterToBundle[j][0]);
                }

                while (read(bundleToRepeater[i - 1][0],&readVal,1) > 0) {
                    //std::cout << "read val: " << readVal << std::endl;
                    for (int j = 0; j < processCount ; ++j) {
                        write(repeaterToBundle[j][1],&readVal,1);
                    }
                }

                close(bundleToRepeater[i - 1][0]);
                for (int j = 0; j < processCount ; ++j) {
                    close(repeaterToBundle[j][1]);
                }

                exit(EXIT_SUCCESS);
            }
            close(bundleToRepeater[i - 1][0]);
            close(bundleToRepeater[i - 1][1]);
            /*
            pid_t wpid2 = waitpid(repeaterid,&child_status,0);
            if (WIFEXITED(child_status)) {
                printf("child %d terminated with exit status %d\n",wpid2,
                       WEXITSTATUS(child_status));
            }
            else {
                printf("child %d terminated abnormally\n",wpid2);
            }
            */
            if(i < bundleCount - 1){
                pipe(bundleToRepeater[i]);
            }
            for (int j = 0; j < bundleObjects.at(i).getArgs().size(); ++j) {
                int pid2 = fork();
                if(!pid2) {
                    dup2(repeaterToBundle[j][0],STDIN_FILENO);
                    if (i < bundleCount - 1){
                        dup2(bundleToRepeater[i][1],STDOUT_FILENO);
                        close(bundleToRepeater[i][0]);
                        close(bundleToRepeater[i][1]);
                    }
                    if (i == bundleCount - 1) {
                        int output_file;
                        if(bundles[i].output != nullptr) {
                            output_file = open(bundles[i].output,O_CREAT | O_WRONLY | O_APPEND,0777);
                            dup2(output_file,STDOUT_FILENO);
                            close(output_file);
                        }
                    }

                    for (int k = 0; k < processCount ; ++k) {
                        close(repeaterToBundle[k][0]);
                        close(repeaterToBundle[k][1]);
                    }

                    execvp(bundleObjects.at(i).getArgs().at(j)[0],bundleObjects.at(i).getArgs().at(j));
                }
            }
            for (int j = 0; j < processCount; ++j) {
                close(repeaterToBundle[j][0]);
                close(repeaterToBundle[j][1]);
            }
        }

    }
    for (int i = 0; i < totalProcess; ++i) {
        pid_t wpid = wait(&child_status);
        if (WIFEXITED(child_status)) {
            //printf("child %d terminated with exit status %d\n",wpid,WEXITSTATUS(child_status));
        }
        else {
            printf("child %d terminated abnormally\n",wpid);
        }
    }

}

void execute_one_bundle (Bundle bundle,bundle_execution* bundles) {
    int child_status;
    if (bundles->input == nullptr && bundles->output == nullptr) {
        for (int i = 0; i < bundle.getArgs().size() ; ++i) {
            int pid = fork();
            if (pid == 0) {
                execvp(bundle.getArgs().at(i)[0],bundle.getArgs().at(i));
            }
        }
        for (int i = 0; i < bundle.getArgs().size(); ++i) {
            pid_t wpid = wait(&child_status);
            if (WIFEXITED(child_status)) {
                //printf("child %d terminated with exit status %d\n",wpid,WEXITSTATUS(child_status));
            }
            else {
                printf("child %d terminated abnormally\n",wpid);
            }
        }
    }
    else if (bundles->output != nullptr && bundles->input == nullptr) {

        for (int i = 0; i < bundle.getArgs().size(); ++i) {
            int fd = open(bundles->output,O_WRONLY | O_APPEND | O_CREAT,0777);
            if (fd < 0) { printf("error while opening file\n");}
            int pid = fork();
            if (pid == 0) {
                dup2(fd,STDOUT_FILENO);
                close(fd);
                execvp(bundle.getArgs().at(i)[0],bundle.getArgs().at(i));
            }
        }
        for (int i = 0; i < bundle.getArgs().size(); ++i) {
            pid_t wpid = wait(&child_status);
            if (WIFEXITED(child_status)) {//printf("child %d terminated with exit status %d\n",wpid,WEXITSTATUS(child_status));
            }
            else {
                printf("child %d terminated abnormally\n",wpid);
            }
        }
    }
    else if (bundles->input != nullptr && bundles->output == nullptr) {

        for (int i = 0; i < bundle.getArgs().size(); ++i) {
            int fd = open(bundles->input,O_RDONLY | O_APPEND,0777);
            if (fd < 0) { printf("error while opening file\n");}
            int pid = fork();
            if (pid == 0) {
                dup2(fd,STDIN_FILENO);
                close(fd);
                execvp(bundle.getArgs().at(i)[0],bundle.getArgs().at(i));
            }
        }
        for (int i = 0; i < bundle.getArgs().size(); ++i) {
            pid_t wpid = wait(&child_status);
            if (WIFEXITED(child_status)) {
                //printf("child %d terminated with exit status %d\n",wpid,WEXITSTATUS(child_status));
            }
            else {
                printf("child %d terminated abnormally\n",wpid);
            }
        }
    }
    else{
        for (int i = 0; i < bundle.getArgs().size() ; ++i) {
            int fd1 = open(bundles->input,O_APPEND | O_RDONLY,0777);
            if (fd1 < 0) { printf("error while opening file\n");}
            int fd2 = open(bundles->output,O_APPEND | O_WRONLY | O_CREAT,0777);
            if (fd2 < 0) { printf("error while opening file\n");}
            int pid = fork();
            if (pid == 0) {
                dup2(fd1,STDIN_FILENO);
                close(fd1);
                dup2(fd2,STDOUT_FILENO);
                close(fd2);
                execvp(bundle.getArgs().at(i)[0],bundle.getArgs().at(i));
            }
        }
        for (int i = 0; i < bundle.getArgs().size(); ++i) {
            pid_t wpid = wait(&child_status);
            if (WIFEXITED(child_status)) {
                //printf("child %d terminated with exit status %d\n",wpid,WEXITSTATUS(child_status));
            }
            else {
                printf("child %d terminated abnormally\n",wpid);
            }
        }
    }
}
int main() {

    std :: string inp;
    parsed_input *pi = new parsed_input;
    int is_bundle_creation = 0;
    std :: vector<char**> arguments;
    std :: vector<Bundle> allBundles;
    char* bundleName;

    while (true){
        getline(std :: cin,inp);
        char *line = new char[inp.length() + 2];
        std::strcpy(line,inp.c_str());
        line[inp.length()] = '\n';
        //line[inp.length() + 1] = '\0';
        parse(&(*line),is_bundle_creation,pi);
        if (pi->command.type == QUIT) {
            break;
        }
        if (inp != "pbs" && is_bundle_creation) {
            arguments.push_back(pi->argv);
        }
        if (inp.find("pbc") != std :: string :: npos){
            bundleName = pi->command.bundle_name;
            is_bundle_creation = 1;
        }
        if (inp == "pbs") {
            is_bundle_creation = 0;
            Bundle bundle = Bundle(bundleName,arguments);
            arguments.clear();
            allBundles.push_back(bundle);
        }
        if (pi->command.type == PROCESS_BUNDLE_EXECUTION) {
            std::vector<Bundle> pipeline;
            if (pi->command.bundle_count == 1){
                int index = 0;
                bool flag = false;
                for (int i = 0; i < allBundles.size() ; ++i) {
                    if (strcmp(allBundles[i].getBundleName(),pi->command.bundles->name) == 0){
                        index = i;
                        flag = true;
                        break;
                    }
                }
                if (flag) {
                    execute_one_bundle(allBundles[index],pi->command.bundles);
                    allBundles.erase(allBundles.begin() + index);
                }
            }
            else {
                std::vector<int> indexes;
                for (int i = 0; i < pi->command.bundle_count; ++i) {
                    for (int j = 0; j < allBundles.size() ; ++j) {
                        if (strcmp(pi->command.bundles[i].name,allBundles.at(j).getBundleName()) == 0){
                            pipeline.push_back(allBundles.at(j));
                            indexes.push_back(j);
                            break;
                        }
                    }
                }
                if (indexes.size() == pi->command.bundle_count) {
                    multi_process_bundle(pipeline,pi->command.bundle_count,pi->command.bundles);
                    for (int i = 0; i < indexes.size(); ++i) {
                        allBundles.erase(allBundles.begin() + indexes.at(i));
                    }
                }
            }
        }
    }
    return 0;

}
