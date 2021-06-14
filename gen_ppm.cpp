#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

// Return Code
typedef uint32_t RC;

static constexpr RC RC_OK = 0u;
static constexpr RC RC_FAIL = 1u;

//
// Result of argument parsing
//
struct Args {
  std::string output_file = "output.ppm";
  int width = 4;
  int height = 4;
};

void fprint8(FILE *file, const uint16_t &r, const uint16_t &g,
             const uint16_t &b) {
  fprintf(file, "%3d %3d %3d", r, g, b);
}

void fprint_format(FILE *file, int i, uint32_t image_width) {
  if ((i % image_width) < (image_width - 1)) {
    fprintf(file, "   ");
  }
  if ((i > 0) && ((i + 1) % image_width == 0)) {
    fprintf(file, "\n");
  }
}

//
// Saves Image object to specified file
//
RC save_ppm(const Args &args) {
  FILE *file = fopen(args.output_file.c_str(), "w");
  if (!file) {
    return RC_FAIL;
  }

  int max_color = 255;
  fprintf(file, "P3\n");
  fprintf(file, "%d %d\n", args.width, args.height);
  fprintf(file, "%d\n", max_color);

  int i = 0;
  for (int y = 0; y < args.height; y++) {
    for (int x = 0; x < args.width; x++) {
      fprint8(file, 255, 255, 255);
      fprint_format(file, i, args.width);
      i++;
    }
  }

  fclose(file);
  return RC_OK;
}

void parseArgs(Args &args, int argc, char **argv) {
  int i = 0;
  while (i < argc) {
    std::string arg = argv[i];
    if (arg == "-w") {
      if (i + 1 < argc) {
        args.width = std::atoi(argv[++i]);
      }
    } else if (arg == "-h") {
      if (i + 1 < argc) {
        args.height = std::atoi(argv[++i]);
      }
    } else if (arg == "-o") {
      if (i + 1 < argc) {
        args.output_file = argv[++i];
      }
    }
    i++;
  }
}

} // namespace

int main(int argc, char **argv) {

  Args args{};
  parseArgs(args, argc, argv);

  if (save_ppm(args) != RC_OK) {
    return 1;
  }

  return 0;
}
