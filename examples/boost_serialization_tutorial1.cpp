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
        : m_degrees(p_degrees), m_minutes(p_minutes), m_seconds(p_seconds)
    {};

    ~GpsPosition(){};

private:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive& p_archive, const unsigned int p_version)
    {
        p_archive & m_degrees;
        p_archive & m_minutes;
        p_archive & m_seconds;
    }
public:
    void Print() const
    {
        std::cout << m_degrees << " " << m_minutes << " " << m_seconds << std::endl;
    }

// Members
private:
    int m_degrees;
    int m_minutes;
    float m_seconds;
};

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