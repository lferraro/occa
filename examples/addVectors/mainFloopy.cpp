#include <iostream>

#include "occa.hpp"

int main(int argc, char **argv){
  int entries = 5;

  float *a  = new float[entries];
  float *b  = new float[entries];
  float *ab = new float[entries];

  for(int i = 0; i < entries; ++i){
    a[i]  = i;
    b[i]  = 1 - i;
    ab[i] = 0;
  }

  std::string mode = "OpenCL";
  int platformID = 1;
  int deviceID   = 0;

  occa::device device;
  occa::kernel addVectors;
  occa::memory o_a, o_b, o_ab;

  device.setup(mode, platformID, deviceID);

  o_a  = device.malloc(entries*sizeof(float));
  o_b  = device.malloc(entries*sizeof(float));
  o_ab = device.malloc(entries*sizeof(float));

  addVectors = device.buildKernelFromLoopy("addVectors.floopy",
                                           "addVectors",
                                           occa::useFloopy);

  int dims = 1;
  int itemsPerGroup(16);
  int groups((entries + itemsPerGroup - 1)/itemsPerGroup);

  addVectors.setWorkingDims(dims, itemsPerGroup, groups);

  o_a.copyFrom(a);
  o_b.copyFrom(b);

  occa::initTimer(device);

  occa::tic("addVectors");

  addVectors(entries, o_a, o_b, o_ab);

  double elapsedTime = occa::toc("addVectors", addVectors);

  o_ab.copyTo(ab);

  std::cout << "Elapsed time = " << elapsedTime << " s" << std::endl;

  occa::printTimer();

  for(int i = 0; i < 5; ++i)
    std::cout << i << ": " << ab[i] << '\n';

  for(int i = 0; i < entries; ++i){
    if(ab[i] != (a[i] + b[i]))
      throw 1;
  }

  delete [] a;
  delete [] b;
  delete [] ab;

  addVectors.free();
  o_a.free();
  o_b.free();
  o_ab.free();
  device.free();

  return 0;
}