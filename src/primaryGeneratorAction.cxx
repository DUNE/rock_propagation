#include "primaryGeneratorAction.hxx"

//ugly static magic
G4ParticleTable* rockprop::PrimaryGeneratorAction::fParticleTable = nullptr;

void rockprop::PrimaryGeneratorAction::GeneratePrimaries(G4Event* event){
  //Make the G4 particle table if we don't have it
  if (fParticleTable == nullptr){
    fParticleTable = G4ParticleTable::GetParticleTable();
  }

  //Loop over the particles we've accumulated in the vector
  for (std::vector<genie::GHepParticle*>::iterator it = fParticles.begin(); it != fParticles.end(); it++){
    genie::GHepParticle *particle = (*it);
    if (!particle){
      std::cout<<"rockprop::PrimaryGeneratorAction::GeneratePrimaries -- DID NOT GET THE PARTICLE FROM THE VECTOR FOR G4GENERATOR!!!"<<std::endl;
    }

    G4int PDG = particle->Pdg();
    G4ParticleDefinition *def = fParticleTable->FindParticle(PDG);
    //Check the particle exists
    if (def == 0){
      std::cout<<"rockprop::PrimaryGeneratorAction::GeneratePrimaries -- FOUND UNKNOWN PARTICLE with PDG: " << PDG <<std::endl;
      continue;
    }

    //Get the position of the particle (and include its offset from the nucleus centre (the correction is in femtometres (lol))
    G4double x = (fEventRecord->Vertex()->X()+1.e-15*particle->Vx()) * CLHEP::m;
    G4double y = (fEventRecord->Vertex()->Y()+1.e-15*particle->Vy()) * CLHEP::m;
    G4double z = (fEventRecord->Vertex()->Z()+1.e-15*particle->Vz()) * CLHEP::m;
    //Take the time as the vertex time (the particle times seem to be 0?)
    G4double t = fEventRecord->Vertex()->T() * CLHEP::s;

    //Get the mometum of the particle
    G4double px = particle->GetP4()->X() * CLHEP::GeV;
    G4double py = particle->GetP4()->Y() * CLHEP::GeV;
    G4double pz = particle->GetP4()->Z() * CLHEP::GeV;

    //Make a vertex from the time
    G4PrimaryVertex* vertex = new G4PrimaryVertex(x,y,z,t);
    //Make a g4 particle to go in the vertex
    G4PrimaryParticle* g4particle = new G4PrimaryParticle( def, px, py, pz);
    //Add the particle to the vertex
    vertex->SetPrimary(g4particle);
    //Add the vertex to event and let G4 take its course
    event->AddPrimaryVertex(vertex);
  }
  return;
}


