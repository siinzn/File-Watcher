#include <iostream>
#include <unistd.h>
#include <sys/inotify.h>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    char buffer[4096];
    const struct inotify_event *event;
    std::unordered_map<int, fs::path> dirMap;

    if(argc != 2) {
        std::cout << "Please include the directory to watch" << argv[0] << "& <directory>\n";
        return -1; 
    }
    const char *filePath = argv[1];
    std::cout << "Watching directory <" << filePath << ">\n";

    int fd = inotify_init();
    if(fd == -1) perror("inotify_init");
    
    int mainFileWd = inotify_add_watch(fd, filePath, IN_CLOSE_NOWRITE | IN_DELETE | IN_CREATE);
    dirMap.insert({mainFileWd, filePath});

    try{
        if(fs::exists(filePath) && fs::is_directory(filePath)){
            for(const auto& entry : fs::recursive_directory_iterator(filePath)){
                int wd = inotify_add_watch(fd, entry.path().c_str(), IN_CLOSE_NOWRITE | IN_DELETE | IN_CREATE);
                dirMap.insert({wd, entry.path()}); 
            }
        }
    }
    catch(const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    
   while(true) {
        ssize_t size = read(fd, buffer, sizeof(buffer));
        if(size < 0) perror("could not read the file");
        
        //inotify_event struct has a field called name after the struct in memory, so to walk through u need to do the below to get to next struct
        for(char *ptr = buffer; ptr < buffer + size; ptr += sizeof(struct inotify_event) + event->len) {
            event = reinterpret_cast<const struct inotify_event*>(ptr);
            auto it = dirMap.find(event->wd);
            if(it != dirMap.end()) {
                if(event->len == 0) continue;
                fs::path fullPath = it->second / event->name;
                if(event->mask & IN_CREATE) {
                    if(event->mask & IN_ISDIR) {
                        int twd = inotify_add_watch(fd, fullPath.c_str(), IN_CLOSE_NOWRITE | IN_DELETE | IN_CREATE);
                        dirMap.insert({twd, fullPath});
                    }
                    std::cout << "Created: " << fullPath << "\n";
                }
                else if(event-> mask & IN_DELETE) std::cout << "Deleted: " << fullPath  << "\n";
            }
            
        }
            
   }
    return 0;
}