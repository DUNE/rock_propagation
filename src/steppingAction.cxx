#include "steppingAction.hxx"

void rockprop::SteppingAction::UserSteppingAction(const G4Step* step){
  G4int pdg = step->GetTrack()->GetDefinition()->GetPDGEncoding();
  if (std::abs(pdg) == 14 || std::abs(pdg) == 12 || std::abs(pdg)==16){
    step->GetTrack()->SetTrackStatus(fStopAndKill);
    return;
  }

  if (!step->GetPreStepPoint()->GetPhysicalVolume()) return;
  if (!step->GetPostStepPoint()->GetPhysicalVolume()) return;
  G4String preVolumeName = step->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetName();
  G4String postVolumeName = step->GetPostStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetName();
  if ((preVolumeName != postVolumeName) && ((postVolumeName == "volDetEnclosure") || postVolumeName == "volDetector")){
    //Std::cout<<"Found entering particle with pdg " << pdg << std::endl;
    //Std::cout<<"---Pre volume: " << preVolumeName << std::endl;
    //Std::cout<<"---Post volume: " << postVolumeName << std::endl;
    G4ThreeVector p_g4 = step->GetPostStepPoint()->GetMomentum();
    TLorentzVector p(p_g4.x() / CLHEP::GeV,
                     p_g4.y() / CLHEP::GeV,
                     p_g4.z() / CLHEP::GeV,
                     step->GetPostStepPoint()->GetTotalEnergy() / CLHEP::GeV);

    G4ThreeVector v_g4 = step->GetPostStepPoint()->GetPosition();
    TLorentzVector v((v_g4.x()/CLHEP::m - fEventRecord->Vertex()->X())/1.e-15,
                     (v_g4.y()/CLHEP::m - fEventRecord->Vertex()->Y())/1.e-15,
                     (v_g4.z()/CLHEP::m - fEventRecord->Vertex()->Z())/1.e-15,
                     step->GetPostStepPoint()->GetGlobalTime()/CLHEP::ns);

    fSavedGHepParticles.emplace_back(new genie::GHepParticle(pdg, genie::kIStStableFinalState, fMotherID, -1, -1, -1, p, v));

    //std::cout<<"Found a particle with PDG " << pdg << ", ID: " << step->GetTrack()->GetTrackID() << ", momentum (GeV) ("<<p.X() <<","<<p.Y() <<","<<p.Z() <<")" << " entering the cavern at the following position (m) (" <<(v.X()*1.e-15+fEventRecord->Vertex()->X())<<","<<(v.Y()*1.e-15+fEventRecord->Vertex()->Y())<<","<<(v.Z()*1.e-15+fEventRecord->Vertex()->Z())<<")"<<std::endl;
    step->GetTrack()->SetTrackStatus(fStopAndKill);
    return;

  }

  return;
}

std::vector<std::unique_ptr<genie::GHepParticle> > rockprop::SteppingAction::HandOverPropagatedGHepParticles(){
  std::vector<std::unique_ptr<genie::GHepParticle> > returned_ghep_particles;
  for (size_t i_ghep = 0; i_ghep < fSavedGHepParticles.size(); i_ghep++){
    returned_ghep_particles.emplace_back(std::move(fSavedGHepParticles[i_ghep]));
  }
  fSavedGHepParticles.clear();
  return returned_ghep_particles;
}
