#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <prediction-by-partial-matching.h>
#include <file-vector-converter.h>
#include <vector>

int main() {
  auto buffer = ResearchLibrary::File::load("../cpplint,py");
  auto ppm = ResearchLibrary::PredictionByPartialMatching::NumericEncode<MethodC, 4>(buffer);
  printf("%zu -> %zu\n", buffer.size(), ppm.first.size());
  return 0;
}
