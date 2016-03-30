// Copyright 2015 pixie.grasper

#include <cstdlib>
#include <vector>

#include "../includes/raster-image.h"

int main() {
  auto image = ResearchLibrary::Graphics::load("lena.bmp");
  image.save_as_bitmap("lena-out.bmp");
  image.save_as_png("lena-out.png");
  return 0;
}
