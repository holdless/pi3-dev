#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

void task1(string msg)
{
    cout<<endl<<"task1 says: "<<msg<<endl;
    sleep(1);
    cout<<endl<<"task1 says: "<<msg<<endl;
    sleep(1);
    cout<<endl<<"task1 says: "<<msg<<endl;
    sleep(1);
    cout<<endl<<"task1 says: "<<msg<<endl;
    sleep(1);
    cout<<endl<<"task1 says: "<<msg<<endl;
    sleep(1);
}

void task2()
{
    cout<<endl<<"i'm task2"<<endl;
    sleep(2);
    cout<<endl<<"i'm task2"<<endl;
    sleep(2);
    cout<<endl<<"i'm task2"<<endl;
    sleep(2);
}

int main()
{
    thread t2(task2);
    thread t1(task1, "hello");
    t1.join();
    t2.join();
    return 0;
}
