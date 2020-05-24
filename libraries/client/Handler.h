//
// Created by mateusz on 24.05.2020.
//

#ifndef MFS_HANDLER_H
#define MFS_HANDLER_H

#include <string>

class Handler {
public:
    static std::string getDirectory(const std::string& path) {
        std::string filename(path);
        std::string directory;
        const size_t last_slash_idx = filename.rfind('/');
        if (std::string::npos != last_slash_idx)
            directory = filename.substr(0, last_slash_idx);
        return directory;
    }

    static std::string getFileName(const std::string& path) {
        std::string filename(path);
        std::string name;
        const size_t last_slash_idx = filename.rfind('/');
        if (std::string::npos != last_slash_idx)
            name = filename.substr(last_slash_idx + 1, filename.size() - 1);
        return name;
    }
    static FileStatus getStatus(const int& mode) {
        if(mode == FileStatus::WRONLY)
            return FileStatus::WRONLY;
        else if(mode == FileStatus::RDONLY)
            return FileStatus::RDONLY;
        else if(mode == FileStatus::RDWR)
            return FileStatus::RDWR;
        else
            throw std::invalid_argument("Wrong file open mode");
    }
};
#endif //MFS_HANDLER_H
