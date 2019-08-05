#include "propagator.hxx"

rockprop::Propagator::Propagator::Propagator(int argc, char ** argv) :
  fOptNEvt(-9999),
  fOptRandomSeed(-9999),
  fOptInputFilename("default_file_name.root"),
  fOptOutPrefix("default_output_prefix"),
  fNRockVertices(-9999)
{
  GetCommandLineArgs(argc, argv);
}

int rockprop::Propagator::RunPropagation(){
  //A bit of initialisation
  fNRockVertices = 0;
  //Setup all of the g4 stuff
  G4Random::setTheSeed(fOptRandomSeed);
  std::unique_ptr<G4RunManager> runManager = std::unique_ptr<G4RunManager>(new G4RunManager);
  G4GDMLParser parser;
  parser.Read(fOptGeometryFileName);
  runManager->SetUserInitialization(new QGSP_BERT_HP);
  runManager->SetUserInitialization(new rockprop::WorldConstruction(parser.GetWorldVolume()));
  runManager->SetUserAction(new rockprop::PrimaryGeneratorAction());
  runManager->SetUserAction(new rockprop::SteppingAction());
  runManager->Initialize();

  //Get the genie tree from the input file
  TFile input_file(fOptInputFilename.c_str(),"READ");
  TTree *input_tree = (TTree*) input_file.Get("gtree");
  if(!input_tree) {
    std::cout<<"rockprop::Propagator -- could not find gtree in " << fOptInputFilename << ".  Exiting!" << std::endl;
    return 1;
  }

  //Grab all of the stuff from the genie tree that we will either analyse or copy straight over to the output file
  genie::NtpMCEventRecord *ntp_event_record = nullptr;
  input_tree->SetBranchAddress("gmcrec", &ntp_event_record);

  genie::flux::GSimpleNtpEntry *gsimple_entry = nullptr;
  input_tree->SetBranchAddress("simple",&gsimple_entry);

  genie::flux::GSimpleNtpNuMI *gsimple_numi = nullptr;
  input_tree->SetBranchAddress("numi",&gsimple_numi);

  genie::flux::GSimpleNtpAux *gsimple_aux = nullptr;
  input_tree->SetBranchAddress("aux",&gsimple_aux);

  //Figure out how many 'events' we will loop over
  int nevents = (fOptNEvt > 0) ?
        TMath::Min(fOptNEvt, (int)input_tree->GetEntries()) :
        (int) input_tree->GetEntries();


  //Setup the output file
  genie::NtpWriter ntp_writer(genie::kNFGHEP, 0);
  ntp_writer.CustomizeFilenamePrefix(fOptOutPrefix.c_str());
  ntp_writer.Initialize();
  ntp_writer.EventTree()->Branch("simple",&gsimple_entry);
  ntp_writer.EventTree()->Branch("numi",&gsimple_numi);
  ntp_writer.EventTree()->Branch("aux",&gsimple_aux);
  ntp_writer.EventTree()->SetWeight(input_tree->GetWeight());


  //
  // Loop over the events in the input tree
  //
  std::cout<<"rockprop::Propagator::RunPropagation -- Running over " << nevents << " genie events" << std::endl;
  for(int i_event = 0; i_event < nevents; i_event++) {
    std::cout<<"rockprop::Propagator::RunPropagation -- Processing Event: " << i_event << std::endl;

    // get next input_tree entry
    input_tree->GetEntry(i_event);

    // get the GENIE event
    genie::EventRecord &  event = *(ntp_event_record->event);

    //A bit of verbosity for when the time is right
    event.SetPrintLevel(10);

    //Propagate all of the final state particles from the genie vertex and collect all particles which enter the detector cavern, represented as a vector of GHepParticles
    std::vector<std::unique_ptr<genie::GHepParticle> > cavern_entering_particles = PropagateAndFindCavernEnteringParticlesInEvent(event, runManager);
    //Get the number of particles which reached the cavern (good book keeping)
    int NCavernEnteringParticlesInEvent = cavern_entering_particles.size();
    //Add the GHEP particles which reached the cavern back into the GHEP event record, each particle's mother ID should point to the original particle that was propagated
    UpdateEventWithPropagatedParticles(event, std::move(cavern_entering_particles));
    
    //If we've found some cavern entering particles, sing about it and write the output to the output tree
    if (NCavernEnteringParticlesInEvent > 0) {
      std::cout<<"rockprop::Propagator::RunPropagation -- found " << NCavernEnteringParticlesInEvent << " particles in event " << i_event << ".  Dumping the event record" << std::endl;
      std::cout<<event<<std::endl;
      fNRockVertices++;
      ntp_writer.AddEventRecord(i_event,&event);
    }
    // clear current mc event record
    ntp_event_record->Clear();

  }//end loop over events

  //Copy the meta tree over to the new file
  TTree *t1 = (TTree*) input_file.Get("meta");
  if ( t1 ) {
    size_t nmeta = t1->GetEntries();
    TTree* t2 = (TTree*)t1->Clone(0);

    for (size_t i = 0; i < nmeta; i++) {
      t1->GetEntry(i);
      t2->Fill();
    }
    t2->Write();
  }
 
  //Save the output
  ntp_writer.Save();

  // close input GHEP event file
  input_file.Close();


  //A bit of a print dump at the end
  std::cout<<"Number of entering rock vertices found: " << fNRockVertices << std::endl;
  std::cout<<"Number of entering rock particles"<<std::endl;
  for (std::map<int,int>::iterator it = fNPDGToNRockParticlesMap.begin(); it != fNPDGToNRockParticlesMap.end(); it++){
    std::cout<<"---PDG: " << it->first << "  N: " << it->second << std::endl;
  }
  return 0;
}

