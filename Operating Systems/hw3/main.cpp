#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <bitset>
#include <ctime>
#include "fat32.h"
#include "parser.h"
typedef struct{
    int is_pathed;
    int current;
    int parent;
    std::vector<std::string> folder_vector;
}Cluster_Info;
typedef struct{
    int year;
    std::string month;
    std::string day;
}Date_Info;
typedef struct{
    std::string hour;
    std::string minute;
    int second;
}Time_Info;
BPB_struct first_sector;
int fd;
int begin_of_data_sec,current_cluster,begin_of_fat,parent_cluster,root_cluster;
int entry_count_per_cluster;
std::vector<int> fat_table;
std::vector<std::string> prompt_folders;
std::string currentFolder;
int find_free_cluster(){
    for (int i = 0; i < fat_table.size() ; ++i) {
        if(fat_table[i] == 0){
            return i;
        }
    }
    return 0;
}
unsigned char lfn_checksum(unsigned char *name)
{
    int i = 0;
    unsigned char sum = 0;
    int len = strlen(reinterpret_cast<const char *>(name));
    while(i < len + 1){
        sum = ((sum & 1) << 7) + (sum >> 1) + *name++;
        i += 1;
    }

    return sum;
}
FatFileLFN fill_file_entry(std::string &temp,bool &end_of_string){
    FatFileLFN fatFileLfn;
    fatFileLfn.attributes = 0x0f;
    fatFileLfn.reserved = 0x00;
    fatFileLfn.firstCluster = 0x0000;
    for (int j = 0; j < 5 ; ++j) {
        if(!temp.empty()){
            fatFileLfn.name1[j] = int(temp[0]);
            temp.erase(0,1);
        }
        else{
            if(!end_of_string){
                fatFileLfn.name1[j] = '\0';
                end_of_string = true;
            }
            else{
                fatFileLfn.name1[j] = 65535;
            }
        }
    }
    for (int j = 0; j < 6 ; ++j) {
        if(!temp.empty()){
            fatFileLfn.name2[j] = int(temp[0]);
            temp.erase(0,1);
        }
        else{
            if(!end_of_string){
                fatFileLfn.name2[j] = 0;
                end_of_string = true;
            }
            else{
                fatFileLfn.name2[j] = 65535;
            }
        }
    }
    for (int j = 0; j < 2 ; ++j) {
        if(!temp.empty()){
            fatFileLfn.name3[j] = int(temp[0]);
            temp.erase(0,1);
        }
        else{
            if(!end_of_string){
                fatFileLfn.name3[j] = 0;
                end_of_string = true;
            }
            else{
                fatFileLfn.name3[j] = 65535;
            }
        }
    }
    return fatFileLfn;
}
Time_Info string_to_time(std::string time){
    Time_Info timeInfo;
    std::string hour,minute,second;
    for (int i = 0; i < time.length() ; ++i) {
        if(i < 5){
            hour += time[i];
        }
        else if(i > 4 && i < 11){
            minute += time[i];
        }
    }
    if(std::stoi(hour,0,2) < 10){
        timeInfo.hour = "0" + std::to_string(std::stoi(hour,0,2));
    }
    else{
        timeInfo.hour = std::to_string(std::stoi(hour,0,2));
    }
    if(std::stoi(minute,0,2) < 10){
        timeInfo.minute = "0" + std::to_string(std::stoi(minute,0,2));
    }
    else{
        timeInfo.minute = std::to_string(std::stoi(minute,0,2));
    }
    return timeInfo;
}
Date_Info string_to_date(std::string date){
    Date_Info dateInfo;
    std::string year,month,day;
    std::string months[12] = {"January","February","March","April","May","June",
                              "July","August","September","October","November","December"};
    for (int i = 0; i < date.length() ; ++i) {
        if(i < 7){
            year += date[i];
        }
        else if(i > 6 && i < 11){
            month += date[i];
        }
        else{
            day += date[i];
        }
    }
    dateInfo.year = std::stoi(year,0,2) + 1980;
    dateInfo.month = months[std::stoi(month,0,2) - 1];
    if(std::stoi(day,0,2) < 10){
        dateInfo.day = "0" + std::to_string(std::stoi(day,0,2));
    }
    else{
        dateInfo.day = std::to_string(std::stoi(day,0,2));
    }
    return dateInfo;
}
std::string get_filename(FatFileEntry fileEntry){
    std::string return_val = "";
    for (int i = 0; i < 5 ; ++i) {
        if(char(fileEntry.lfn.name1[i]) != '\0'){
            return_val += char(fileEntry.lfn.name1[i]);
        }
        else{
            goto returnPt;
        }
    }
    for (int i = 0; i < 6 ; ++i) {
        if(char(fileEntry.lfn.name2[i]) != '\0'){
            return_val += char(fileEntry.lfn.name2[i]);
        }
        else{
            goto returnPt;
        }
    }
    for (int i = 0; i < 2 ; ++i) {
        if(char(fileEntry.lfn.name3[i]) != '\0'){
            return_val += char(fileEntry.lfn.name3[i]);
        }
        else{
            goto returnPt;
        }
    }
    returnPt:
    return return_val;
}
int get_first_sector_of_cluster(int clusterNo){
    return (clusterNo - 2) * int(first_sector.SectorsPerCluster) * BPS + begin_of_data_sec;
}
void get_fat_table_vector(){
    for (int i = begin_of_fat; i < int(first_sector.extended.FATSize) * BPS; i += 4) {
        lseek(fd,i,SEEK_SET);
        int cluster_entry;
        read(fd,&cluster_entry,4);
        cluster_entry = cluster_entry & 0x0fffffff;
        if(cluster_entry >= 0x0ffffff0){
            fat_table.push_back(-1);
        }
        else{
            fat_table.push_back(cluster_entry);
        }
    }
}
std::vector<std::string> char_array_to_vector(char* arg){
    std::string temp = "";
    std::vector<std::string> char_vector;
    for (int i = 0; i < strlen(arg) ; ++i) {
        if(arg[i] == '/'){
            if(temp != ""){
                char_vector.push_back(temp);
            }
            temp = "";
        }
        else{
            temp += arg[i];
        }
    }
    char_vector.push_back(temp);
    return char_vector;
}
std::pair<int,int> get_cluster_info(int offset){
    std::pair<int,int> cluster(root_cluster,root_cluster);
    FatFileEntry fileEntry;
    int index = 0;
    for (int j = 0; j < int(first_sector.SectorsPerCluster) * BPS; j += 32) {
        lseek(fd, offset + j, SEEK_SET);
        read(fd, &fileEntry, 32);
        if(fileEntry.msdos.attributes == 0x10){
            if(fileEntry.msdos.filename[0] == 0x2E){
                if(!index){
                    cluster.first = int(fileEntry.msdos.firstCluster);
                    index += 1;
                }
                else{
                    if(int(fileEntry.msdos.firstCluster) == 0){
                        cluster.second = 2;
                    }
                    else{
                        cluster.second = int(fileEntry.msdos.firstCluster);
                    }
                    return cluster;
                }
            }

        }
    }
    return cluster;
}
std::string folder_vector_to_prompt(){
    std::string temp = "";
    if(!prompt_folders.empty()){
        for (int i = 0; i < prompt_folders.size(); ++i) {
            if(i != prompt_folders.size() - 1){
                temp += prompt_folders[i] + "/";
            }
            else{
                temp += prompt_folders[i];
            }
        }
    }
    return temp;
}
std::pair<int,int> check_free_space_in_cluster(int clusterNo){
    std::pair<int,int> pair_value(0,0);
    int free_space_offset_start = 0;
    int file_count = 0;
    int offset = get_first_sector_of_cluster(clusterNo);
    FatFileEntry fileEntry;
    for (int j = 0; j < int(first_sector.SectorsPerCluster) * BPS; j += 32) {
        lseek(fd,offset + j,SEEK_SET);
        read(fd,&fileEntry,32);
        if(fileEntry.lfn.attributes == 0x0f && fileEntry.lfn.reserved == 0x00){

        }
        else{
            if(fileEntry.msdos.filename[0] == 0x00){
                free_space_offset_start = offset + j;
                break;
            }
            else{
                file_count += 1;
            }
        }
    }
    pair_value.first = file_count;
    pair_value.second = free_space_offset_start;
    return pair_value;
}
void fill_directory_entry(bool is_directory,std::string directory_name,int file_count,int lfn_count,int free_space_offset_start,std::string &date_str,std::string & time_str,int parent,int parent_of_parent){
    int free_cluster = find_free_cluster();
    int cluster_value = 0x0fffffff;
    lseek(fd,begin_of_fat + free_cluster * 4,SEEK_SET);
    write(fd,&cluster_value,4);
    get_fat_table_vector();
    std::string temp = directory_name;
    bool end_of_string = false;
    std::string file_index_str = std::to_string(file_count + 1);
    auto* file_name_8_3 = new unsigned char[file_index_str.length() + 2];
    file_name_8_3[0] = '~';
    for (int j = 0; j < file_index_str.length() ; ++j) {
        file_name_8_3[j+1] = file_index_str[j];
    }
    file_name_8_3[file_index_str.length() + 1] = '\0';
    for (int i = 0; i < lfn_count + 1; ++i) {
        if(i != lfn_count){
            FatFileEntry fatFileEntry;
            fatFileEntry.lfn = fill_file_entry(temp,end_of_string);
            fatFileEntry.lfn.checksum = lfn_checksum(file_name_8_3);
            lseek(fd,free_space_offset_start + i * 32,SEEK_SET);
            write(fd,&fatFileEntry,32);
        }
        else{
            FatFileEntry fatFileEntry;
            for (int j = 0; j < strlen(reinterpret_cast<const char *>(file_name_8_3)) ; ++j) {
                fatFileEntry.msdos.filename[j] = int(file_name_8_3[j]);
            }
            fatFileEntry.msdos.firstCluster = free_cluster;
            fatFileEntry.msdos.eaIndex = 0;
            if(is_directory == true){
                fatFileEntry.msdos.attributes = 0x10;
            }
            else {
                fatFileEntry.msdos.attributes = 32;
            }
            std::time_t t = std::time(0);
            std::tm* now = std::localtime(&t);
            date_str += std::bitset<7>(now->tm_year - 80).to_string();
            date_str += std::bitset<4>(now->tm_mon + 1).to_string();
            date_str += std::bitset<5>(now->tm_mday).to_string();
            time_str += std::bitset<5>(now->tm_hour).to_string();
            time_str += std::bitset<6>(now->tm_min).to_string();
            time_str += std::bitset<5>(now->tm_sec / 2).to_string();
            fatFileEntry.msdos.creationDate = std::stoi(date_str,0,2);
            fatFileEntry.msdos.modifiedDate = std::stoi(date_str,0,2);
            fatFileEntry.msdos.creationTime = std::stoi(time_str,0,2);
            fatFileEntry.msdos.modifiedTime = std::stoi(time_str,0,2);
            fatFileEntry.msdos.fileSize = 0;
            lseek(fd,free_space_offset_start + i * 32,SEEK_SET);
            write(fd,&fatFileEntry.msdos,32);
        }
    }
    if(is_directory == true){
        FatFileEntry dot_entry_1;
        dot_entry_1.msdos.filename[0] = 46;
        for (int i = 1; i < 8; ++i) {
            dot_entry_1.msdos.filename[i] = 32;
        }
        for (int i = 0; i < 3 ; ++i) {
            dot_entry_1.msdos.extension[i] = 32;
        }
        dot_entry_1.msdos.attributes = 0x10;
        dot_entry_1.msdos.reserved = 0;
        dot_entry_1.msdos.creationTimeMs = 0;
        dot_entry_1.msdos.creationTime = std::stoi(time_str,0,2);
        dot_entry_1.msdos.modifiedTime = std::stoi(time_str,0,2);
        dot_entry_1.msdos.creationDate = std::stoi(date_str,0,2);
        dot_entry_1.msdos.modifiedDate = std::stoi(date_str,0,2);
        dot_entry_1.msdos.eaIndex = 0;
        dot_entry_1.msdos.fileSize = 0;
        dot_entry_1.msdos.firstCluster = free_cluster;
        int offset_1 = get_first_sector_of_cluster(free_cluster);
        lseek(fd,offset_1,SEEK_SET);
        write(fd,&dot_entry_1,32);
        FatFileEntry dot_entry_2;
        dot_entry_2.msdos.filename[0] = 46;
        dot_entry_2.msdos.filename[1] = 46;
        for (int i = 2; i < 8; ++i) {
            dot_entry_2.msdos.filename[i] = 32;
        }
        for (int i = 0; i < 3; ++i) {
            dot_entry_2.msdos.extension[i] = 32;
        }
        dot_entry_2.msdos.attributes = 0x10;
        dot_entry_2.msdos.reserved = 0;
        dot_entry_2.msdos.creationTimeMs = 0;
        dot_entry_2.msdos.creationTime = std::stoi(time_str,0,2);
        dot_entry_2.msdos.modifiedTime = std::stoi(time_str,0,2);
        dot_entry_2.msdos.creationDate = std::stoi(date_str,0,2);
        dot_entry_2.msdos.modifiedDate = std::stoi(date_str,0,2);
        dot_entry_2.msdos.eaIndex = 0;
        dot_entry_2.msdos.fileSize = 0;
        if(parent == 2){
            dot_entry_2.msdos.firstCluster = 0;
        }
        else{
            dot_entry_2.msdos.firstCluster = parent;
        }
        lseek(fd,offset_1 + 32,SEEK_SET);
        write(fd,&dot_entry_2,32);
    }

    int cursor_cluster = parent_of_parent;
    while(true){
        int offset = get_first_sector_of_cluster(cursor_cluster);
        FatFileEntry fileEntry;
        for (int j = 0; j < int(first_sector.SectorsPerCluster) * BPS; j += 32) {
            lseek(fd, offset + j, SEEK_SET);
            read(fd, &fileEntry, 32);
            if(fileEntry.msdos.firstCluster == parent){
                FatFileEntry reserve_file_entry;
                reserve_file_entry = fileEntry;
                reserve_file_entry.msdos.modifiedDate = std::stoi(date_str,0,2);
                reserve_file_entry.msdos.modifiedTime = std::stoi(time_str,0,2);
                lseek(fd,offset + j,SEEK_SET);
                write(fd,&reserve_file_entry,32);
                break;
            }
        }
        if(fat_table[cursor_cluster] != -1){
            cursor_cluster = fat_table[cursor_cluster];
        }
        else{
            break;
        }
    }

}
char* vector_to_char_pointer(std::vector<std::string> arg_vector){
    int total_size = 1;
    total_size += arg_vector.size();
    for (int i = 0; i < arg_vector.size() ; ++i) {
        total_size += arg_vector[i].size();
    }
    char* result = new char[total_size];
    int ind = 0;
    for (int i = 0; i < arg_vector.size() ; ++i) {
        result[ind] = '/';
        ind += 1;
        for (int j = 0; j < arg_vector[i].size() ; ++j) {
            result[ind] = arg_vector[i][j];
            ind += 1;
        }
    }
    result[ind] = '\0';
    return result;
}
std::pair<bool,int> find_file(std::string file_name,int cluster_no){
    std::pair<bool,int> res(false,0);
    int current = cluster_no;
    std::string temp;
    while(true){
        int offset = get_first_sector_of_cluster(current);
        FatFileEntry fileEntry;
        for (int j = 0; j < int(first_sector.SectorsPerCluster) * BPS; j += 32) {
            lseek(fd, offset + j, SEEK_SET);
            read(fd, &fileEntry, 32);
            if(fileEntry.lfn.attributes == 0x0f && fileEntry.lfn.reserved == 0x00){
                temp = get_filename(fileEntry);
            }
            else{
                if(fileEntry.msdos.attributes != 0x10){
                    if(temp == file_name){
                        res.first = true;
                        res.second = fileEntry.msdos.firstCluster;
                        return res;
                    }
                    else{
                        temp = "";
                    }
                }
                else{
                    temp = "";
                }
            }
        }
        if(fat_table[current] != -1){
            current = fat_table[current];
        }
        else{
            break;
        }
    }

    return res;
}
Cluster_Info locate(char* path,int current_arg,int parent_arg){
    std::vector<std::string> folder_array = char_array_to_vector(path);
    std::vector<std::string> obtained_folders;
    if(current_arg == 2 && parent_arg == 2){
    }
    else{
        obtained_folders = prompt_folders;
    }
    std::string filename = "";
    int parent = parent_arg;
    int current = current_arg;
    int target_dir = false;
    Cluster_Info clusterInfo;

    for (int i = 0;i < folder_array.size() ; ++i) {
        if(folder_array[i] == ".."){
            current = parent;
            parent = get_cluster_info(get_first_sector_of_cluster(current)).second;
            if(!obtained_folders.empty()){
                obtained_folders.pop_back();
            }
            continue;
        }
        else if(folder_array[i] == "."){
            continue;
        }
        while (true){
            int offset = get_first_sector_of_cluster(current);
            FatFileEntry fileEntry;
            for (int j = 0; j < int(first_sector.SectorsPerCluster) * BPS; j += 32) {
                lseek(fd,offset + j,SEEK_SET);
                read(fd,&fileEntry,32);
                if(fileEntry.lfn.attributes == 0x0f && fileEntry.lfn.reserved == 0x00){
                    filename = get_filename(fileEntry);
                }
                else{
                    if(fileEntry.msdos.attributes == 0x10){
                        if(filename == folder_array[i]){
                            target_dir = true;
                            obtained_folders.push_back(filename);
                            std::pair<int,int> current_cluster_pair = get_cluster_info(
                                    get_first_sector_of_cluster(int(fileEntry.msdos.firstCluster)));
                            current = current_cluster_pair.first;
                            parent = current_cluster_pair.second;
                            filename = "";
                            break;
                        }

                    }
                }
            }
            if(target_dir){
                if(i != folder_array.size() - 1){
                    target_dir = false;
                }
                break;
            }
            if(fat_table[current_cluster] != -1){
                current = fat_table[current];
            }
            else{
                obtained_folders.clear();
                clusterInfo.is_pathed = false;
                clusterInfo.current = 0;
                clusterInfo.parent = 0;
                clusterInfo.folder_vector = obtained_folders;
                return clusterInfo;
            }
        }
    }

    clusterInfo.current = current;
    clusterInfo.parent = parent;
    clusterInfo.folder_vector = obtained_folders;
    clusterInfo.is_pathed = true;
    return clusterInfo;
}
void cd(char* path){
    Cluster_Info clusterInfo = locate(path,current_cluster,parent_cluster);
    if(clusterInfo.is_pathed){
        current_cluster = clusterInfo.current;
        parent_cluster = clusterInfo.parent;
        prompt_folders = clusterInfo.folder_vector;
        currentFolder = folder_vector_to_prompt();
    }
    else{
        clusterInfo = locate(path,root_cluster,root_cluster);
        if(clusterInfo.is_pathed){
            current_cluster = clusterInfo.current;
            parent_cluster = clusterInfo.parent;
            prompt_folders = clusterInfo.folder_vector;
            currentFolder = folder_vector_to_prompt();
        }
    }
}
void ls(bool is_detailed,char* path){
    int current;
    int index = 0;
    std::string filename = "";
    Cluster_Info clusterInfo;
    if (path != nullptr){
        clusterInfo = locate(path,current_cluster,parent_cluster);
        if(clusterInfo.is_pathed){
            current = clusterInfo.current;
        }
        else{
            clusterInfo = locate(path,root_cluster,root_cluster);
            if(clusterInfo.is_pathed){
                current = clusterInfo.current;
            }
            else{
                return;
            }
        }
    }
    else{
        current = current_cluster;
    }
    while(true){
        int offset = get_first_sector_of_cluster(current);
        FatFileEntry fileEntry;
        for (int j = 0; j < int(first_sector.SectorsPerCluster) * BPS; j += 32) {
            lseek(fd,offset + j,SEEK_SET);
            read(fd,&fileEntry,32);
            if(fileEntry.lfn.attributes == 0x0f && fileEntry.lfn.reserved == 0x00){
                filename = get_filename(fileEntry);
                int t;
            }
            else{
                if(!filename.empty()){
                    if(fileEntry.msdos.filename[0] != 0x00){
                        if(is_detailed == true){
                            std::bitset<16> date_bits(fileEntry.msdos.modifiedDate);
                            std::bitset<16> time_bits(fileEntry.msdos.modifiedTime);
                            std::string date = date_bits.to_string();
                            Date_Info dateInfo = string_to_date(date);
                            std::string time = time_bits.to_string();
                            Time_Info timeInfo = string_to_time(time);
                            if(fileEntry.msdos.attributes == 0x10){
                                std::cout << "drwx------ 1 root root 0" << " " << dateInfo.month << " " << dateInfo.day << " " << timeInfo.hour << ":" << timeInfo.minute << " " << filename << std::endl;
                            }
                            else{
                                std::cout << "-rwx------ 1 root root" << " " << fileEntry.msdos.fileSize << " " << dateInfo.month << " " << dateInfo.day << " " << timeInfo.hour << ":" << timeInfo.minute << " " << filename << std::endl;
                            }
                        }
                        if(is_detailed == false){
                            if(index != 0){
                                std::cout << " " << filename;
                                index += 1;
                            }
                            else{
                                std::cout << filename;
                                index += 1;
                            }
                        }
                    }
                    else{
                        if(is_detailed == false){
                            std::cout << "\n";
                            break;
                        }
                    }
                }
            }
        }
        if(fat_table[current_cluster] != -1){
            current = fat_table[current];
        }
        else{
            break;
        }
    }
}
void touch(char* path){
    std::vector<std::string> folder_array = char_array_to_vector(path);
    int current,parent;
    bool isDirect= false;
    int current_first;
    int lfn_count = 1;
    std::string directory_name;
    std::string time_str = "";
    std::string date_str = "";
    if(folder_array.size() == 1){
        current = current_cluster;
        parent = parent_cluster;
        directory_name = folder_array[0];
    }
    else{
        directory_name = folder_array[folder_array.size() - 1];
        folder_array.pop_back();
        std::vector<std::string> parent_vector = folder_array;
        char* parent_path = vector_to_char_pointer(parent_vector);
        Cluster_Info located = locate(parent_path,current_cluster,parent_cluster);
        if(located.is_pathed == true){
            current = located.current;
            parent = located.parent;
        }
        else{
            located = locate(parent_path,root_cluster,root_cluster);
            if(located.is_pathed == true){
                current = located.current;
                parent = located.parent;
            }
            else{
                return;
            }
        }
    }
    int dir_name_len = int(directory_name.length());
    while(dir_name_len > 13){
        dir_name_len -= 13;
        lfn_count += 1;
    }
    int free_space_offset_start = 0;
    int file_count = 0;
    std::pair<int,int> free_space_pair = check_free_space_in_cluster(current);
    if((lfn_count + 1) <= (entry_count_per_cluster - free_space_pair.first)){
        free_space_offset_start = free_space_pair.second;
        file_count = free_space_pair.first;
        fill_directory_entry(isDirect,directory_name,file_count,lfn_count,free_space_offset_start,date_str,time_str,current,parent);
    }
    else{
        current_first = current;
        while(fat_table[current] != -1){
            std::pair<int,int> return_val = check_free_space_in_cluster(fat_table[current]);
            if((lfn_count + 1) <= (entry_count_per_cluster - return_val.first)){
                free_space_offset_start = free_space_pair.second;
                file_count = free_space_pair.first;
                fill_directory_entry(isDirect,directory_name,file_count,lfn_count,free_space_offset_start,date_str,time_str,current_first,parent);
                return;
            }
            else{
                current = fat_table[current];
            }
        }
        int free_cluster = find_free_cluster();
        int cluster_value = 0x0fffffff;
        std::string update_val = std::bitset<32>(free_cluster).to_string();
        int update_val_int = std::stoi(update_val,0,2);
        lseek(fd,begin_of_fat + current * 4,SEEK_SET);
        write(fd,&update_val_int,4);
        lseek(fd,begin_of_fat + free_cluster * 4,SEEK_SET);
        write(fd,&cluster_value,4);
        get_fat_table_vector();
        free_space_offset_start = get_first_sector_of_cluster(free_cluster);
        fill_directory_entry(isDirect,directory_name,0,lfn_count,free_space_offset_start,date_str,time_str,current_first,parent);
    }
}
void mkdir(char* path){
    std::vector<std::string> folder_array = char_array_to_vector(path);
    int current,parent;
    int lfn_count = 1;
    std::string directory_name;
    bool isDirect = true;
    int current_first;
    std::string time_str = "";
    std::string date_str = "";
    if(folder_array.size() == 1){
        current = current_cluster;
        parent = parent_cluster;
        directory_name = folder_array[0];
    }
    else{
        directory_name = folder_array[folder_array.size() - 1];
        folder_array.pop_back();
        std::vector<std::string> parent_vector = folder_array;
        char* parent_path = vector_to_char_pointer(parent_vector);
        Cluster_Info located = locate(parent_path,current_cluster,parent_cluster);
        if(located.is_pathed == true){
            current = located.current;
            parent = located.parent;
        }
        else{
            located = locate(parent_path,root_cluster,root_cluster);
            if(located.is_pathed == true){
                current = located.current;
                parent = located.parent;
            }
            else{
                return;
            }
        }
    }
    int dir_name_len = int(directory_name.length());
    while(dir_name_len > 13){
        dir_name_len -= 13;
        lfn_count += 1;
    }
    int free_space_offset_start = 0;
    int file_count = 0;
    std::pair<int,int> free_space_pair = check_free_space_in_cluster(current);
    if((lfn_count + 1) <= (entry_count_per_cluster - free_space_pair.first)){
        free_space_offset_start = free_space_pair.second;
        file_count = free_space_pair.first;
        fill_directory_entry(isDirect,directory_name,file_count,lfn_count,free_space_offset_start,date_str,time_str,current,parent);
    }
    else{
        current_first = current;
        while(fat_table[current] != -1){
            std::pair<int,int> return_val = check_free_space_in_cluster(fat_table[current]);
            if((lfn_count + 1) <= (entry_count_per_cluster - return_val.first)){
                free_space_offset_start = free_space_pair.second;
                file_count = free_space_pair.first;
                fill_directory_entry(isDirect,directory_name,file_count,lfn_count,free_space_offset_start,date_str,time_str,current_first,parent);
                return;
            }
            else{
                current = fat_table[current];
            }
        }
        int free_cluster = find_free_cluster();
        int cluster_value = 0x0fffffff;
        std::string update_val = std::bitset<32>(free_cluster).to_string();
        int update_val_int = std::stoi(update_val,0,2);
        lseek(fd,begin_of_fat + current * 4,SEEK_SET);
        write(fd,&update_val_int,4);
        lseek(fd,begin_of_fat + free_cluster * 4,SEEK_SET);
        write(fd,&cluster_value,4);
        get_fat_table_vector();
        free_space_offset_start = get_first_sector_of_cluster(free_cluster);
        fill_directory_entry(isDirect,directory_name,0,lfn_count,free_space_offset_start,date_str,time_str,current_first,parent);
    }
}
void cat(char* path){
    std::vector<std::string> argument_vector = char_array_to_vector(path);
    int current;
    std::string file_name;
    std::string print_val;
    Cluster_Info res;
    std::pair<bool,int> is_file_found;
    if(argument_vector.size() == 1){
        file_name = argument_vector[0];
        is_file_found = find_file(file_name,current_cluster);
        if(is_file_found.first == false){
            return;
        }
        else{
            current = is_file_found.second;
        }
    }
    else{
        file_name = argument_vector[argument_vector.size() - 1];
        argument_vector.pop_back();
        std::vector<std::string> parent_vector = argument_vector;
        char* parent_path = vector_to_char_pointer(parent_vector);
        Cluster_Info located = locate(parent_path,current_cluster,parent_cluster);
        if(located.is_pathed == true){
            is_file_found = find_file(file_name,located.current);
            if(is_file_found.first == true){
                current = is_file_found.second;
            }
            else{
                return;
            }
        }
        else{
            located = locate(parent_path,root_cluster,root_cluster);
            if(located.is_pathed == true){
                is_file_found = find_file(file_name,located.current);
                if(is_file_found.first == true){
                    current = is_file_found.second;
                }
                else{
                    return;
                }
            }
        }
    }
    while(true){
        int offset = get_first_sector_of_cluster(current);
        FatFileEntry fileEntry;
        for (int j = 0; j < int(first_sector.SectorsPerCluster) * BPS; j += 32) {
            lseek(fd, offset + j, SEEK_SET);
            read(fd, &fileEntry, 32);
            for (int i = 0; i < 8; ++i) {
                print_val += char(fileEntry.msdos.filename[i]);
            }
            for (int i = 0; i < 3 ; ++i) {
                print_val += char(fileEntry.msdos.extension[i]);
            }
            print_val += char(fileEntry.msdos.attributes);
            print_val += char(fileEntry.msdos.reserved);
            print_val += char(fileEntry.msdos.creationTimeMs);
            print_val += char(uint8_t(fileEntry.msdos.creationTime));
            print_val += char(uint8_t(fileEntry.msdos.creationTime >> 8));
            print_val += char(uint8_t(fileEntry.msdos.creationDate));
            print_val += char(uint8_t(fileEntry.msdos.creationDate >> 8));
            print_val += char(uint8_t(fileEntry.msdos.lastAccessTime));
            print_val += char(uint8_t(fileEntry.msdos.lastAccessTime >> 8));
            print_val += char(uint8_t(fileEntry.msdos.eaIndex));
            print_val += char(uint8_t(fileEntry.msdos.eaIndex >> 8));
            print_val += char(uint8_t(fileEntry.msdos.modifiedTime));
            print_val += char(uint8_t(fileEntry.msdos.modifiedTime >> 8));
            print_val += char(uint8_t(fileEntry.msdos.modifiedDate));
            print_val += char(uint8_t(fileEntry.msdos.modifiedDate >> 8));
            print_val += char(uint8_t(fileEntry.msdos.firstCluster));
            print_val += char(uint8_t(fileEntry.msdos.firstCluster >> 8));
            print_val += char(uint8_t(uint16_t(fileEntry.msdos.fileSize)));
            print_val += char(uint8_t(uint16_t(fileEntry.msdos.fileSize) >> 8));
            print_val += char(uint8_t(uint16_t(fileEntry.msdos.fileSize >> 16)));
            print_val += char(uint8_t(uint16_t(fileEntry.msdos.fileSize >> 16) >> 8));
        }
        if(fat_table[current] != -1){
            current = fat_table[current];
        }
        else{
            break;
        }
    }
    std::string result = "";
    for (int i = 0; i < print_val.length(); ++i) {
        if(print_val[i] != '\0'){
            result += print_val[i];
        }
        else{
            break;
        }
    }
    std::cout << result << std::endl;
}
int main(int argc, char** argv) {
    fd = open(argv[1],O_RDWR);
    read(fd,&first_sector, sizeof(BPB_struct));
    begin_of_data_sec = int(first_sector.ReservedSectorCount) * BPS + int(first_sector.NumFATs) * int(first_sector.extended.FATSize) * BPS;
    current_cluster = int(first_sector.extended.RootCluster);
    parent_cluster = int(first_sector.extended.RootCluster);
    begin_of_fat = int(first_sector.ReservedSectorCount) * BPS;
    root_cluster = int(first_sector.extended.RootCluster);
    entry_count_per_cluster = int(first_sector.SectorsPerCluster) * BPS / 32;
    get_fat_table_vector();


    auto *pi = new parsed_input;
    std::string inp;
    currentFolder = "";
    while(true){
        std::cout << "/" << currentFolder << ">";
        getline(std :: cin,inp);
        if(inp == "quit"){
            break;
        }
        char *line = new char[inp.length() + 2];
        std::strcpy(line,inp.c_str());
        parse(pi,line);
        if(pi->type == CD){
            cd(pi->arg1);
            clean_input(pi);
        }
        else if(pi->type == LS){
            if(pi->arg1 == nullptr && pi->arg2 == nullptr){
                ls(false, nullptr);
            }
            else if(pi->arg1 != nullptr && pi->arg2 == nullptr){
                std::string arg(pi->arg1);
                if(arg == "-l"){
                    ls(true, nullptr);
                }
                else{
                    ls(false,pi->arg1);
                }
            }
            else{
                ls(true,pi->arg2);
            }
            clean_input(pi);
        }
        else if(pi->type == MKDIR){
            mkdir(pi->arg1);
            clean_input(pi);
        }
        else if(pi->type == TOUCH){
            touch(pi->arg1);
            clean_input(pi);
        }
        else if(pi->type == CAT){
            cat(pi->arg1);
            clean_input(pi);
        }
    }


    return 0;
}
