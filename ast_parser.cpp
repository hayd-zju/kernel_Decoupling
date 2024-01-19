/*
1.建模：提取函数定义、全局变量、宏定义，建立图中节点，提取函数调用、变量调用和宏调用。
*/

extern "C" {
    #include "clang-c/Index.h"
}
#include "llvm/Support/CommandLine.h"
#include <iostream>

using namespace llvm;

static cl::opt<std::string>
FileName(cl::Positional, cl::desc("Input file"), cl::Required);
/*
回调函数：返回枚举CXChildVisitResult 的一个成员值
三个参数：
    1.代表我们当前正在访问的AST 节点的 cursor；
    2.代表这个节点的父节点的另一个 cursor；
    3.以及一个CXClientData 对象，它是 void 指针的 typedef，这个指针能够在不同回调函数调用之间传递任意的数据结构，其中包含想维护的状态；
*/
enum CXChildVisitResult visitNode (CXCursor cursor, CXCursor parent, CXClientData client_data) {
    // if(clang_getCursorKind(cursor) == CXCursor_FunctionDecl){
    //     if(clang_Cursor_hasVarDeclExternalStorage(cursor)){//TODO:判断是否有外部变量的引用
    //         CXString cursor_name = clang_getCursorSpelling(cursor);
    //         std::cout << "clang_Cursor_hasVarDeclGlobalStorage: " << clang_getCString(cursor_name) << std::endl;
    //     }
    //     return CXChildVisit_Recurse;
    // }
    parent = clang_getCursorSemanticParent(cursor)
    if(clang_getCursorKind(cursor) == CXCursor_CallExpr){//函数调用
        printf("CXCursor_CallExpr--------\n");
        CXString cursor_name = clang_getCursorSpelling(cursor);
        CXString parent_name = clang_getCursorSpelling(parent);
        std::cout << "call function: " << clang_getCString(cursor_name) << " in: " << clang_getCString(parent_name) << std::endl;
        return CXChildVisit_Continue;
    }
    if(clang_getCursorKind(cursor) == CXCursor_DeclRefExpr){//refers to some value declaration, such as a function, variable, or enumerator.
        CXString cursor_name = clang_getCursorSpelling(cursor);
        CXString parent_name = clang_getCursorSpelling(parent);
        std::cout << "call: " << clang_getCString(cursor_name) << " in: " << clang_getCString(parent_name) << std::endl;
        return CXChildVisit_Continue;
    }


    if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl || clang_getCursorKind(cursor) == CXCursor_VarDecl) {//函数声明或全局变量
        CXString name = clang_getCursorSpelling(cursor);
        CXSourceLocation loc = clang_getCursorLocation (cursor);
        CXString fName;
        unsigned line = 0, col = 0;
        clang_getPresumedLocation(loc, &fName, &line, &col);
        std::cout << clang_getCString(fName) << ":" << line << ":" << col << " declares " << clang_getCString(name) << std::endl;
        clang_disposeString(fName);
        clang_disposeString(name);
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
    clang_visitChildren(cur, visitNode, NULL);
    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(index);
    return 0;
}