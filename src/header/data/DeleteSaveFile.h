#ifndef DELETESAVEFILE_H
#define DELETESAVEFILE_H

#include "user/FileManager.h"
#include <string>

class DeleteSaveFile : public FileManager {
    public:
        bool deleteFile(const std::string& username, const std::string& filename);

        
};

#endif