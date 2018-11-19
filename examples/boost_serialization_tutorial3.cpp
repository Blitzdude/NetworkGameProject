#include <iostream>
#include <fstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

/*
    Boost_serializtion_tutorial3 
    Serializable class with serializable members
*/

class GpsPosition
{
// Methods
public:
    GpsPosition() {};
    GpsPosition(int p_degrees, int p_minutes, float p_seconds)
        : degrees(p_degrees), minutes(p_minutes), seconds(p_seconds)
    {};

    ~GpsPosition(){};

    //friend class boost::serialization::access;

    void Print() const
    {
        std::cout << degrees << " " << minutes << " " << seconds << std::endl;
    }

private:
public:
    int degrees;
    int minutes;
    float seconds;


// Members
private:
};



namespace boost {
namespace serialization {

// When the class Archive corresponds to an output archive, the
// & operator is defined similar to <<.  Likewise, when the class Archive
// is a type of input archive the & operator is defined similar to >>.
template<class Archive>
void serialize(Archive& p_archive, GpsPosition& p_gpsPos, const unsigned int p_version)
{
    p_archive & p_gpsPos.degrees;
    p_archive & p_gpsPos.minutes;
    p_archive & p_gpsPos.seconds;
}

} // namespace serialization
} // namespace boost

class BusStop
{
    // Methods
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& p_archive, const unsigned int p_version)
    {
        p_archive & latitude;
        p_archive & longitude;
    }

public:
    BusStop(const GpsPosition& p_latitude, const GpsPosition& p_longitude)
        : latitude(p_latitude), longitude(p_longitude)
    {}

    BusStop(){}
    // See item # 14 in Effective C++ by Scott Meyers.
    // re non-virtual destructors in base classes.
    virtual ~BusStop(){}

    void Print() const
    {
        latitude.Print();
        longitude.Print();
    }

    // Members
private:
public: 
    GpsPosition latitude;
    GpsPosition longitude;
};


int main(int argc, char* argv[])
{
    // create and open a character archive for output
    std::ofstream l_ofstream("filename.txt");

    // create class instance and save data to archive
    const BusStop l_busStop({35, 59, 24.467f}, {23,45,25.544f});
    l_busStop.Print();
    std::cout << std::endl;

    {
        boost::archive::text_oarchive l_oArchive(l_ofstream);
        // write class instance to archive
        l_oArchive << l_busStop;
    }

    // Do stuff ...
    BusStop l_newBusStop({14, 23, 2.345f}, {1, 2, 3.456f});
    l_newBusStop.Print();
    std::cout << std::endl;

    // Sometime later, restore the class instance to its original state;
    {
        // create and open an archive for input
        std::ifstream l_ifstream("filename.txt");
        boost::archive::text_iarchive l_iArchive(l_ifstream);
        // read class state from archive
        l_iArchive >> l_newBusStop;
        // archive and stream closed when out of scope (calls destructor)
    }
    l_newBusStop.Print();

    std::cin.get();
    return 0;
}