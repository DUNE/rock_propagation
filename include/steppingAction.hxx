//Author Dom Brailsford
//Class is a geant4 stepping action.  The class uses the G4Steps to detect when a particle enters the 'volDetEnclosure' or 'volDetector' volumes, 
//creates a genie::GHepParticle out of the particle state at that point, stores that GHEpPArticle and then kills that particle's tracking
//The stored GHepParticles can be picked up later on

//STL
#include <memory>
//ROOT
//GENIE
#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
//G4
#include "G4UserSteppingAction.hh"
#include "G4Step.hh"

namespace rockprop{
  class SteppingAction : public G4UserSteppingAction {
    public:
      SteppingAction() : 
        fMotherID(-9999) {}; //Constructor
      virtual ~SteppingAction(){};
      void ResetCavernParticleGHepVector() { fSavedGHepParticles.clear(); }; //Tidy up function
      std::vector<std::unique_ptr<genie::GHepParticle> > HandOverPropagatedGHepParticles(); //Relinquish control of all created ghep particles after the propagation has finished
      void SetEventRecord(genie::EventRecord const * event) { fEventRecord = event; };
      void SetMotherID(int mother_id) { fMotherID = mother_id; }; //Store the GHEP event record ID so that we can use it when creating GHEP Particles from the tracked g4 particles
      virtual void UserSteppingAction(const G4Step* step); //Where all of the magic happens (this function is run in geant4)

    private:
      std::vector<std::unique_ptr<genie::GHepParticle> > fSavedGHepParticles;
      int fMotherID; //The 'mother' GHEPRecord ID of the particle being processed.  The mother ID here refers to the GHEPEventRecord ID of the original particle added to the particle stack. It's entirely possible (and likely) that the mother GHEP ID actually points back to the same particle in ghep
      genie::EventRecord const * fEventRecord; //A copy of the event record TODO we only need the event record for the vertex position so lets try dropping this in the future

  };
}
