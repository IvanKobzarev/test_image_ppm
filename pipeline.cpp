#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

// Return Code
typedef uint32_t RC;

static constexpr RC RC_OK = 0u;
static constexpr RC RC_FAIL = 1u;

struct Image {
  uint16_t width;
  uint16_t height;
  uint8_t bits;
  std::vector<uint8_t> data;
  size_t data_size_bytes;
  size_t px_size;
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
// Reads uint8 from the position of ptr.
//
uint32_t read_uint32(uint8_t *&ptr, const uint8_t *end) {
  skip_white(ptr, end);

  uint32_t v = 0;
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
  image.width = read_uint32(ptr, end);
  if (image.width < 1) {
    printf("Unsupported width: %d\n", image.width);
    return RC_FAIL;
  }
  image.height = read_uint32(ptr, end);
  if (image.height < 1) {
    printf("Unsupported height: %d\n", image.height);
    return RC_FAIL;
  }

  skip_comment(ptr, end);
  auto color_max = read_uint32(ptr, end);
  if (color_max == 255) {
    image.bits = 8;
  } else if (color_max == 65535) {
    image.bits = 16;
  } else {
    printf("Unsupported color_max: %d\n", color_max);
    return RC_FAIL;
  }
  image.px_size = 3 * image.width * image.height;
  image.data_size_bytes = image.px_size * image.bits / 8;
  image.data.resize(image.data_size_bytes);

  if (image.bits == 8) {
    // bits 8
    uint8_t *image_data = image.data.data();
    if (pmode == 6) {
      ptr++;
      memcpy(image_data, ptr, image.data_size_bytes);
    } else {
      for (int i = 0; i < image.px_size; ++i) {
        const auto c = read_uint32(ptr, end);
        if (c > 255) {
          printf("%dth color value exceeds 255\n", i);
          return RC_FAIL;
        }
        image_data[i] = c;
      }
    }
  } else {
    // bits 16
    uint16_t *image_data = (uint16_t *)image.data.data();
    if (pmode == 6) {
      ptr++;
      memcpy(image_data, ptr, image.data_size_bytes);
    } else {
      for (int i = 0; i < image.px_size; ++i) {
        const auto c = read_uint32(ptr, end);
        if (c > 65535) {
          printf("%dth color value exceeds 65535\n", i);
          return RC_FAIL;
        }
        image_data[i] = c;
      }
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

  if (image.bits == 8) {
    const uint8_t *image_data = image.data.data();
    for (int i = 0; i < image.width * image.height; ++i) {
      uint16_t r = image_data[3 * i + 0];
      uint16_t g = image_data[3 * i + 1];
      uint16_t b = image_data[3 * i + 2];

      if (out16) {
        // 8 -> 16
        r <<= 8;
        g <<= 8;
        b <<= 8;
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
    }
  } else {
    const uint16_t *image_data = (uint16_t *)image.data.data();
    for (int i = 0; i < image.width * image.height; ++i) {
      uint16_t r = image_data[3 * i + 0];
      uint16_t g = image_data[3 * i + 1];
      uint16_t b = image_data[3 * i + 2];
      if (!out16) {
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
    }
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
