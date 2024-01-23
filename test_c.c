static int global=1;
int func(){
    
    return 0;
}

int funcaaa(){
    return 0;
}

int main(){
    funcaaa();
    int a;
    global = 0;
    a = 2;
    func();
}

