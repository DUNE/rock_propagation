//Author Dom Brailsford
//trivial main function which actually runs the propagation code

#include "propagator.hxx"

int main (int argc, char ** argv){
  rockprop::Propagator propagator(argc, argv);
  return propagator.RunPropagation();
}
