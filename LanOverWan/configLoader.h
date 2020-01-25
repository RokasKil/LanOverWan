#ifndef CONFIGLOADER_H_INCLUDED
#define CONFIGLOADER_H_INCLUDED
#include <fstream>
#include <string>
#include <vector>

class CFGL{
    public:
        CFGL();
        CFGL(const char *file,std::string comment);
        bool open(const char *file,std::string comment);
        int request(std::string name,std::string def,std::string &result);
        int exists(std::string name);
        int getSize();
        int at(int id,std::string &name,std::string &info);
        std::string operator[] (int id);
        bool operator==(bool b);
        private:
        struct cfgInfo;
        bool failed=true;
        std::vector <cfgInfo> data;
        ///////
        int load(const char *file,std::string comment);


};
///////////////////////////////////
struct CFGL::cfgInfo{
    std::string name;
    std::string info;
    cfgInfo(std::string name, std::string info);
};
////////////////////////////////////////

#endif // CONFIGLOADER_H_INCLUDED
