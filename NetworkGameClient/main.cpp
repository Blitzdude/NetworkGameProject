#include <iostream>
#include <string>
#include "mainGame.h"
// #include <chrono>
// #include <thread>
// TODO: Clean this main!
//  typedef std::chrono::duration<int, std::ratio<1, 60>> frame_duration;



/*
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>                                         
struct Foo
{
    int x;
    float y;
    double z;
    bool w;

    void Print()
    {
        std::cout << x << " " 
                  << y << " "
                  << z << " "
                  << w << "\n";
    }
};

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, Foo & f, const unsigned int version)
{
    ar & f.x;
    ar & f.y;
    ar & f.z;
    ar & f.w;
}

}
}

*/
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: TagOrDie <ip-address> <port>\n";
        return 1;
    }
    
    /*
    auto start_time = std::chrono::system_clock::now();
    auto end_time = std::chrono::system_clock::now() + frame_duration(1);
    for (int i = 0; i < 100; i++)
    {
        std::cout << "Hello World" << std::endl;
    }
    auto end_loop = std::chrono::system_clock::now();
    */



    /*
    int numberOfObjects = 30;


    std::ostringstream oss;
    boost::archive::binary_oarchive oar(oss);
    oar << numberOfObjects;
    for (int i = 0; i < numberOfObjects; ++i)
    {
        Foo fooTemp{1 * i, 1.123f * i, 2.456 * i, i % 2 ? true : false};
        oar << fooTemp;
    }

    std::string myString = oss.str();
    // --------------
    std::istringstream iss(myString);
    boost::archive::binary_iarchive iar(iss);
    int recNumObj;
    iar >> recNumObj;

    std::vector<Foo> v_foos;
    for (int i = 0; i < recNumObj; ++i)
    {
        Foo push_foo;
        iar >> push_foo;
        v_foos.push_back(push_foo);
    }

    for (auto itr : v_foos)
        itr.Print();
    */

    
    MainGame app(argc, argv);
    if (app.Construct(800, 600, 1, 1))
        app.Start();
    
    // std::cin.get();
    return 0;
}