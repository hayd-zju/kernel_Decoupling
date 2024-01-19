#ifndef MODEL_H  
#define MODEL_H  

#include<vector>
#include<string>
/*
节点类
*/
class Node{
    public:
        int type;   //节点类型：0-函数节点，1-全局变量节点
        std::string name;    //函数名or变量名
        
};


#endif