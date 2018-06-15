#include "nal-sps.h"

#include <iostream>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include <cassert>

#include "common.h"
#include "nal.h"

using namespace std;

// default values in case SPS is not decoded yet...
SpsInfo::SpsInfo() : is_ok(false), log2_max_frame_num(4), frame_mbs_only_flag(1), log2_max_poc_lsb(5), poc_type(0) {};

SpsInfo::SpsInfo(const NalInfo& nal_info, int max_size) {
	is_ok = decode(nal_info);
}

bool SpsInfo::decode(const NalInfo& nal_info) {
	uchar* pos = nal_info.payload_;
//	cout << "nal_info.type = " << nal_info.nal_type << '\n';
//	cout << "I am here:\n";
//	printBuffer(pos, 20);
	logg(V, "decoding SPS ...\n");
	int offset = 0;
	pos += 3; // skip 24 bits
	readGolomb(pos, offset); // sps_id

	int log2_max_frame_num_minus4 = readGolomb(pos, offset);
	log2_max_frame_num = log2_max_frame_num_minus4 + 4;
	logg(V, "log2_max_frame_num: ", log2_max_frame_num);

	poc_type = readGolomb(pos, offset);
	if (poc_type == 0) {
		int log2_max_poc_lsb_minus4 = readGolomb(pos, offset);
		log2_max_poc_lsb = log2_max_poc_lsb_minus4 + 4;
	} else if (poc_type == 1) {
		readBits(1, pos, offset); // delta_pic_order_always_zero_flag
		readGolomb(pos, offset); // offset_for_non_ref_pic
		readGolomb(pos, offset); // offset_for_top_to_bottom_field
		int poc_cycle_length = readGolomb(pos, offset);
		for (int i = 0; i < poc_cycle_length; i++)
			readGolomb(pos, offset); // offset_for_ref_frame[i]
	} else if (poc_type != 2) {
		cout << "invalid poc_type\n";
		return false;
	}

	readGolomb(pos, offset); // ref_frame_count
	readBits(1, pos, offset); // gaps_in_frame_num_allowed_flag
	readGolomb(pos, offset); // mb_width
	readGolomb(pos, offset); // mb_height

	frame_mbs_only_flag = readBits(1, pos, offset);
	return true;
}