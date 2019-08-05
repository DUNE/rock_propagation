//Author Dom Brailsford
//
//Class represents a 'world' detector geometry (which needs to include volDetEnclosure and/or volDetector volumes) as needed by geant4
//The world should nominally represent a detector cavern surrounded by rock (cavern can be empty if you like)
//This class is very minimal...

//STL
//ROOT
//G4
#include "G4VUserDetectorConstruction.hh"
//RockProp

namespace rockprop{
  class WorldConstruction : public G4VUserDetectorConstruction
  {
    public:
      WorldConstruction(G4VPhysicalVolume *setWorld = 0) :
        fWorld(setWorld) {};
      virtual G4VPhysicalVolume* Construct() { return fWorld; };
    private:
      G4VPhysicalVolume *fWorld;
  };
}
