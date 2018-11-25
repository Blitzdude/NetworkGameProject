#include <iostream>
#include <fstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

/*
    TODO: finish tutorials
    Boost_serialize_tutorial 5
    Pointers
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

class BusStopCorner : public BusStop
{
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& p_archive, const unsigned int p_version)
    {
        // Serialize base class information first
        p_archive & boost::serialization::base_object<BusStop>(*this);
        p_archive & m_street1;
        p_archive & m_street2;
    }
    
    virtual std::string description() const // Needed by boost::serialization ???
    {
        return m_street1 + " and " + m_street2;
    }

public:    
    BusStopCorner() {};
    BusStopCorner(const GpsPosition& p_latitude, const GpsPosition& p_longitude,
        const std::string& p_street1, const std::string& p_street2)
        : BusStop(p_latitude, p_longitude), m_street1(p_street1), m_street2(p_street2)
    {};

    ~BusStopCorner() override {};

    void Print() const 
    {
        std::cout << description() << std::endl;
    };

// Members
private:
    std::string m_street1;
    std::string m_street2;
};

class BusRoute
{
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& p_archive, const unsigned int p_version)
    {
        for (int i = 0; i < 10; ++i)
            p_archive & stops[i];
    }
public:
    BusRoute(){};
    
// members
private:
    BusStop* m_stops[10];
};

int main(int argc, char* argv[])
{
    // create and open a character archive for output
    std::ofstream l_ofstream("filename.txt");

    // create class instance and save data to archive
    const BusStopCorner l_busStopCorner({35, 59, 24.467f}, {23,45,25.544f}, "Wall Street", "Wayne Street");
    l_busStopCorner.Print();

    {
        boost::archive::text_oarchive l_oArchive(l_ofstream);
        // write class instance to archive
        l_oArchive << l_busStopCorner;
    }

    // Do stuff ...
    BusStopCorner l_newBusStopCorner({14, 23, 2.345f}, {1, 2, 3.456f}, "Alabaster Avenue", "Lordi's Street");
    l_newBusStopCorner.Print();

    // Sometime later, restore the class instance to its original state;
    {
        // create and open an archive for input
        std::ifstream l_ifstream("filename.txt");
        boost::archive::text_iarchive l_iArchive(l_ifstream);
        // read class state from archive
        l_iArchive >> l_newBusStopCorner;
        // archive and stream closed when out of scope (calls destructor)
    }
    l_newBusStopCorner.Print();

    std::cin.get();
    return 0;
}