std::vector<std::unique_ptr<genie::GHepParticle> > rockprop::Propagator::PropagateAndFindCavernEnteringParticlesInEvent(genie::EventRecord &event, std::unique_ptr<G4RunManager> &run_manager){

  //Make the vector that will store all cavern-entering particles
  std::vector<std::unique_ptr<genie::GHepParticle> > all_cavern_entering_ghep_particles;

  //Get the primary generator and update it with the latest genie event record
  rockprop::PrimaryGeneratorAction *primary_generator = (rockprop::PrimaryGeneratorAction*) run_manager->GetUserPrimaryGeneratorAction();
  primary_generator->SetEventRecord(&event);

  //Get the stepping action and update it with the latest genie event record
  rockprop::SteppingAction *stepper = (rockprop::SteppingAction*) run_manager->GetUserSteppingAction();
  stepper->SetEventRecord(&event);

  //Get the G4UI manager as we'll need it to start the propagation
  G4UImanager* fUIManager = G4UImanager::GetUIpointer();

  //Iterate over all GHEP particles in the event record
  TIter event_iter(&event);
  genie::GHepParticle * ghep_particle = nullptr;
  int ghep_id = 0; //Needed to get the GHEP ID of each GHEP Particle (so we can include the correct 'mother' ID of the propagated particles which reach the cavern)
  while((ghep_particle=dynamic_cast<genie::GHepParticle *>(event_iter.Next())))
  {
    //If we find a particle that should be propagated
    if (ghep_particle->Status() == genie::kIStStableFinalState){
      ghep_particle->SetStatus(genie::kIStUndefined); //Pick a state which would mean a downstream G4 propagator wouldn't automatically track the particle again (not a great choice but the enums were limited)
      //Reset everything
      primary_generator->Reset();
      stepper->ResetCavernParticleGHepVector();
      //Record the GHEP ID of the current particle in the stepper so it can correctly assign the GHEP mother ID to all propagated particles
      stepper->SetMotherID(ghep_id);
      //Accumulate the GHEP particles for ytracking
      primary_generator->Append(ghep_particle);
      //Run G4
      fUIManager->ApplyCommand("/run/beamOn 1");
      //Collect all particles which reached the cavern (the GHEP particles are the particle states just as the enter the detector cavern)
      std::vector<std::unique_ptr<genie::GHepParticle> > cavern_entering_ghep_particles = stepper->HandOverPropagatedGHepParticles();
      for (size_t i_cavernparticle = 0; i_cavernparticle < cavern_entering_ghep_particles.size(); i_cavernparticle++){
        all_cavern_entering_ghep_particles.emplace_back(std::move(cavern_entering_ghep_particles[i_cavernparticle]));
      }
    }
    ghep_id++; //increase the counter
  }// end loop over particles	

  return all_cavern_entering_ghep_particles;
}


