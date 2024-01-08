#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GET_RANDOM_DOUBLE(min, max) (((double)rand() / RAND_MAX) * ((max) - (min)) + (min))

#define HIGH_DANGER_MAX 500
#define MEDIUM_DANGER_MAX 1000

typedef unsigned char byte;

byte calculate_checksum_short(short v) {
  const byte *ptr = (byte *)&v;
  return ptr[0] ^ ptr[1];
}

byte calculate_checksum_double(double v) {
  const byte *ptr = (byte *)&v;
  return ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3] ^ ptr[4] ^ ptr[5] ^ ptr[6] ^ ptr[7];
}

byte calculate_checksum(unsigned short distance1, unsigned short distance2, unsigned short distance3, double latitude,
                        double longitude, byte pulse) {
  byte checksum = 0;
  checksum ^= calculate_checksum_short(distance1);
  checksum ^= calculate_checksum_short(distance2);
  checksum ^= calculate_checksum_short(distance3);
  checksum ^= calculate_checksum_double(latitude);
  checksum ^= calculate_checksum_double(longitude);
  checksum ^= pulse;
  return checksum;
}

int main(int argc, char **argv) {
  srand(time(NULL));
  if (sizeof(unsigned short) != 2 || sizeof(double) != 8 || sizeof(byte) != 1) {
    puts("AssertionFailed: system does not meet assumptions");
    return 1;
  }
  if (argc < 2) {
    puts("missing argument");
    return 1;
  }
  if (strcmp(argv[1], "generate") == 0) {
    if (argc < 3) {
      printf("usage: %s generate <count>\n", argv[0]);
      return 1;
    }
    int count = atoi(argv[2]);
    if (count == 0) {
      puts("count must be greater than 0");
      return 1;
    }
    for (int i = 0; i < count; i++) {
      unsigned short distance1 = rand() % 4000;
      unsigned short distance2 = rand() % 6000;
      unsigned short distance3 = rand() % 4000;
      // double latitude = GET_RANDOM_DOUBLE(-90.0, 90.0);
      // double longitude = GET_RANDOM_DOUBLE(-180.0, 180.0);
      double latitude = GET_RANDOM_DOUBLE(52.5, 52.6);
      double longitude = GET_RANDOM_DOUBLE(13.4, 13.5);
      byte pulse = 0;
      byte checksum = calculate_checksum(distance1, distance2, distance3, latitude, longitude, pulse);

      fwrite(&distance1, 2, 1, stdout);
      fwrite(&distance2, 2, 1, stdout);
      fwrite(&distance3, 2, 1, stdout);
      fwrite(&latitude, 8, 1, stdout);
      fwrite(&longitude, 8, 1, stdout);
      fwrite(&pulse, 1, 1, stdout);
      fwrite(&checksum, 1, 1, stdout);
    }
    return 0;
  } else if (strcmp(argv[1], "show") == 0) {
    while (1) {
      unsigned short distance1;
      unsigned short distance2;
      unsigned short distance3;
      double latitude;
      double longitude;
      byte pulse;
      byte checksum;
      int n = fread(&distance1, 2, 1, stdin);
      if (n == 0) {
        printf("eof\n");
        return 0;
      }
      fread(&distance2, 2, 1, stdin);
      fread(&distance3, 2, 1, stdin);
      fread(&latitude, 8, 1, stdin);
      fread(&longitude, 8, 1, stdin);
      fread(&pulse, 1, 1, stdin);
      fread(&checksum, 1, 1, stdin);

      printf("%dmm\t%dmm\t%dmm\t%lf°\t%lf°\t%dHz\t", distance1, distance2, distance3, latitude, longitude, pulse);

      if ((calculate_checksum(distance1, distance2, distance3, latitude, longitude, pulse) ^ checksum) == 0) {
        printf("VALID\n");
      } else {
        printf("INVALID\n");
      }
    }
  } else if (strcmp(argv[1], "filter") == 0) {
    if (argc < 3) {
      fprintf(stderr, "usage: %s filter <medium|high>\n", argv[0]);
      return 1;
    }
    const char *filter_type = argv[2];
    int limit = -1;
    if (strcmp(filter_type, "medium") == 0) {
      limit = MEDIUM_DANGER_MAX;
    } else if (strcmp(filter_type, "high") == 0) {
      limit = HIGH_DANGER_MAX;
    } else {
      fprintf(stderr, "Invalid filter type\n");
      return 1;
    }
    while (1) {
      unsigned short distance1;
      unsigned short distance2;
      unsigned short distance3;
      double latitude;
      double longitude;
      byte pulse;
      byte checksum;
      int n = fread(&distance1, 2, 1, stdin);
      if (n == 0) {
        return 0;
      }
      fread(&distance2, 2, 1, stdin);
      fread(&distance3, 2, 1, stdin);
      fread(&latitude, 8, 1, stdin);
      fread(&longitude, 8, 1, stdin);
      fread(&pulse, 1, 1, stdin);
      fread(&checksum, 1, 1, stdin);

      if ((calculate_checksum(distance1, distance2, distance3, latitude, longitude, pulse) ^ checksum) != 0) {
        fprintf(stderr, "Found invalid entry\n");
        continue;
      }

      if (distance1 < limit || distance2 < limit || distance3 < limit) {
        fwrite(&distance1, 2, 1, stdout);
        fwrite(&distance2, 2, 1, stdout);
        fwrite(&distance3, 2, 1, stdout);
        fwrite(&latitude, 8, 1, stdout);
        fwrite(&longitude, 8, 1, stdout);
        fwrite(&pulse, 1, 1, stdout);
        fwrite(&checksum, 1, 1, stdout);
      }
    }
    return 0;
  } else {
    puts("unkown argument");
    return 1;
  }
}