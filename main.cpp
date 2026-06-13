#include <iostream>
#include <unistd.h>
#include <sys/inotify.h>
#include <string>

char buffer[4096];
const struct inotify_event *event;

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cout << "Please include the directory to watch" << argv[0] << "& <directory>\n";
        return -1; 
    }
    const char *filePath = argv[1];
    std::cout << "Watching directory <" << filePath << ">\n";

    int fd = inotify_init();
    std::cout << fd << "\n";

    int wd = inotify_add_watch(fd, filePath, IN_CLOSE_NOWRITE | IN_DELETE | IN_CREATE);
    std::cout << "wd = " << wd << "\n";
    if (wd == -1) perror("inotify_add_watch");

   while(true) {
        ssize_t size = read(fd, buffer, sizeof(buffer));
        if(size < 0) perror("could not read the file");
        
        //inotify_event struct has a field called name after the struct in memory, so to walk through u need to do the below to get to next struct
        for(char *ptr = buffer; ptr < buffer + size; ptr += sizeof(struct inotify_event) + event->len) {
            event = reinterpret_cast<const struct inotify_event*>(ptr);
            if(event->mask & IN_CREATE) std::cout << "Created: " << event->name << "\n";
            else if(event-> mask & IN_DELETE) std::cout << "Deleted: " << event->name << "\n";
        }
   }
    return 0;
}