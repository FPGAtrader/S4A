//https://stackoverflow.com/questions/39709328/why-does-perf-and-papi-give-different-values-for-l3-cache-references-and-misses


#include <iostream>

#if defined __GNUC__

#include <papi.h>

using namespace std;

struct node{
    int l, r;
};

void handle_error(int err){
    std::cerr << "PAPI error: " << err << std::endl;
}

int main(int argc, char* argv[])
{

    int n = 1000000;
    node* A = new node[n];
    int i;

    //warm up
    for(i=0;i<n;i++){
        A[i].l = 0;
        A[i].r = 0;
    }

    int numEvents = 2;
    long long values[2];
    int events[2] = {PAPI_L3_TCA,PAPI_L3_TCM};
    if (PAPI_start_counters(events, numEvents) != PAPI_OK)
        handle_error(1);

    for(i=0;i<n;i++){
        A[i].l = 1;
        A[i].r = 4;
    }

    if ( PAPI_stop_counters(values, numEvents) != PAPI_OK)
        handle_error(1);

    cout<<"L3 accesses: "<<values[0]<<endl;
    cout<<"L3 misses: "<<values[1]<<endl;
    cout<<"L3 miss/access ratio: "<<(double)values[1]/values[0]<<endl;

    return 0;
}
#else
int
main(int argc, char **argv)
{
    printf("Skip %s for GNU CC only.", __FILE__);
    return 0;
}
#endif