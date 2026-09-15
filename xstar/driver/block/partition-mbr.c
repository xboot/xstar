/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <kernel/core/logger.h>
#include <driver/block/partition.h>

struct mbr_entry_t {
	uint8_t flag;
	uint8_t start_head;
	uint8_t start_sector;
	uint8_t start_cylinder;
	uint8_t type;
	uint8_t end_head;
	uint8_t end_sector;
	uint8_t end_cylinder;
	uint8_t start[4];
	uint8_t length[4];
} __attribute__ ((packed));

struct mbr_header_t {
	uint8_t code[446];
	struct mbr_entry_t entry[4];
	uint8_t signature[2];
} __attribute__ ((packed));

static int is_valid_mbr(struct mbr_header_t * mbr)
{
	if((mbr->signature[0] != 0x55) || (mbr->signature[1] != 0xaa))
		return 0;
	return 1;
}

static int is_protective_gpt(struct mbr_header_t * mbr)
{
	if((mbr->entry[0].type == 0xee) || (mbr->entry[1].type == 0xee) || (mbr->entry[2].type == 0xee) || (mbr->entry[3].type == 0xee))
		return 1;
	return 0;
}

static int is_extended(uint8_t type)
{
	if((type == 0x5) || (type == 0xf) || (type == 0x85))
		return 1;
	return 0;
}

static void register_mbr_partition(struct block_t * pblk, uint64_t offset, uint64_t length, const char * name, char * sbuf)
{
	if((offset < block_capacity(pblk)) && (length > 0) && (length <= block_capacity(pblk) - offset))
	{
		struct device_t * dev = register_sub_block(pblk, offset, length, name);
		if(dev)
		{
			struct block_t * blk = (struct block_t *)dev->priv;
			LOG("  0x%016Lx ~ 0x%016Lx %s %*s- %s\r\n", offset, offset + length - 1, xos_ssize(sbuf, length), 9 - xos_strlen(sbuf), "", blk->name);
		}
	}
}

int partition_detect_mbr(struct block_t * pblk)
{
	struct mbr_header_t mbr;
	char sbuf[64];
	char nbuf[64];
	uint64_t ext_base = 0;
	int logical = 5;

	if(!pblk || !pblk->name || (block_capacity(pblk) <= 0))
		return 0;

	if(block_read(pblk, (uint8_t *)(&mbr), 0, sizeof(struct mbr_header_t)) != sizeof(struct mbr_header_t))
		return 0;

	if(!is_valid_mbr(&mbr) || is_protective_gpt(&mbr))
		return 0;

	LOG("Found mbr partition:\r\n");
	LOG("  0x%016Lx ~ 0x%016Lx %s %*s- %s\r\n", 0ULL, block_capacity(pblk) - 1, xos_ssize(sbuf, block_capacity(pblk)), 9 - xos_strlen(sbuf), "", pblk->name);
	for(int i = 0; i < 4; i++)
	{
		if(mbr.entry[i].type != 0)
		{
			uint32_t lba = get_unaligned_le32(mbr.entry[i].start);
			uint32_t cnt = get_unaligned_le32(mbr.entry[i].length);
			if(is_extended(mbr.entry[i].type))
			{
				if((lba > 0) && (ext_base == 0))
					ext_base = lba;
			}
			else if((lba > 0) && (cnt > 0))
			{
				xos_snprintf(nbuf, sizeof(nbuf), "p%d", i);
				register_mbr_partition(pblk, (uint64_t)lba * 512, (uint64_t)cnt * 512, nbuf, sbuf);
			}
		}
	}
	for(uint64_t ebr_lba = ext_base; (ebr_lba > 0) && (logical < 5 + 128); logical++)
	{
		struct mbr_header_t ebr;
		if((ebr_lba * 512 + sizeof(struct mbr_header_t)) > block_capacity(pblk))
			break;
		if(block_read(pblk, (uint8_t *)(&ebr), ebr_lba * 512, sizeof(struct mbr_header_t)) != sizeof(struct mbr_header_t))
			break;
		if(!is_valid_mbr(&ebr))
			break;
		uint32_t start = get_unaligned_le32(ebr.entry[0].start);
		uint32_t count = get_unaligned_le32(ebr.entry[0].length);
		if((ebr.entry[0].type != 0) && !is_extended(ebr.entry[0].type) && (start > 0) && (count > 0))
		{
			xos_snprintf(nbuf, sizeof(nbuf), "p%d", logical);
			register_mbr_partition(pblk, (ebr_lba + start) * 512, (uint64_t)count * 512, nbuf, sbuf);
		}
		uint32_t next = get_unaligned_le32(ebr.entry[1].start);
		if((ebr.entry[1].type == 0) || (next == 0))
			break;
		ebr_lba = ext_base + next;
	}
	return 1;
}
