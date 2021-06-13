#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

// Return Code
typedef uint32_t RC;

static constexpr RC RC_OK = 0u;
static constexpr RC RC_FAIL = 1u;

// TODO: support 8bit without overhead
struct RGB {
  uint16_t r, g, b;
};

struct Image {
  uint16_t width;
  uint16_t height;
  uint16_t color_max;
  std::vector<RGB> data;
};

//
// Reads all the file into buffer.
//
RC load_file(std::vector<uint8_t> &buffer, const std::string &file_name) {
  FILE *file = fopen(file_name.c_str(), "rb");
  if (!file) {
    printf("Error to open file %s\n", file_name.c_str());
    return RC_FAIL;
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  fseek(file, 0, SEEK_SET);

  buffer.resize(size + 1);
  fread((char *)&buffer[0], 1, size, file);
  buffer[size] = ' ';

  fclose(file);
  return RC_OK;
}

//
// Moves ptr to the next non-whitespace character.
//
void skip_white(uint8_t *&ptr, const uint8_t *end) {
  for (; ptr != end; ++ptr) {
    if (*ptr != '\n' && *ptr != '\r' && *ptr != '\t' && *ptr != ' ') {
      break;
    }
  }
}

//
// Reads uint16 from the position of ptr.
//
uint16_t read_uint16(uint8_t *&ptr, const uint8_t *end) {
  skip_white(ptr, end);

  uint16_t v = 0;
  for (; ptr != end; ++ptr) {
    if (*ptr < '0' || *ptr > '9')
      break;

    v *= 10;
    v += *ptr - '0';
  }
  return v;
}

//
// Moves ptr to the next non-newline character.
//
void skip_line(uint8_t *&ptr, const uint8_t *end) {
  for (; ptr != end; ++ptr) {
    if (*ptr == '\n') {
      break;
    }
  }
  ptr++;
}

//
// Moves ptr to the next character after comment line.
//
void skip_comment(uint8_t *&ptr, const uint8_t *end) {
  while (ptr != end) {
    skip_white(ptr, end);
    if (*ptr != '#') {
      break;
    }
    skip_line(ptr, end);
  }
}

//
//  Reads specified file to output parameter Image.
//  Returns RC_OK on success else RC_FAIL
//
RC load_ppm(Image &image, const std::string &file_name) {
  std::vector<uint8_t> file_data;
  if (load_file(file_data, file_name)) {
    printf("Failed to read file %s\n", file_name.c_str());
    return RC_FAIL;
  }

  uint8_t *ptr = &file_data[0];
  const uint8_t *end = ptr + file_data.size();

  skip_comment(ptr, end);
  skip_white(ptr, end);

  int pmode = 0;
  if (ptr + 2 < end && ptr[0] == 'P') {
    pmode = ptr[1] - '0';
    ptr += 2;
  }
  if (pmode != 3 && pmode != 6) {
    printf("Unknown pmode %d\n", pmode);
    return RC_FAIL;
  }

  skip_comment(ptr, end);
  image.width = read_uint16(ptr, end);
  if (image.width < 1) {
    printf("Unsupported width: %d\n", image.width);
    return RC_FAIL;
  }
  image.height = read_uint16(ptr, end);
  if (image.height < 1) {
    printf("Unsupported height: %d\n", image.height);
    return RC_FAIL;
  }

  skip_comment(ptr, end);
  image.color_max = read_uint16(ptr, end);
  if (image.color_max != 255 && image.color_max != 65535) {
    printf("Unsupported color_max: %d\n", image.color_max);
    return RC_FAIL;
  }

  image.data.resize(image.width * image.height);
  if (pmode == 6) {
    ptr++;
    memcpy(&image.data[0], ptr, image.data.size() * 3);
    for (int i = 0; i < image.data.size(); ++i) {
      const RGB &rgb = image.data[i];
      if (rgb.r >= image.color_max || rgb.g >= image.color_max ||
          rgb.b >= image.color_max) {
        printf("Color value of %dth pixel exceeds specified color_max:%d\n", i,
               image.color_max);
        return RC_FAIL;
      }
    }
  } else if (pmode == 3) {
    for (int i = 0; i < image.data.size(); i++) {
      const int r = read_uint16(ptr, end);
      const int g = read_uint16(ptr, end);
      const int b = read_uint16(ptr, end);

      if (r >= image.color_max || g >= image.color_max ||
          b >= image.color_max) {
        printf("Color value of %dth pixel exceeds specified color_max:%d\n", i,
               image.color_max);
        return RC_FAIL;
      }

      image.data[i].r = r;
      image.data[i].g = g;
      image.data[i].b = b;
    }
  }
  return RC_OK;
}

//
// Saves Image object to specified file
//
RC save_ppm(const Image &image, const std::string &file_name, bool out16) {
  FILE *file = fopen(file_name.c_str(), "w");
  if (!file) {
    printf("Error to open output file %s\n", file_name.c_str());
    return RC_FAIL;
  }

  fprintf(file, "P3\n");
  fprintf(file, "%d %d\n", image.width, image.height);
  fprintf(file, "%d\n", out16 ? 65535 : 255);

  int i = 0;
  for (const RGB &rgb : image.data) {
    auto r = rgb.r;
    auto g = rgb.g;
    auto b = rgb.b;

    if (image.color_max == 255 && out16) {
      // 8 -> 16
      r <<= 8;
      g <<= 8;
      b <<= 8;
    }
    if (image.color_max == 65535 && !out16) {
      // 16 -> 8
      r >>= 8;
      g >>= 8;
      b >>= 8;
    }

    if (out16) {
      fprintf(file, "%5d %5d %5d", r, g, b);
    } else {
      fprintf(file, "%3d %3d %3d", r, g, b);
    }
    if ((i % image.width) < (image.width - 1)) {
      fprintf(file, "   ");
    }
    if ((i > 0) && ((i + 1) % image.width == 0)) {
      fprintf(file, "\n");
    }
    i++;
  }

  fclose(file);
  return RC_OK;
}

//
// Result of argument parsing
//
struct Args {
  std::string input_file = "input_image.ppm";
  std::string output_file{};
  bool out16;
};

//
// Parses command line arguments.
// -i input file path
// -o output file path
// -16 if output file bits=16 (8 by default)
//
void parseArgs(Args &args, int argc, char **argv) {
  int i = 0;
  bool hasO = false;

  while (i < argc) {
    std::string arg = argv[i];
    if (arg == "-i") {
      if (i + 1 < argc) {
        args.input_file = argv[++i];
      }
    } else if (arg == "-o") {
      hasO = true;
      if (i + 1 < argc) {
        args.output_file = argv[++i];
      }
    } else if (arg == "-16") {
      args.out16 = true;
    }
    i++;
  }

  if (!hasO) {
    args.output_file =
        args.input_file.substr(0, args.input_file.find('.')) + "_out.ppm";
  }
}

} // namespace

int main(int argc, char **argv) {
  Args args{};
  parseArgs(args, argc, argv);

  Image image{};
  if (load_ppm(image, args.input_file) != RC_OK) {
    return 1;
  }

  if (save_ppm(image, args.output_file, args.out16) != RC_OK) {
    return 1;
  }

  return 0;
}
