#include <string>
#include <iostream>
#include <thread>
//#include <unistd.h>
// c embedding py
#include <stdio.h>
#include <python2.7/Python.h>
// c call java
//#include <stdio.h> // instead of stdafx.h in VC++
#include <stdlib.h> // malloc
//#include <iostream>
//#include <string.h>
#include <jni.h>

using namespace std;

void task1(string msg)
{
   
}

void task2()
{
}

int main()
{
    thread t2(task2);
    thread t1(task1, "hello");
    t1.join();
    t2.join();
    return 0;
}