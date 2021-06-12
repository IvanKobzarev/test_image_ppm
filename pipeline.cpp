#include <cstdio>
#include <gflags/gflags.h>

DEFINE_string(i, "input_image.ppm", "input ppm file");

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
  int width;
  int height;
  int color_max;
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
      if (rgb.r >= image.color_max || rgb.g >= image.color_max || rgb.b >= image.color_max) {
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

      if (r >= image.color_max || g >= image.color_max || b >= image.color_max) {
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

} // namespace

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  const std::string &input_file = FLAGS_i;
  Image image{};
  if (load_ppm(image, input_file) != RC_OK) {
    return 1;
  }
  return 0;
}