/*
1.建模：提取函数定义、全局变量、宏定义，建立图中节点，提取函数调用、变量调用和宏调用。
*/

extern "C" {
    #include "clang-c/Index.h"
}
#include "llvm/Support/CommandLine.h"
#include <iostream>

using namespace llvm;

static cl::opt<std::string> FileName(cl::Positional, cl::desc("Input file"), cl::Required);
CXCursorSet cset;//节点set
/*
得到该引用所在的函数体cursor
*/
CXCursor getVarFromFunc(CXCursor cursor){
    if(clang_isDeclaration(clang_getCursorKind(cursor)) == 1)
        return cursor;
    return getVarFromFunc(clang_getCursorSemanticParent(cursor));
}

/*
TODO:1.遍历到CXCursor_FunctionDecl，添加节点信息，返回CXChildVisit_Recurse继续遍历子节点
     2.遍历到CXCursor_VarDecl，添加节点信息，返回CXChildVisit_Continue略过当前节点子节点
     3.遍历到CXCursor_CallExpr，添加边信息，返回CXChildVisit_Continue略过当前节点子节点
     4.遍历到CXCursor_DeclRefExpr（function, variable, or enumerator），添加边信息，返回CXChildVisit_Continue略过当前节点子节点
回调函数：返回枚举CXChildVisitResult 的一个成员值
三个参数：
    1.代表我们当前正在访问的AST 节点的 cursor；
    2.代表这个节点的父节点的另一个 cursor；
    3.以及一个CXClientData 对象，它是 void 指针的 typedef，这个指针能够在不同回调函数调用之间传递任意的数据结构，其中包含想维护的状态；
*/
enum CXChildVisitResult visitNode (CXCursor cursor, CXCursor parent, CXClientData client_data) {
    if (clang_getCursorKind(cursor) == CXCursor_VarDecl) {//全局变量
        CXString name = clang_getCursorSpelling(cursor);
        CXSourceLocation loc = clang_getCursorLocation (cursor);
        CXString fName;
        unsigned line = 0, col = 0;
        clang_getPresumedLocation(loc, &fName, &line, &col);
        std::cout << clang_getCString(fName) << ":" << line << ":" << col << " global var declares " << clang_getCString(name) << std::endl;
        clang_disposeString(fName);
        clang_disposeString(name);

        //TODO:添加节点信息
        clang_CXCursorSet_insert(cset,cursor);

        return CXChildVisit_Continue;
    }

    if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {//函数声明
        CXString name = clang_getCursorSpelling(cursor);
        CXSourceLocation loc = clang_getCursorLocation (cursor);
        CXString fName;
        unsigned line = 0, col = 0;
        clang_getPresumedLocation(loc, &fName, &line, &col);
        std::cout << clang_getCString(fName) << ":" << line << ":" << col << " func declares " << clang_getCString(name) << std::endl;
        clang_disposeString(fName);
        clang_disposeString(name);

        //TODO:添加节点信息
        clang_CXCursorSet_insert(cset,cursor);

        return CXChildVisit_Continue;
    }
    return CXChildVisit_Continue;
}

enum CXChildVisitResult visitEdge (CXCursor cursor, CXCursor parent, CXClientData client_data) {
    if(clang_getCursorKind(cursor) == CXCursor_CallExpr){//函数调用
        parent = getVarFromFunc(parent);//parent = clang_getCursorSemanticParent(parent);//消除CompoundStmt函数体的干扰
    
        CXString cursor_name = clang_getCursorSpelling(cursor);
        CXString parent_name = clang_getCursorSpelling(parent);

        //todo:添加边信息
        std::cout << "call function: " << clang_getCString(cursor_name) << " in: " << clang_getCString(parent_name) << std::endl;
        
        return CXChildVisit_Continue;
    }
    //TODO:识别全局变量引用
    else if(clang_getCursorKind(cursor) == CXCursor_DeclRefExpr){
        parent = getVarFromFunc(parent);//TOOD:所属函数体，前面有局部变量声明会有影响

        CXString cursor_name = clang_getCursorSpelling(cursor);
        CXString parent_name = clang_getCursorSpelling(parent);

        //todo:添加边信息
        std::cout << "-----call function: " << clang_getCString(cursor_name) << " in: " << clang_getCString(parent_name) << std::endl;
        return CXChildVisit_Continue;
    }
    return CXChildVisit_Recurse;
}

int main(int argc, char** argv){
    cl::ParseCommandLineOptions(argc, argv, "AST Traversal Example\n");
    CXIndex index = clang_createIndex(0,0);
    const char *args[] = {"-I/usr/include", "-I."};
    CXTranslationUnit translationUnit = clang_parseTranslationUnit(index, FileName.c_str(), args, 2, NULL, 0, CXTranslationUnit_None);
    
    CXCursor cur = clang_getTranslationUnitCursor(translationUnit);
    
    cset = clang_createCXCursorSet();//cursorset初始化

    //添加点
    clang_visitChildren(cur, visitNode, NULL);

    //添加边
    clang_visitChildren(cur, visitEdge, NULL);

    clang_disposeTranslationUnit(translationUnit);//translationUnit释放
    clang_disposeIndex(index);//index释放

    clang_disposeCXCursorSet(cset);//cursorset释放
    return 0;
}