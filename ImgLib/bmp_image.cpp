#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

	const array<char, 2> BMP_TYPE = { 'B', 'M' };
	const uint16_t BMP_RESERVED = 0;
	const uint16_t BMP_PLANES = 1;
	const uint16_t BMP_BIT_COUNT = 24;
	const uint32_t BMP_COMPRESSION = 0;
	const int32_t BMP_PIX_PER_METER = 11811;
	const uint32_t BMP_COLOR_USED = 0;
	const uint32_t BMP_COLOR_IMPORTANT = 0x1000000;

	PACKED_STRUCT_BEGIN BitmapFileHeader{
		array<char, 2> type;
		uint32_t file_size;
		uint16_t reserved_1;
		uint16_t reserved_2;
		uint32_t off_bits;
	}PACKED_STRUCT_END

	PACKED_STRUCT_BEGIN BitmapInfoHeader{
		uint32_t struct_size;
		int32_t width;
		int32_t height;
		uint16_t planes;
		uint16_t bit_count;
		uint32_t compression;
		uint32_t img_size;
		int32_t x_pix_per_meter;
		int32_t y_pix_per_meter;
		uint32_t color_used;
		uint32_t color_important;
	}PACKED_STRUCT_END

	// функция вычисления отступа по ширине
	static int GetBMPStride(int width) {
		return 4 * ((width * 3 + 3) / 4);
	}

	// напишите эту функцию
	bool SaveBMP(const Path& file, const Image& image) {
		ofstream fout(file, ios::binary);
		int image_width = image.GetWidth();
		int image_height = image.GetHeight();

		int padding = GetBMPStride(image_width);

		BitmapFileHeader bit_file_header = {
			BMP_TYPE,
			sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + static_cast<uint32_t>(padding * image_height),
			BMP_RESERVED,
			BMP_RESERVED,
			sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader)
		};

		fout.write(bit_file_header.type.data(), 2);
		fout.write(reinterpret_cast<char*>(&bit_file_header.file_size), 4);
		fout.write(reinterpret_cast<char*>(&bit_file_header.reserved_1), 2);
		fout.write(reinterpret_cast<char*>(&bit_file_header.reserved_2), 2);
		fout.write(reinterpret_cast<char*>(&bit_file_header.off_bits), 4);

		// -------------------------------------

		BitmapInfoHeader bit_info_header = {
			sizeof(BitmapInfoHeader),
			image.GetWidth(),
			image.GetHeight(),
			BMP_PLANES,
			BMP_BIT_COUNT,
			BMP_COMPRESSION,
			static_cast<uint32_t>(padding * image_height),
			BMP_PIX_PER_METER,
			BMP_PIX_PER_METER,
			BMP_COLOR_USED,
			BMP_COLOR_IMPORTANT
		};

		fout.write(reinterpret_cast<char*>(&bit_info_header.struct_size), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.width), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.height), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.planes), 2);
		fout.write(reinterpret_cast<char*>(&bit_info_header.bit_count), 2);
		fout.write(reinterpret_cast<char*>(&bit_info_header.compression), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.img_size), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.x_pix_per_meter), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.y_pix_per_meter), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.color_used), 4);
		fout.write(reinterpret_cast<char*>(&bit_info_header.color_important), 4);

		// -------------------------------------

		vector<char> buffer(image_width * 3);

		padding -= image_width * 3;
		vector<char> padding_buffer(padding, 0);

		for (int y = image_height - 1; y >= 0; --y) {
			const Color* line = image.GetLine(y);

			for (int x = 0; x < image_width; ++x) {
				buffer[x * 3 + 0] = static_cast<char>(line[x].b);
				buffer[x * 3 + 1] = static_cast<char>(line[x].g);
				buffer[x * 3 + 2] = static_cast<char>(line[x].r);
			}

			fout.write(buffer.data(), image_width * 3);
			fout.write(padding_buffer.data(), padding);
		}

		return true;
	}

	// напишите эту функцию
	Image LoadBMP(const Path& file) {
		ifstream fin(file, ios::binary);
		if (!fin.is_open()) { return {}; }

		BitmapFileHeader bit_file_header;

		fin.read(bit_file_header.type.data(), 2);
		fin.read(reinterpret_cast<char*>(&bit_file_header.file_size), 4);
		fin.read(reinterpret_cast<char*>(&bit_file_header.reserved_1), 2);
		fin.read(reinterpret_cast<char*>(&bit_file_header.reserved_2), 2);
		fin.read(reinterpret_cast<char*>(&bit_file_header.off_bits), 4);

		// -------------------------------------

		BitmapInfoHeader bit_info_header;

		fin.read(reinterpret_cast<char*>(&bit_info_header.struct_size), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.width), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.height), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.planes), 2);
		fin.read(reinterpret_cast<char*>(&bit_info_header.bit_count), 2);
		fin.read(reinterpret_cast<char*>(&bit_info_header.compression), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.img_size), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.x_pix_per_meter), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.y_pix_per_meter), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.color_used), 4);
		fin.read(reinterpret_cast<char*>(&bit_info_header.color_important), 4);

		// -------------------------------------

		int image_width = bit_info_header.width;
		int image_height = bit_info_header.height;

		Image new_image(image_width, image_height, Color::Black());
		
		int padding = GetBMPStride(image_width) - (image_width * 3);

		vector<char> buffer(image_width * 3);
		vector<char> padding_buffer(padding);

		for (int y = image_height - 1; y >= 0; --y) {
			Color* line = new_image.GetLine(y);

			fin.read(buffer.data(), image_width * 3);

			for (int x = 0; x < image_width; ++x) {
				line[x].b = static_cast<byte>(buffer[x * 3 + 0]);
				line[x].g = static_cast<byte>(buffer[x * 3 + 1]);
				line[x].r = static_cast<byte>(buffer[x * 3 + 2]);
			}

			fin.read(padding_buffer.data(), padding);
		}

		return new_image;
	}

}  // namespace img_lib