void rockprop::Propagator::UpdateEventWithPropagatedParticles(genie::EventRecord &event, std::vector<std::unique_ptr<genie::GHepParticle> > cavern_entering_ghep_particles){
  //Loop over all the cavern-entering GHEP particles
  for (size_t i_cavernparticle = 0; i_cavernparticle < cavern_entering_ghep_particles.size(); i_cavernparticle++){
    //Add the particle to the event
    event.AddParticle(*(cavern_entering_ghep_particles[i_cavernparticle]));
    //Do some counting bookkeeping
    int pdg = cavern_entering_ghep_particles[i_cavernparticle]->Pdg();
    fNPDGToNRockParticlesMap[pdg]++;
  }
  return;
}

void rockprop::Propagator::GetCommandLineArgs(int argc, char ** argv){
  genie::CmdLnArgParser parser(argc,argv);
  // get GENIE event file
  if( parser.OptionExists('f') ) {
    fOptInputFilename = parser.ArgAsString('f');
  } 
  else {
    PrintCommandLineUsageOptions();
    std::cout<<"rockprop::Propagator::GetCommandLineArgs -- you did not specify an input GHEP file with -f.  Exiting!"<<std::endl;
    exit(1);
  }
  // Get the geometry file name
  if( parser.OptionExists('g') ) {
    fOptGeometryFileName = parser.ArgAsString('g');
  } 
  else {
    PrintCommandLineUsageOptions();
    std::cout<<"rockprop::Propagator::GetCommandLineArgs -- you did not specify a geometry GDML file with -g.  Exiting!"<<std::endl;
    exit(1);
  }
  //Output prefix
  if( parser.OptionExists('o') ) {
    fOptOutPrefix = parser.ArgAsString('o');
  }
  // number of events to analyse
  if( parser.OptionExists('n') ) {
    fOptNEvt = parser.ArgAsInt('n');
  } 
  else {
    fOptNEvt = -1;
  }
  //The random seed
  if( parser.OptionExists('s') ) {
    fOptRandomSeed = parser.ArgAsInt('s');
  } 
  else {
    //Use the system time if no seed was specified
    time_t systime = time(NULL);
    fOptRandomSeed = (systime*G4UniformRand());
  }
  //Print help if requested
  if ( parser.OptionExists('h') ){
    PrintCommandLineUsageOptions();
    exit(2);
  }
  return;
}

void rockprop::Propagator::PrintCommandLineUsageOptions(){
  std::cout<<"--------------------------------------------------------------------------------------------------"<<std::endl;
  std::cout<<"--------------------------------------------------------------------------------------------------"<<std::endl;
  std::cout<<"-------------------------------GENIE Rock Propagator CLI usage------------------------------------"<<std::endl;
  std::cout<<"--------------------------------------------------------------------------------------------------"<<std::endl;
  std::cout<<"--------------------------------------------------------------------------------------------------"<<std::endl;
  std::cout<<" -g GEOMEMTRY_GDML_FILE           - geometry GDML file path (MANDATORY)" << std::endl;
  std::cout<<" -f INPUT_GHEP_GENIE_FILE         - input GHEP GENIE file path (MANDATORY)" << std::endl;
  std::cout<<" -o OUTPUT_PREFIX                 - output file prefix" << std::endl;
  std::cout<<" -n NUM_EVENTS_TO_PROCESS         - the number of events to process" << std::endl;
  std::cout<<" -s RANDOM_SEED                   - the random seed (don't specify to use system time-based seed)" << std::endl;
  std::cout<<" -h                               - print this help screen" << std::endl;
  std::cout<<"--------------------------------------------------------------------------------------------------"<<std::endl;
  std::cout<<"don't be sad"<<std::endl;
  std::cout<<"because sad backwards is das"<<std::endl;
  std::cout<<"and"<<std::endl;
  std::cout<<"das not good"<<std::endl;
  std::cout<<"--------------------------------------------------------------------------------------------------"<<std::endl;
  return;
}
