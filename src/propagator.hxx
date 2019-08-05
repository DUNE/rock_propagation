//Author Dom Brailsford
//The main class that ties everything togeth and runs it
//The class is in charge of
//  -command line parsing
//  -reading in the genie event record from a TFile
//  -Extracting the GHEP Particles from the genie event record
//  -Passing the ghep particles to the primary generator action
//  -Running geant4
//  -Storing the newly propagated G4 particles (that have reached the cavern) back in the GHEP event record
//  -Copying other stuff from the input TFile to the output TFile

//STL
#include <string>
#include <ctime>
#include <memory>

//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TIterator.h"

//GENIE
#include "Ntuple/NtpWriter.h"
#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCEventRecord.h"
#include "Utils/CmdLnArgParser.h"
#include "FluxDrivers/GSimpleNtpFlux.h"

//G4
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "QGSP_BERT_HP.hh"
#include "G4Navigator.hh"
#include "G4GDMLParser.hh"

//ROCKPROP
#include "steppingAction.hxx"
#include "primaryGeneratorAction.hxx"
#include "worldConstruction.hxx"

namespace rockprop{
  class Propagator {
    public:
      Propagator(int argc, char **argv); //Constructor
      int RunPropagation(); //The main function that does everything
    private:
      //CLI variables
      int fOptNEvt;
      long fOptRandomSeed;
      string fOptGeometryFileName;
      string fOptInputFilename;
      string fOptOutPrefix;
      //Counting variables
      std::map<int,int> fNPDGToNRockParticlesMap; //counts the number of cavern-entering particles, separated by PDG
      int fNRockVertices; //Counts the numbrer of GHEP vertices which contain a cavern-entering particles

      std::vector<std::unique_ptr<genie::GHepParticle> > PropagateAndFindCavernEnteringParticlesInEvent(genie::EventRecord &event, std::unique_ptr<G4RunManager> &run_manager); //Runs geant4 and returns a vector of GHePParticles which successfully reached the detector cavern
      void UpdateEventWithPropagatedParticles(genie::EventRecord &event, std::vector<std::unique_ptr<genie::GHepParticle> > cavern_entering_ghep_particles); //Updates the genie event record with a vector of GHEP particles which reached the detector cavern
      void GetCommandLineArgs(int argc, char ** argv); //Parse the command line arguments
      void PrintCommandLineUsageOptions(); //Print usage options
  };
}
