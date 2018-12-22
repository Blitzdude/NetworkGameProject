Boost kirjaston asennus Network Projektia varten:
1. Lataa Boost.1.68.0 - kirjasto
2. pura zip- Third party kansioon
3. aja koment bootstrap
4. b2 variant=debug link=static threading=multi address-model=32 runtime-link=static
---4. aja Komento :  b2 release define=BOOST_USE_WINAPI_VERSION=0x0A00 stage ( Windows 10 versio)

- aseta visual studio c++ language versioksi c++14

b2 variant=debug link=static threading=multi address-model=32 runtime-link=static

Muuta seuraavat asetukset NetworkGameClient, ja NetworkGameServerin properties valikoista
(right-click project -> properties)

- Additional Include Directories:
	- Lisää ..\third_party\boost_1.68.0\
- Code Generation
	- Vaihda Runtime Library -> Multi-Threaded Debug
- Language
	- C++ Language Standard -> ISO C++14 Standard
- Additional Linker files:
	- Lisää ..\third_party\boost_1.68.0\stage\lib
- Linker -> Input
	- Lisää libboost_system-vc141-mt-sgd-x32-1_68.lib (b2 komennon jälkeen)
	
Network clientin rakentamisessa syntyy ongelmia, koska sekä boost-kirjast ja olcPixelGameEngine sisällyttävä windowsin 
#include windows.h tiedoston. Tämä ratkaistaa lisäämällä PreProcessor makrot, jotka kieltävät boostia lisäämästä windwsin headerietä

