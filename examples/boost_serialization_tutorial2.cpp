#include <iostream>
#include <fstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// GPS-coordinates
// illustrates serializing a simple type

class GpsPosition
{
// Methods
public:
    GpsPosition() {};
    GpsPosition(int p_degrees, int p_minutes, float p_seconds)
        : degrees(p_degrees), minutes(p_minutes), seconds(p_seconds)
    {};

    ~GpsPosition(){};

    friend class boost::serialization::access;

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

// This time the serialization function has been moved outside
// of the class. This however requires members to be public!
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


int main(int argc, char* argv[])
{
    // create and open a character archive for output
    std::ofstream l_ofstream("filename.txt");

    // create class instance and save data to archive
    const GpsPosition l_gpsPos(35, 59, 24.467f);
    l_gpsPos.Print();
    {
        boost::archive::text_oarchive l_oArchive(l_ofstream);
        // write class instance to archive
        l_oArchive << l_gpsPos;
    }

    // Do stuff ...
    GpsPosition l_newGpsPos(14, 23, 2.345f);
    l_newGpsPos.Print();

    // Sometime later, restore the class instance to its original state;
    {
        // create and open an archive for input
        std::ifstream l_ifstream("filename.txt");
        boost::archive::text_iarchive l_iArchive(l_ifstream);
        // read class state from archive
        l_iArchive >> l_newGpsPos;
        // archive and stream closed when out of scope (calls destructor)
    }
    l_newGpsPos.Print();

    std::cin.get();
    return 0;
}