#ifndef CONFIGLOADER_H_INCLUDED
#define CONFIGLOADER_H_INCLUDED

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
    cfgInfo(std::string name,std::string info){
        this->name=name;
        this->info=info;
    }
};
////////////////////////////////////////
CFGL::CFGL(){

}

CFGL::CFGL(const char *file,std::string comment){
    open(file,comment);
}

bool CFGL::open(const char *file,std::string comment){
    if(file==NULL||load(file,comment)!=0){
        failed=true;
    }
    else{
        failed=false;
    }
    return !failed;
}

int CFGL::request(std::string name,std::string def,std::string &result){
    int id;
    if((id=exists(name))!=-1){
        result=data[id].info;
        return id;
    }
    else{
        result=def;
        return -1;
    }
}

int CFGL::exists(std::string name){
    for(int i=0;i<data.size();i++){
        if(data[i].name==name){
            return i;
        }
    }
    return -1;
}

int CFGL::getSize(){
    return data.size();
}

int CFGL::load(const char *file,std::string comment){
    std::string line;
    int pos1;
    std::ifstream in(file);
    if(!in){
        in.close();
        return 1;
    }
    while(!in.eof()){
        getline(in,line);
        if(comment!=""&&(pos1=line.find(comment))!=-1){
            line=line.substr(0,pos1);
        }
        if(line=="")
            continue;
        if((pos1=line.find('='))==-1){
            data.push_back({line,""});
        }
        else{
            data.push_back({line.substr(0,pos1),line.substr(pos1+1)});
        }
    }
    in.close();
    return 0;
}

int CFGL::at(int id,std::string &name,std::string &info){
    if(id<0||id>=data.size()){
        return -1;
    }
    else{
        name=data[id].name;
        info=data[id].info;
        return 0;
    }
}

std::string CFGL::operator[] (int id){
    if(id<0||id>=data.size()){
        return "";
    }
    else{
        return data[id].info;
    }
}

bool CFGL::operator==(bool b){
    return b!=failed;
}

#endif // CONFIGLOADER_H_INCLUDED
