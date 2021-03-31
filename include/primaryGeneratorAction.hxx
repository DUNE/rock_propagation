//Author Dom Brailsford
//Class that defines the primary generator action as needed by geant4.
//The class accumulates GHEP particle pointers (usually passed to it from a GHEP event record) and then (for each GHEP particle), creates a G4 particle, a G4 vertex and adds them to the G4 event for tracking

//STL
//ROOT
//G4
#include "G4ParticleTable.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4Event.hh"
//GENIE
#include "GHEP/GHepParticle.h"
#include "EVGCore/EventRecord.h"


namespace rockprop{
  class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
    public:
      PrimaryGeneratorAction() {}; //Constructor
      void Append(genie::GHepParticle* particle) { fParticles.push_back(particle); }; //Store GHEP particle, ready for adding to the G4 event to be tracked
      virtual void GeneratePrimaries(G4Event*); //Where the magic happens (the function that gets called by geant 4)
      void Reset() { fParticles.clear(); }; //tidy up function so we are ready to start again
      void SetEventRecord(genie::EventRecord const * event) { fEventRecord = event; }; //Setting the underlying genie event record pointer
    private:
      static G4ParticleTable* fParticleTable; //A handy PDG table to check that a particular particle pdg exists
      std::vector<genie::GHepParticle*> fParticles; //The vector of GHEP particles get added to the G4 event for tracking
      genie::EventRecord const * fEventRecord; //The underlying genie event record TODO we only need the event record for the vertex position so drop this in the future
  };
}
